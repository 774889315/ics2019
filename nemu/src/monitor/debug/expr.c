#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ = '=',

  /* TODO: Add more token types */
  TK_PLUS = '+',
  TK_MINUS = '-',
  TK_MUL = '*',
  TK_DIV = '/',
  TK_LB = '(',
  TK_RB = ')',
  TK_NUM = '0',
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
  {"[0-9]+|0[xX][0-9a-fA-F]+", TK_NUM},
  {"\\$[a-z0-9]+", '$'},
  {"\\-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"\\[", '['},
  {"\\]", ']'},
  {"==", '='},         // equal
  {"\\!=", '!'},
  {"\\&\\&", '&'}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

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

static Token tokens[1024] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

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

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            goto a;
          case '0':
          case '$':
            sprintf(tokens[nr_token].str, "%.*s", substr_len, substr_start);
        }
        tokens[nr_token++].type = rules[i].token_type;
a:      break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int pos0;
uint32_t eval(int pos);
struct node0 {
  char op;
  int val;
  struct node0 *next;
};


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  *success = true;
  return eval(pos0 = 0);
}

uint32_t eval(int pos) {
  struct node0 head;
  head.val = 0;
  struct node0 *node = &head;
  bool suc;
  while (pos0 < nr_token) {
    struct node0 next;
    int val;
    char op;
    if (pos0 == pos) op = '+';
    else op = tokens[pos0 - 1].type;
    switch (tokens[pos0].type) {
    case '0':
      val = strtoul(tokens[pos0].str, NULL, 0);
      break;
    case '$':
      val = isa_reg_str2val(tokens[pos0].str + 1, &suc);
      break;
    case '(':
      val = eval(++pos0);
      break;
    case '[':
      val = pmem[eval(++pos0)];
      break;
    case ']':
    case ')':
      pos0++;
      goto b;
    default:
      goto a;
    }
    node = node->next = (struct node0 *) malloc(sizeof(struct node0));
    node->val = val;
    node->op = op;
a:  pos0++;
  }
b:node->next = NULL;
  node = &head;
  struct node0 *next;
  while (next = node->next) {
    if (next->op == '*') {
      node->val *= next->val;
      node->next = next->next;
    }
    else if (next->op == '/') {
      node->val /= next->val;
      node->next = next->next;
    }
    else node = next;
  }
  node = &head;
  while (next = node->next) {
    if (next->op == '+') {
      node->val += next->val;
      node->next = next->next;
    }
    else if (next->op == '-') {
      node->val -= next->val;
      node->next = next->next;
    }
    else node = next;
  }
  node = &head;
  while (next = node->next) {
    if (next->op == '=') {
      node->val = node->val == next->val;
      node->next = next->next;
    }
    else if (next->op == '!') {
      node->val = node->val != next->val;
      node->next = next->next;
    }
    else node = next;
  }
  node = &head;
  while (next = node->next) {
    if (next->op == '&') {
      node->val = node->val && next->val;
      node->next = next->next;
    }
    else node = next;
  }
  return head.val;
}

