#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DEC, TK_HEX, TK_REG, TK_NEQ, TK_AND, TK_DEREF, TK_UMINUS,

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {


  {" +", TK_NOTYPE},    // spaces
  {"^[1-9][0-9]*", TK_DEC},
  {"^(0x|0X)[0-9,a-z,A-Z]+", TK_HEX},
  {"^\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|"
    "ax|cx|dx|bx|sp|bp|si|di|"
    "al|cl|dl|bl|ah|ch|dh|bh)", TK_REG},
  {"\\+", '+'},         // plus
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"==", TK_EQ},         // equal
  {"!=", TK_NEQ},
  {"&&", TK_AND},
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

#define max_token_num 65536

Token tokens[max_token_num];
int nr_token;

typedef struct paren {
  int left, right;
} Paren;

Paren parens[max_token_num / 2];
int nr_paren;

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

        /*Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",*/
            /*i, rules[i].regex, position, substr_len, substr_len, substr_start);*/
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
	if (nr_token > max_token_num) {
	  /*Log("Too many tokens");*/
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

  //匹配括号
  int stack[max_token_num];
  int top = 0;
  nr_paren = 0;
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '(') {
      stack[top++] = i;
    }
    if (tokens[i].type == ')') {
      assert(top >= 0);
      if (top == 0) {
	printf("unmatched right paren at %d\n", i);
	return false;
      }
      parens[nr_paren].left = stack[top - 1];
      parens[nr_paren].right = i;
      nr_paren++;
      stack[--top] = -1;
    }
  }
  if (top != 0) {
    printf("number of left paren and right paren not equal\n");
    return false;
  }

  return true;
}

bool check_parentheses(int p, int q) {
  for (int i = 0; i < nr_paren; i++) {
    if (parens[i].left == p && parens[i].right == q) {
      return true;
    }
  }  
  return false;
}

