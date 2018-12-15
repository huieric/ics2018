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
    int op = 0;
    for (int i = p + 1; i <= q - 1; i++) {
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
      //后面有优先级更低的吗？
      if (tokens[i].type == '*' || tokens[i].type == '/') {
	bool bHasLower = false;
	int iStackSize = 0;
	for (int j = i; j <= q; j++) {
	  if (tokens[j].type == '(') {
	    iStackSize++;
	  }
	  if (tokens[j].type == ')') {
	    iStackSize--;
	  }
	  assert(iStackSize >= 0);
	  if (iStackSize == 0 && (tokens[j].type == '+' || tokens[j].type == '-')) {
	    bHasLower = true;
	    break;
	  }
	}
	if (bHasLower) {
	  continue;
	}
      }

      op = i;
      break;
    }      

    assert(p <= op && op < q);

    int op_type = tokens[op].type;
    Log("main operator %c at %d", op_type, op);
    uint32_t val2 = eval(op + 1, q);
    if (op_type == '-' && op == p) {
      Log("unary minus at %d", op);
      return -val2;
    }
    uint32_t val1 = eval(p, op - 1);

    switch (op_type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: Assert(0, "Unknown op type!");
    }
  }
}

bool check_expr() {
  //括号检查
  //1. 括号要成对、匹配
  //2. 成对的括号中间至少有一个token
  int stack[32] = { -1 };
  int top = 0;
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '(') {
      stack[top++] = i;
    }
    if (tokens[i].type == ')') {
      assert(top >= 0);
      if (top == 0) {
	Log("unmatched right paren at %d", i);
	return false;
      }      
      if (i - stack[top - 1] <= 1) {
	Log("At least one token should be in the pair of parens at (%d, %d)",
	    stack[top - 1], i);
	return false;
      }
      stack[--top] = -1;
    }
  }
  if (top != 0) {
    Log("number of left parens and right parens not equal");
    return false;
  }
  
  //运算符检查
  //1. 算术运算符相邻的两个token必须是数字（除去空格）
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '+' ||
	tokens[i].type == '*' || tokens[i].type == '/') {
      int left = i - 1;
      while (left >= 0 && (tokens[left].type == TK_NOTYPE || 
			   tokens[left].type == '(' ||
			   tokens[left].type == ')')) {
	left--;
      }
      if (left < 0 || tokens[left].type != TK_DEC) {
	Log("Leftmost token of operator %c at %d should be a number", 
	    tokens[i].type, i);
	return false;
      }
      
    }
    if (tokens[i].type == '+' || tokens[i].type == '-' || 
	tokens[i].type == '*' || tokens[i].type == '/') {
      int right = i + 1;
      while (right < nr_token && (tokens[right].type == TK_NOTYPE || 
				  tokens[right].type == '(' ||
				  tokens[right].type == ')')) {
	right++;
      }
      if (right >= nr_token || tokens[right].type != TK_DEC) {
	Log("Rightmost token of operator %c at %d should be a number",
	    tokens[i].type, i);
	return false;
      }
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    Log("make_token: %s failed", e);
    *success = false;
    return 0;
  }
  if (!check_expr()) {
    Log("Illegal expression");
    *success = false;
    return  0;
  }
  
  //保证表达式合法 
  return eval(0, nr_token - 1);
}
