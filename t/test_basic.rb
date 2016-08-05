#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolBasicTests < MiniTest::Test
  include XdoTestHelper

  def test_noargs_exit_1
    status, lines = xdotool ""
    assert_equal(1, status, "Exit status for xdotool with no args must be 1")
  end # def test_noargs_exit_1

  def test_getwindowfocus_gets_a_valid_window
    # There's a bug in Xvfb, Xephyr, and possibly other X servers that if you
    # just run the X server, no WM, and launch an xterm (or nothing),
    # XGetInputFocus yields '1' as the window with focus. This is a bug we
    # work-around by telling the X server to focus our window.
    xdotool "windowfocus --sync #{@wid}"
    status, lines = xdotool "getwindowfocus -f"
    assert_status_ok(status)
    assert_equal(1, lines.length, "Only one line of output expected from getwindowfocus")
    assert_equal(@wid, lines[0].to_i)
  end

  def test_windowraise_succeeds_on_valid_window
    status, lines = xdotool "windowraise #{@wid}"
    assert_status_ok(status)
    assert_equal(0, lines.length, "No output from windowraise")
  end

  def test_windowraise_fails_on_invalid_window
    status, lines = xdotool "windowraise 1 2> /dev/null"
    assert_status_fail(status)
  end

  def test_windowsize_by_pixel_works
    w = 500
    h = 400
    status, lines = xdotool_ok "windowsize --sync #{@wid} #{w} #{h}"
    assert_equal(0, lines.length, "No output expected from windowsize")

    status, xwininfo_output = runcmd "xwininfo -id #{@wid}"
    reported_width = xwininfo_output.grep(/  Width:/).first[/[0-9]+/].to_i
    reported_height = xwininfo_output.grep(/  Height:/).first[/[0-9]+/].to_i
    assert_status_ok(status)

    # Some windowmanagers don't strictly obey window size changes to the pixel.
    # So, let's give a tolerance of 10 pixels for these values
    assert_in_delta(w, reported_width, 20, "Expected width was not correct");
    assert_in_delta(h, reported_height, 20, "Expected height was not correct");
  end

  def test_windowsize_by_size_hints
    w = 120
    h = 50
    status, lines = xdotool "windowsize --usehints #{@wid} #{w} #{h}"
    assert_status_ok(status)
    assert_equal(0, lines.length, "No output expected from windowsize")

    try do
      status, xwininfo_output = runcmd "xwininfo -id #{@wid}"
      assert_status_ok(status)
      geometry = xwininfo_output.grep(/^ *-geometry /)[0][/[0-9]+x[0-9]+/]
      assert_equal("#{w}x#{h}", geometry,
             "Expected xwininfo to report geometry of #{w}x#{h}, got #{geometry}")
    end
  end

  def test_windowfocus
    #status, lines = runcmd("xdpyinfo")
    #rootwin = ("%d" % (lines.grep(/root window id:/).first[/0x[0-9A-Ea-e]+/])).to_i

    try do
      status, lines = xdotool "windowfocus #{@wid}"
      assert_status_ok(status)
      assert_equal(0, lines.length, "windowfocus should have no output")
    end

    try do
      status, lines = xdotool "getwindowfocus -f"
      assert_status_ok(status)
      assert_equal(1, lines.length, "getwindowfocus should have one line of output")
      assert_equal(@wid, lines.first.to_i,
                   "Our (#{@wid}) window should be focused")
    end
  end

  def test_windowmove
    x = 300
    y = 400
    status, lines = xdotool "windowmove #{@wid} #{x} #{y}"
    assert_status_ok(status)
    assert_equal(0, lines.length, "windowmove should have no output")

    try do
      status, lines = runcmd("xwininfo -id #{@wid}")
      reported_x = lines.grep(/Absolute upper-left X:/).first[/[0-9]+/].to_i
      reported_y = lines.grep(/Absolute upper-left Y:/).first[/[0-9]+/].to_i
      # Some windowmanagers don't strictly obey window size changes to the pixel.
      # So, let's give a tolerance of 10 pixels for these values
      # The reason for the is likely due to window borders and titlebars drawn
      # by window managers.
      assert_in_delta(x, reported_x, 30,
                      "Reported X coordinate expected to be near #{x} (+- 30 pixels)")
      assert_in_delta(y, reported_y, 30,
                      "Reported Y coordinate expected to be near #{y} (+- 30 pixels)")
    end
  end

  def test_windowmapping
    try do
      status, lines = xdotool "windowunmap #{@wid}"
      assert_status_ok(status)
      assert_equal(0, lines.length, "windowunmap should have no output")
    end

    try do
      status, lines = runcmd("xwininfo -id #{@wid}")
      state = lines.grep(/Map State: /).first[/Is(UnMapped|Viewable)/]
      assert_equal("IsUnMapped", state);
    end

    # Now map it again
    status, lines = xdotool "windowmap #{@wid}"
    assert_status_ok(status)
    assert_equal(0, lines.length, "windowmap should have no output")
 
    try do
      status, lines = runcmd("xwininfo -id #{@wid}")
      state = lines.grep(/Map State: /).first[/Is(UnMapped|Viewable)/]
      assert_equal("IsViewable", state);
    end
  end

  def test_misc
    cmds = ["mousedown 1", "mouseup 1", "mousemove 0 0", "mousemove 50 50",
            "click 1", "key \"ctrl+w\""]
    #"type \"hello\"",
    cmds_withoutput = []

    # Try to focus our window in case it's not already. This works around
    # a bug in Xvfb that says window '1' is focused when it is not (at first).
    xdotool_ok "windowfocus --sync #{@wid}"
    if (wm_supports?("_NET_ACTIVE_WINDOW"))
      cmds << "windowactivate #{@wid}"
      if (detect_window_manager != :none)
        cmds_withoutput << "getwindowfocus"
      end
    else
      puts "Skipping _NET_ACTIVE_WINDOW features (current wm does not support it)"
    end

    if (wm_supports?("_NET_NUMBER_OF_DESKTOPS"))
      cmds << "set_num_desktops 5"
      cmds_withoutput << "get_num_desktops"
    else
      puts "Skipping _NET_NUMBER_OF_DESKTOPS features (current wm does not support it)"
    end

    desktop = 3
    if (wm_supports?("_NET_WM_DESKTOP"))
      cmds << "set_desktop_for_window #{@wid} #{desktop}"
      cmds_withoutput << "get_desktop"
    else
      puts "Skipping _NET_WM_DESKTOP features (current wm does not support it)"
    end

    if (wm_supports?("_NET_CURRENT_DESKTOP"))
      cmds << "set_desktop #{desktop}"
      cmds_withoutput << "get_desktop_for_window #{@wid}"
    else
      puts "Skipping _NET_CURRENT_DESKTOP features (current wm does not support it)"
    end

    cmds.each do |cmd|
      status, lines = xdotool cmd
      assert_status_ok(status, cmd)
      assert_equal(0, lines.length, "'#{cmd}' should have no output")
    end

    cmds_withoutput.each do |cmd|
      status, lines = xdotool cmd
      assert_status_ok(status, cmd)
      assert_equal(1, lines.length, "'#{cmd}' should have one line of output")
    end
  end

  def test_xdotool_exits_failure_with_bad_flags
    commands = %w{getactivewindow getwindowfocus getwindowpid search click
                  getmouselocation key keydown keyup mousedown mousemove
                  mousemove_relative mouseup type windowactivate windowfocus
                  windowmap windowmove windowraise windowsize windowunmap
                  set_window set_num_desktops get_num_desktops set_desktop
                  get_desktop set_desktop_for_window get_desktop_for_window
    }

    commands.each do |command|
      status, lines = xdotool "#{command} --some-invalid-flag"
      assert_not_equal(0, status,
                       "Expected failing exit status (!= 0), got #{status}")
    end
  end # def test_xdotool_exits_failure_with_bad_flags

  def test_xdotool_exits_success_with_help_flag
    commands = %w{getactivewindow getwindowfocus getwindowpid search click
                  getmouselocation key keydown keyup mousedown mousemove
                  mousemove_relative mouseup type windowactivate windowfocus
                  windowmap windowmove windowraise windowsize windowunmap
                  set_window set_num_desktops get_num_desktops set_desktop
                  get_desktop set_desktop_for_window get_desktop_for_window
    }
    commands.each do |command|
      xdotool_ok "#{command} --help"
    end
  end
end

