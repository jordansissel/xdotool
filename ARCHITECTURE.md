# xdotool Architecture Study

> This document is a study of xdotool's design, its deep coupling to X11, and the lessons it
> offers for building `waydo` — a Wayland equivalent. It is not a contribution to xdotool itself.

---

## Table of Contents

1. [Two-Layer Design](#1-two-layer-design)
2. [X11 Protocols Used — and Why They Don't Port to Wayland](#2-x11-protocols-used--and-why-they-dont-port-to-wayland)
3. [Core State: `xdo_t`](#3-core-state-xdo_t)
4. [Keyboard Simulation — The Hardest Part](#4-keyboard-simulation--the-hardest-part)
5. [Mouse Simulation](#5-mouse-simulation)
6. [Window Search](#6-window-search)
7. [Window Management via EWMH](#7-window-management-via-ewmh)
8. [The Window Stack (CLI Layer)](#8-the-window-stack-cli-layer)
9. [Edge Cases and Gotchas](#9-edge-cases-and-gotchas)
10. [What waydo Must Solve Differently](#10-what-waydo-must-solve-differently)

---

## 1. Two-Layer Design

xdotool is cleanly split into a library and a CLI frontend:

```
┌─────────────────────────────────────────────────────┐
│  xdotool CLI  (xdotool.c + cmd_*.c, ~40 files)      │
│  - command dispatcher (name → function pointer)      │
│  - window stack  (%1, %-1, %@)                       │
│  - argument parsing per command                      │
└───────────────────────┬─────────────────────────────┘
                        │ calls into
┌───────────────────────▼─────────────────────────────┐
│  libxdo  (xdo.c + xdo_search.c)                     │
│  - public C API (xdo.h)                              │
│  - xdo_t context struct                              │
│  - charcodemap_t keyboard table (built at init)      │
└───────────────────────┬─────────────────────────────┘
                        │ speaks
┌───────────────────────▼─────────────────────────────┐
│  X11 Protocol & Extensions                          │
│  Xlib · XTest · XKB · EWMH · ICCCM · Xinerama      │
└─────────────────────────────────────────────────────┘
```

**Key point:** libxdo is independently usable (Python bindings, etc.). The CLI is a thin wrapper
that handles chaining and the window stack. This separation is worth replicating in waydo.

---

## 2. X11 Protocols Used — and Why They Don't Port to Wayland

The table below shows every X11 mechanism xdotool relies on, and the state of Wayland equivalents.

| Protocol / Extension | xdotool use | Wayland situation |
|---|---|---|
| **XTest** (`libxtst`) | Inject fake keyboard and mouse events globally | `zwp_virtual_keyboard_v1`, `wlr-virtual-pointer` — exist but are **compositor-specific, not universally supported** |
| **EWMH atoms** (`_NET_*`) | Activate windows, switch desktops, change window state | **No standard equivalent.** KDE, GNOME, and wlroots each define their own extensions |
| **ICCCM** (`WM_CLASS`, `WM_NAME`, `WM_STATE`) | Window identity, querying, client-window detection | No global equivalent; `xdg-foreign` helps only for explicitly shared handles |
| **XQueryTree / XGetWindowAttributes** | Enumerate all windows on all screens | **Intentionally absent in Wayland** — security model forbids global window lists |
| **XSendEvent** | Deliver synthetic events to a specific window cross-process | No equivalent; compositor controls all event routing |
| **XKB** (keyboard layout introspection) | Map characters → keycodes + modifiers | `xkbcommon` works client-side and is fully reusable |
| **Xinerama** | Multi-monitor geometry | `wl_output` — actually cleaner in Wayland |

**Root cause of incompatibility:** X11 is a shared global namespace. Any client can enumerate
every window, inject input into any process, and send events cross-process. Wayland's security
model deliberately eliminates all of this — only the compositor has the global view, and input
injection requires explicit compositor cooperation via protocol extensions.

This is not a matter of missing libraries. The *capability* must be granted by the compositor,
and each compositor grants it differently (or not at all).

---

## 3. Core State: `xdo_t`

```c
// xdo.h:78–113
typedef struct xdo {
  Display *xdpy;               // X11 display connection — the entire design anchors here
  char    *display_name;
  charcodemap_t *charcodes;    // char → (keycode, group, modmask): built at init via XKB scan
  int      charcodes_len;
  int      keycode_high;       // keycode range reported by X server
  int      keycode_low;
  int      keysyms_per_keycode;
  int      close_display_when_freed;
  int      quiet;              // suppress stderr output (XDO_QUIET env)
  int      debug;              // verbose output (DEBUG env)
  int      features_mask;      // which extensions are available (XTEST, etc.)
} xdo_t;
```

`xdo_new()` (`xdo.c:86`) connects to the display, scans the full XKB keymap into `charcodes`,
and detects extension availability. Everything else is stateless function calls against this
context.

**For waydo:** the equivalent struct will hold a `wl_display *` plus per-capability protocol
object handles (virtual keyboard, virtual pointer, toplevel manager, etc.), and the set of
handles available will vary per compositor.

---

## 4. Keyboard Simulation — The Hardest Part

### How xdotool does it (`xdo.c:1044–1732`)

**Initialization** (`xdo.c:1326–1461`):

Scans every keycode × XKB group × shift level and builds `charcodemap_t[]`:

```
for each keycode in [keycode_low .. keycode_high]:
  for each XKB group:
    for each shift level:
      record: character → { keycode, group, modifier_mask }
```

**Sending a character:**

1. Look up the character in `charcodemap_t` → get keycode + required modifier mask + XKB group
2. If the current XKB group differs, switch to the required group
3. Activate any needed modifier keys (press them)
4. `XTestFakeKeyEvent(press)` → `XTestFakeKeyEvent(release)` — or `XSendEvent` if targeting an unfocused window
5. Deactivate modifiers, restore XKB group

**Sending a character with no keycode** (`xdo.c:1044–1114`):

For exotic Unicode characters not in the current keymap, xdotool dynamically borrows a keycode:

```
1. Find first empty keycode slot
2. XChangeKeyboardMapping() — bind the keysym to that slot
3. XSync() to flush
4. Send the key event using the scratch keycode
5. XChangeKeyboardMapping() — unbind immediately
6. XSync() again
```

### Critical edge cases

- **Capital letters** require explicit `ShiftMask` even if the letter is already uppercase
  (ASCII A–Z and extended Latin A–Ö, Ø–Þ). Checked at `xdo.c:1277–1283`.
- **XKB group switching**: the current group is saved before sending and restored after, to
  avoid disrupting the user's active layout.
- **Dynamic keycode binding has race conditions**: if another process reads the keymap between
  the bind and the send, it sees a briefly mutated keyboard. No lock protects this.
- **XTEST vs XSendEvent split** (`xdo.c:1635–1681`): XTEST is used when targeting the current
  window or a focused window; `XSendEvent` is used to target specific unfocused windows. Apps can
  detect `XSendEvent` events via the `send_event` flag and reject them — Firefox does this.
- **Modifier clearing**: held modifiers (e.g. user holding Ctrl) must be cleared before injection
  and restored after, or injected keys combine with them unintentionally.

### For waydo

`zwp_virtual_keyboard_v1` (wlroots) and `ext_virtual_keyboard_v1` work differently: you upload
a complete XKB keymap to the compositor, then send `(keycode, state)` events. There is no
dynamic keycode trick — the keymap must be pre-defined. The fragmentation challenge is that not
all compositors implement the same virtual keyboard protocol.

---

## 5. Mouse Simulation

### How xdotool does it (`xdo.c:794–981`)

**Absolute movement:**
- `XTestFakeMotionEvent` for screen 0
- `XWarpPointer` for any other screen — because `XTestFakeMotionEvent` ignores its screen
  parameter (documented bug, `xdo.c:797–809`)

**Window-relative clicks** (targeting a specific window):
- `XTranslateCoordinates` to convert window-local coords to screen coords
- `XSendEvent` with an `XButtonEvent` — **not** XTEST — so the window sees `send_event=True`

**Click timing:**
- Hardcoded 12ms delay between mousedown and mouseup
- Always sleeps after the last click in a sequence, even at `--delay 0`, to ensure button state
  is cleared

**Button motion masks:**
- Release events include `ButtonNMotionMask` in the state field to indicate which button was
  released (`xdo.c:864–875`)

**`--sync` for mouse moves** (`cmd_mousemove.c:156–185`):
- Polls in 30ms intervals until *any* movement is detected — not until the cursor reaches the
  target. Documented limitation: some apps lock the cursor to a region, so the target may never
  be reached.

### For waydo

`zwp_virtual_pointer_v1` (wlroots) or `wlr-virtual-pointer`. No per-window click targeting —
Wayland's security model prevents injecting synthetic pointer events into an arbitrary window.
The window must already have pointer focus, or a compositor extension must grant access.

---

## 6. Window Search

### How xdotool does it (`xdo_search.c`)

Search criteria (`xdo.h:177–203`):

```c
typedef struct xdo_search {
  const char *title;        // deprecated alias for winname
  const char *winclass;     // WM_CLASS class field
  const char *winclassname; // WM_CLASS name field
  const char *winname;      // window manager name (_NET_WM_NAME or WM_NAME)
  const char *winrole;      // WM_WINDOW_ROLE property
  int         pid;          // _NET_WM_PID
  long        max_depth;    // tree recursion depth (-1 = unlimited)
  int         only_visible; // MapState == IsViewable only
  int         screen;       // restrict to one screen
  enum { SEARCH_ANY, SEARCH_ALL } require;  // OR vs AND across criteria
  unsigned int searchmask;  // bitmask of active criteria
  long        desktop;      // _NET_WM_DESKTOP
  unsigned int limit;       // max results (0 = unlimited)
} xdo_search_t;
```

**Execution:**

1. `XQueryTree` recursively from each screen's root window
2. For each window, evaluate active criteria using POSIX regex
3. Windows without a given property match an empty-string pattern (important gotcha)
4. Locate the "real" client window by walking up/down the tree for `WM_STATE` — window manager
   decoration frames don't have `WM_STATE`, client windows do

**PID lookup** (`xdo_search.c:199–209`): reads `_NET_WM_PID` as `unsigned long`, casts to `int`.
Returns 0 if missing — and many apps don't set it.

### For waydo

No global window enumeration exists in Wayland by design. Options, in rough order of portability:

| Approach | Portability | Notes |
|---|---|---|
| `ext-foreign-toplevel-list` | Good (growing adoption) | Standard Wayland protocol extension |
| `zwlr_foreign_toplevel_management_v1` | wlroots compositors | Sway, Hyprland, etc. |
| `org_kde_plasma_window_management` | KDE only | |
| DBus (`org.gnome.Shell`) | GNOME only | |
| AT-SPI accessibility bus | Cross-compositor | Requires app support; heavy dependency |

waydo will likely need to implement several of these and select at runtime.

---

## 7. Window Management via EWMH

xdotool communicates with the window manager by sending `XClientMessage` events to the root
window with standard `_NET_*` atom names:

| EWMH atom | xdotool command |
|---|---|
| `_NET_ACTIVE_WINDOW` | `windowactivate` |
| `_NET_WM_STATE` | `windowstate` (maximize, minimize, etc.) |
| `_NET_CURRENT_DESKTOP` | `set_desktop` |
| `_NET_WM_DESKTOP` | `set_desktop_for_window` |
| `_NET_WM_NAME` | `set_window --name` |

All of these require an EWMH-compliant window manager. On bare X11 with no WM, or with a
non-EWMH WM (e.g. twm), these operations silently fail or have no effect.

### For waydo

There is no EWMH for Wayland. The compositor-specific situation:

| Compositor family | Window management protocol |
|---|---|
| wlroots (Sway, Hyprland, etc.) | `zwlr_foreign_toplevel_management_v1` |
| KDE Plasma | `org_kde_plasma_window_management` |
| GNOME Shell | DBus `org.gnome.Shell.Extensions` / `org.gnome.Mutter.DisplayConfig` |
| Generic / emerging | `ext-foreign-toplevel-v1` (still stabilizing) |

This fragmentation is the single biggest architectural challenge for waydo. A backend
abstraction layer with runtime capability detection is essential.

---

## 8. The Window Stack (CLI Layer)

xdotool maintains a stack of window IDs across chained commands. This allows patterns like:

```sh
xdotool search --name "Firefox" windowmove 100 200
xdotool search --name ".*" windowfocus %1 key ctrl+c
```

- `%1` — first window on the stack
- `%-1` — last window on the stack
- `%@` — all windows on the stack (iterates)
- Commands consume from the stack and/or push new results onto it

This is a pure CLI design pattern with no dependency on X11. It can be lifted directly into
waydo unchanged. The `window_each()` macro in `xdotool.c` implements the iteration.

---

## 9. Edge Cases and Gotchas

These are issues discovered in the xdotool source that will need parallel solutions in waydo.

| Issue | Source location | Notes for waydo |
|---|---|---|
| Symbol typing broken under non-US keyboard layouts | `xdotool.pod:1214` | Wayland virtual keyboard requires uploading an explicit keymap; layout must be negotiated with compositor |
| `XSendEvent` events are detectable and rejectable by apps | `xdo.c:1960` | Wayland virtual keyboard/pointer events are also synthetic and detectable |
| `windowactivate --sync` polls at 30ms × 500 tries = 15s max | `xdo.c:44` | Make timeout configurable from day one; fixed polling is a design smell |
| `mousemove --sync` waits for *any* movement, not the target position | `xdotool.pod:175` | Cursor locking to screen regions is a real scenario; document the same limitation |
| Apps that ignore synthetic input when not focused (Firefox) | `xdotool.pod:1086` | Wayland makes this worse: no cross-window injection at all without compositor extension |
| `_NET_WM_PID` is often not set | `xdo_search.c:199` | All Wayland alternatives for PID-based search are similarly unreliable |
| Dynamic keycode binding has race conditions under concurrent input | `xdo.c:1044` | Not applicable to Wayland virtual keyboard protocol; design around it |
| `XTestFakeMotionEvent` ignores screen number (X server bug) | `xdo.c:797` | Wayland has per-output coordinate spaces; design the coordinate model carefully from the start |
| `SEARCH_TITLE` silently redirected to match window name, not WM_CLASS | `xdo_search.c:83` | Document search field semantics clearly; avoid silent aliasing |
| Empty property windows match empty-string patterns | `xdo_search.c:112` | Decide explicitly whether unset properties should match, not-match, or error |
| `windowstate` (maximize/minimize) unreliable with some WMs | xdotool.pod | Compositor support for state changes varies; test matrix needed |
| Mouse click 12ms delay is hardcoded | `xdo.c:978` | Expose timing as a configurable option |

---

## 10. What waydo Must Solve Differently

### Architecture

1. **Backend abstraction layer**: define a single internal interface for input injection and
   window management, then implement it per compositor family. Runtime detection selects the
   backend.

2. **Capability detection**: on startup, probe which Wayland protocol extensions the compositor
   advertises. Degrade gracefully and surface clear errors for missing capabilities rather than
   silent failures.

3. **No global window enumeration**: this is a hard constraint. waydo must either require a
   compositor that supports `ext-foreign-toplevel-list` / `zwlr_foreign_toplevel_management_v1`,
   or use AT-SPI as a fallback, or simply document that search commands require compositor support.

### Keyboard

4. **Upload, don't discover**: unlike xdotool's dynamic keycode binding trick, Wayland virtual
   keyboard requires uploading a complete keymap. waydo must either generate a synthetic keymap
   that covers all needed keysyms, or upload the user's current keymap and map characters into it.

5. **No global modifier state**: xdotool can read and clear currently-held modifiers via X11.
   Wayland gives no such view; waydo must track modifier state it has injected itself.

### Mouse and Clicks

6. **No per-window synthetic clicks**: targeting an arbitrary unfocused window with synthetic
   clicks is not possible in Wayland without compositor extension. Document this clearly, and
   design commands around focus-based interaction.

### Window Management

7. **Per-compositor EWMH equivalents**: implement window activation, desktop switching, and
   state changes for each supported compositor backend. Use a common interface so commands stay
   backend-agnostic.

### Timing and Sync

8. **Configurable timeouts**: xdotool's hardcoded 30ms × 500 polling is a recurring pain point.
   waydo should expose `--timeout` globally and use Wayland's event-driven model (fd polling)
   rather than fixed-interval polling wherever possible.
