#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char *args);
static int cmd_x(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute the program by n (default 1) steps and stop", cmd_si },
  { "info", "Print the value of register r or the info of watchpoint w", cmd_info },
  { "p", "Evaluate expression expr", cmd_p },
  { "x", "Evaluate expression expr and print n doublewords of memory addressed from expr in hex", cmd_x },
  { "w", "Stop the execution when the value of expr changes", cmd_w },
  { "d", "delete the watchpoint of index n", cmd_d },

};

static int cmd_si(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int n = 1;

  if (arg != NULL) {
    n = atoi(arg);
  }

  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  /* extract the first argument */
  char* arg = strtok(NULL, " ");
  int i;
  
  if (strcmp(arg, "r") == 0) {
    for (i = R_EAX; i <= R_EDI; i++) {
      printf("%-10s0x%-15x%u\n", regsl[i], reg_l(i), reg_l(i));
    }
    for (i = R_AX; i <= R_DI; i++) {
      printf("%-10s0x%-15x%u\n", regsw[i], reg_w(i), reg_w(i));
    }
    for (i = R_AL; i <= R_BH; i++) {
      printf("%-10s0x%-15x%u\n", regsb[i], reg_b(i), reg_b(i));
    }
  }
  else if (strcmp(arg, "b") == 0) {

    printf("Num     Type          Disp Enb Address          What\n");
    
  }
  else {
    printf("Unknown subcommand '%s'\n", arg);
  }
  
  return 0;
}

static int cmd_p(char *args) {
  return 0;
}

static int cmd_x(char *args) {
  const char* arg1 = strtok(args, " ");
  const char* arg2 = strtok(NULL, " ");

  /* TODO: calc the expr */
  printf("%s\n", arg2);
  paddr_t addr = atoi(arg2);

  int i;
  int n = atoi(arg1);
  for (i = 0; i < n; i+=4) {
    if (i + 4 < n) {      
      printf("0x%-10x : 0x%-15x0x%-15x0x%-15x0x%-15x\n", addr, paddr_read(addr, 4), 
	  paddr_read(addr + 4, 4), paddr_read(addr + 8, 4), paddr_read(addr + 12, 4));
    }
    else {
      int j;
      printf("0x%-10x : ", addr);
      for (j = 0; j < n - i; j++) {	
	printf("0x%-15x", paddr_read(addr, 4));
	addr += 4;
      }
      printf("\n");
    }

    addr += 16;
  }

  return 0;
}

static int cmd_w(char *args) {
  return 0;
}

static int cmd_d(char *args) {
  return 0;
}

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
