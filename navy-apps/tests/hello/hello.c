#include <unistd.h>
#include <stdio.h>

int main() {
  // printf("1\n");
  write(1, "Hello World!\n", 13);
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000) {
      printf("2\n");
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      j = 0;
    }
  }
  return 0;
}