int main_op(int p, int q) {
  int op_stack[max_token_num] = { -1 };
  int top = 0;  
  for (int i = p; i <= q; i++) {
      if (tokens[i].type != '+' && tokens[i].type != '-' &&
	  tokens[i].type != '*' && tokens[i].type != '/' &&
	  tokens[i].type != TK_EQ && tokens[i].type != TK_NEQ &&
	  tokens[i].type != TK_AND && tokens[i].type != TK_DEREF &&
	  tokens[i].type != TK_UMINUS) {
	continue;
      }
      //在一对括号中吗？
      bool bInParens = false;
      for (int j = 0; j < nr_paren; j++) {
	assert(check_parentheses(p, q) == false);
	if (p <= parens[j].left && parens[j].left < i && 
	    i < parens[j].right && parens[j].right <= q) {
	  bInParens = true;
	}
      }
      if (bInParens) {
	continue;
      }
      op_stack[top++] = i;
    }
  
  int op = -1;  
  if (top > 0) { 
    int priority = 4;
    for (int i = top - 1; i >= 0; i--) {
      if (tokens[op_stack[i]].type == TK_AND && priority > -1) {
	op = op_stack[i];
	priority = -1;
      }
      if ((tokens[op_stack[i]].type == TK_EQ || tokens[op_stack[i]].type == TK_NEQ) && 
	  priority > 0) {
	op = op_stack[i];
	priority = 0;
      }
      if ((tokens[op_stack[i]].type == '+' || tokens[op_stack[i]].type == '-') &&
	  priority > 1) {
	op = op_stack[i];
	priority = 1;
      }
      if ((tokens[op_stack[i]].type == '*' || tokens[op_stack[i]].type == '/') &&
	  priority > 2) {
	op = op_stack[i];
	priority = 2;
      }
      if ((tokens[op_stack[i]].type == TK_DEREF || tokens[op_stack[i]].type == TK_UMINUS) &&
	  priority > 3) {
	op = op_stack[i];
	priority = 3;
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
    /*Log("Single token at %d", p);*/
    if (tokens[p].type == TK_HEX) {
      return strtol(tokens[p].str, NULL, 16);
    }
    if (tokens[p].type == TK_DEC) {
      return atoi(tokens[p].str);
    }
    if (tokens[p].type == TK_REG) {
      for (int i = R_EAX; i <= R_EDI; i++) {
	if (strcmp(tokens[p].str + 1, regsl[i]) == 0) {
	  return reg_l(i);
	}
      }
      for (int i = R_AX; i <= R_DI; i++) {
	if (strcmp(tokens[p].str + 1, regsw[i]) == 0) {
	  return reg_w(i);
	}
      }
      for (int i = R_AL; i <= R_BH; i++) {
	if (strcmp(tokens[p].str + 1, regsb[i]) == 0) {
	  return reg_b(i);
	}
      }
      if (strcmp(tokens[p].str + 1, "eip") == 0) {
	return cpu.eip;
      }
      assert(0);      
    }
    //unreachable
    assert(0);
  }
  else if (tokens[p].type == TK_NOTYPE) {
    /*Log("remove whitespace at %d", p);*/
    return eval(p + 1, q);
  }
  else if (tokens[q].type == TK_NOTYPE) {
    /*Log("remove whitespace at %d", q);*/
    return eval(p, q - 1);
  }
  else if (check_parentheses(p, q) == true) {
    /* surrounded by a matched pair of parentheses */
    /*Log("remove a pair of parentheses at %d and %d", p, q);*/
    return eval(p + 1, q - 1);
  }
  else { //寻找主运算符
    int op = main_op(p, q);
    assert(p <= op && op < q);
    int op_type = tokens[op].type;
    /*Log("main operator %c at %d", op_type, op);*/
    if (op_type == TK_UMINUS) {
      /*Log("unary minus at %d", op);*/
      assert(op == p);
      return -eval(op + 1, q);
    }
    if (op_type == TK_DEREF) {
      /*Log("unary deref at %d", op);*/
      assert(op == p);
      return vaddr_read(eval(op + 1, q), 4);
    }
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);
    
    /*Log("%u %c %u", val1, op_type, val2);*/
    switch (op_type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      default: Assert(0, "Unknown op type!");
    }
  }
}

bool check_expr(int p, int q) {
  assert(p <= q);
  if (p == q) {
    if (tokens[p].type == TK_DEC) {
      /*Log("Single number at %d", p);*/
      return true;
    }
    else {
      /*Log("Illegal expr at %d", p);*/
      return false;
    }
  }
  if (check_parentheses(p, q) == true) {
    /*Log("A pair of parens at (%d, %d)", p, q);*/
    return check_expr(p + 1, q - 1);
  }
  if (tokens[p].type == TK_NOTYPE) {
    /*Log("Left whitespace at %d", p);*/
    return check_expr(p + 1, q);
  }
  if (tokens[q].type == TK_NOTYPE) {
    /*Log("Right whitespace at %d", q);*/
    return check_expr(p, q - 1);
  }
  int op = main_op(p, q);
  if (op == -1) {
    /*Log("cannot find main operator");*/
    return false;
  }
  if (op == p) {
    /*Log("unary minus at %d", op);*/
    return (tokens[op].type == '-') && check_expr(op + 1, q);
  }
  else {
    return check_expr(p, op - 1) && check_expr(op + 1, q);
  }
  return false;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    /*Log("make_token: %s failed", e);*/
    *success = false;
    return 0;
  }
  for (int i = 0; i < nr_token; i++) {
    int prev = i - 1; 
    while (prev >= 0 && tokens[prev].type == TK_NOTYPE) prev--;
    if (i == 0 || tokens[prev].type == '(' || tokens[prev].type == '+' || 
	tokens[prev].type == '-' || tokens[prev].type == '*' ||
	tokens[prev].type == '/' || tokens[prev].type == TK_EQ || 
	tokens[prev].type == TK_NEQ || tokens[prev].type == TK_AND) {
      if (tokens[i].type == '-') {
	tokens[i].type = TK_UMINUS;
      }
      if (tokens[i].type == '*') {
	tokens[i].type = TK_DEREF;
      }
    }
  }

  /*if (!check_expr(0, nr_token - 1)) {*/
    /*Log("Illegal expression");*/
    /**success = false;*/
    /*return  0;*/
  /*}*/
 
  //保证表达式合法 
  return eval(0, nr_token - 1);
}
