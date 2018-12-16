#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define MAX_BUF_SIZE 65535

// this should be enough
static char buf[65536];
static const int max_whitespace_num = 5;

static inline uint32_t choose(uint32_t n) {
  return rand() % n;
}

void gen_rand_whitespace() { //随机插入空格
  for (int i = 0; i <= choose(max_whitespace_num); i++) {
    sprintf(buf + strlen(buf), " ");
  }
}

static inline void gen_rand_expr();
void gen(char* str) {
  //如果将要溢出，则重来
  if (strlen(buf) + strlen(str) + 5 > MAX_BUF_SIZE) {
    memset(buf, 0, sizeof(buf));
    gen_rand_expr();
    return;
  }
  sprintf(buf + strlen(buf), "%s", str);
  gen_rand_whitespace();
}

void gen_num() {
  char strNum[15];
  sprintf(strNum, "%d", rand());
  gen(strNum);
}

void gen_rand_op() {
  switch (choose(4)) {
    case 0: gen("+"); break;
    case 1: gen("-"); break;
    case 2: gen("*"); break;
    default: gen("/"); break;
  }
}

static inline void gen_rand_expr() {
  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen("("); gen_rand_expr(); gen(")"); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    memset(buf, 0, sizeof(buf));
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc .code.c -o .expr -Werror=div-by-zero");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
