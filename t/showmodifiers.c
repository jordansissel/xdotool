/* Build with:
 * gcc -I.. -lX11 -lXtst showmodifiers.c
 */

/* Yes, I know including .c files is insanity. */
#include "xdo.c"

int main(int argc, char **argv) {
  xdo_t *xdo = NULL;
  charcodemap_t *keys = NULL;
  int nkeys = 0;
  xdo = xdo_new(NULL);

  while (1) {
    int i = 0;
    xdo_active_modifiers_to_keycode_list(xdo, &keys, &nkeys);
    if (nkeys > 0) {
      for (i = 0; i < nkeys; i++) {
        printf("%d, ", keys[i].code);
      }
      printf("\n");
    }
    free(keys);
  }
  xdo_free(xdo);
}
