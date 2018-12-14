#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DEC,

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"^-?[1-9][0-9]*", TK_DEC},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;
	
	if (substr_len >= 32)
	{
	  assert(0);
	  return false;
	}

        switch (rules[i].token_type) {
          default: tokens[nr_token].type = rules[i].token_type;
		   strncpy(tokens[nr_token].str, substr_start, substr_len);
		   tokens[nr_token].str[substr_len] = '\0';
        }
	
	nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(const char* e, int p, int q) {
  if (e[p] != '(' || e[q] != ')') {
    return false;
  }

  int iStackSize = 0;
  for (int i = p + 1; i < q; i++) {
    if (e[i] == '(') {
      iStackSize++;
    }
    if (e[i] == ')') {
      iStackSize--;
    }

    if (iStackSize < 0) {
      return false;
    }    
  }

  if (iStackSize != 0) {
    return false;
  }
  return true;
}

uint32_t eval(const char* e, int p, int q) {
  if (p > q) {
    Assert(0, "Bad expression");
    return 0;
  }
  else if (p == q) {
    /* Single token */
    return e[p] - '0';
  }
  else if (check_parentheses(e, p, q) == true) {
    /* surrounded by a matched pair of parentheses */
    return eval(e, p + 1, q - 1);
  }
  else {
    int op = 0;
    for (int i = 0, pos = 0; i < nr_token; i++, pos += strlen(tokens[i].str)) {
      if (pos > q) {
	Assert(0, "failed to find main operator");
	return 0;
      }      
      if (pos < p) {
	continue;
      }

      assert(p <= pos && pos <= q);

      if (tokens[i].type != '+' && tokens[i].type != '-' &&
	  tokens[i].type != '*' && tokens[i].type != '/') {
	continue;
      }
      for (int j = pos; j >= p && e[j] != ')'; j--) {
	if (e[j] == '(') {
	  continue;
	}
      }
      //后面有没有优先级更低的？
      if (tokens[i].type == '*' || tokens[i].type == '/') {
	int iStackSize = 0;
	bool bHasLower = false;
	for (int j = pos; j <= q; j++) {
	  if (e[j] == '(') {
	    iStackSize++;
	  }
	  if (e[j] == ')') {
	    iStackSize--;
	  }
	  assert(iStackSize >= 0);
	  if (iStackSize == 0 && (e[j] == '+' || e[j] == '-')) {
	    bHasLower = true;
	    break;
	  }
	}
	if (bHasLower) {
	  continue;
	}
      }

      break;
    }
    
    assert(p < op && op < q);
    Log("main operator at %d", op);

    int op_type = e[op];
    uint32_t val1 = eval(e, p, op - 1);
    uint32_t val2 = eval(e, op + 1, q);

    switch (op_type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: Assert(0, "Uknown op type!");
    }
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    Log("make_token: %s failed", e);
    *success = false;
    return 0;
  }

  return eval(e, 0, strlen(e) - 1);
}
