#include "monitor/expr.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);
  
  /*FILE* fin = fopen("../tools/gen-expr/input", "r");*/
  /*assert(fin != NULL);*/
  /*int result;*/
  /*FILE* fout = fopen("./output", "w");*/
  /*while (fscanf(fin, "%u ", &result) != 0) {*/
    /*char e[65536];*/
    /*fgets(e, 65536, fin);*/
    /*e[strlen(e) - 1] = 0;*/
    /*bool success;*/
    /*if (result != expr(e, &success)) {*/
      /*fprintf(fout, "%u %s", result, e);*/
    /*}*/
  /*}*/
  /*fclose(fin);*/
  /*fclose(fout);*/

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);
  
  return 0;
}
