#include <stdio.h>

#include "hcc.h"

void print_token(Token *token)
{
  switch (token->type)
  {
  case TK_NUM:
    fprintf(stderr, "type: TK_NUM, value: %d\n", token->value);
    break;
  case TK_IDENT:
    fprintf(stderr, "type: TK_IDENT, name: %c\n", token->identifier);
    break;
  case TK_EOF:
    fprintf(stderr, "type: TK_EOF\n");
    break;
  default:
    fprintf(stderr, "type: %c\n", token->type);
    break;
  }
}
