#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

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
  {"^-?[1-9][0-9]*", TK_DEC},
  {"\\+", '+'},         // plus
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"==", TK_EQ},         // equal
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
	if (nr_token > 32) {
	  Log("Too many tokens");
	  return false;
	}

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

bool check_parentheses(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')') {
    return false;
  }
  
  //成对括号中间至少有一个token
  assert(q - p >= 2);

  int iStackSize = 0;
  for (int i = p + 1; i <= q - 1; i++) {
    if (tokens[i].type == '(') {
      iStackSize++;
    }
    if (tokens[i].type == ')') {
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

int main_op(int p, int q) {
  int op_stack[32] = { -1 };
  int top = 0;  
  for (int i = p; i <= q; i++) {
      if (tokens[i].type != '+' && tokens[i].type != '-' &&
	  tokens[i].type != '*' && tokens[i].type != '/') {
	continue;
      }
      //在一对括号中吗？
      bool bInParens = false;
      for (int j = i; j >= p && tokens[j].type != ')'; j--) {
	if (tokens[j].type == '(') {
	  bInParens = true;
	  break;
	}
      }
      if (bInParens) {
	continue;
      }
      op_stack[top++] = i;
    }
  
  int op = -1;  
  if (top > 0) { 
    op = op_stack[top - 1];
    for (int i = top - 1; i >= 0; i--) {
      if (tokens[op_stack[i]].type == '+' || tokens[op_stack[i]].type == '-') {
	op = op_stack[i];
	break;
      }
    }
  }
  return op;
}

uint32_t eval(int p, int q) {
  if (p > q) {
    Assert(0, "Bad expression");
    return 0;
  }
  else if (p == q) {
    /* Single token */
    Log("Single token at %d", p);
    return atoi(tokens[p].str);
  }
  else if (tokens[p].type == TK_NOTYPE) {
    Log("remove whitespace at %d", p);
    return eval(p + 1, q);
  }
  else if (tokens[q].type == TK_NOTYPE) {
    Log("remove whitespace at %d", q);
    return eval(p, q - 1);
  }
  else if (check_parentheses(p, q) == true) {
    /* surrounded by a matched pair of parentheses */
    Log("remove a pair of parentheses at %d and %d", p, q);
    return eval(p + 1, q - 1);
  }
  else { //寻找主运算符
    int op = main_op(p, q);
    assert(p <= op && op < q);
    int op_type = tokens[op].type;
    Log("main operator %c at %d", op_type, op);
    if (op_type == '-' && op == p) {
      Log("unary minus at %d", op);
      return -eval(op + 1, q);
    }
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);
    
    Log("%u %c %u", val1, op_type, val2);
    switch (op_type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: Assert(0, "Unknown op type!");
    }
  }
}

bool check_expr(int p, int q) {
  assert(p <= q);
  if (p == q) {
    if (tokens[p].type == TK_DEC) {
      Log("Single number at %d", p);
      return true;
    }
    else {
      Log("Illegal expr at %d", p);
      return false;
    }
  }
  if (tokens[p].type == '(' && tokens[q].type == ')') {
    Log("A pair of parens at (%d, %d)", p, q);
    return check_expr(p + 1, q - 1);
  }
  if (tokens[p].type == TK_NOTYPE) {
    Log("Left whitespace at %d", p);
    return check_expr(p + 1, q);
  }
  if (tokens[q].type == TK_NOTYPE) {
    Log("Right whitespace at %d", q);
    return check_expr(p, q - 1);
  }
  int op = main_op(p, q);
  if (op == -1) {
    Log("cannot find main operator");
    return false;
  }
  if (op == p) {
    Log("unary minus at %d", op);
    return (tokens[op].type == '-') && check_expr(op + 1, q);
  }
  else {
    return check_expr(p, op - 1) && check_expr(op + 1, q);
  }
  return false;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    Log("make_token: %s failed", e);
    *success = false;
    return 0;
  }
  if (!check_expr(0, nr_token - 1)) {
    Log("Illegal expression");
    *success = false;
    return  0;
  }
  
  //保证表达式合法 
  return eval(0, nr_token - 1);
}
