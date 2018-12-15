#include "monitor/expr.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  /*int is_batch_mode = init_monitor(argc, argv);*/

  /* Receive commands from user. */
  /*ui_mainloop(is_batch_mode);*/
  
  FILE* fp = fopen("../tools/gen-expr/input", "r");
  assert(fp != NULL);
  int result;
  while (fscanf(fp, "%u ", &result) != 0) {
    char e[65536];
    fgets(e, 65536, fp);
    e[strlen(e) - 1] = 0;
    bool success;
    if (result != expr(e, &success)) {
      printf("%u %s", result, e);
    }
  }
  fclose(fp);
  return 0;
}
