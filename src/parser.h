#pragma once

#include "vector.h"
#include "map.h"
#include "tokenizer.h"
#include "utils.h"

typedef enum
{
    ND_NUM = 256,
    ND_IDENT,
    ND_RETURN,
} NODE_TYPE;

typedef struct tagNode
{
    NODE_TYPE type;
    struct tagNode *lhs;
    struct tagNode *rhs;
    int value;
    char *name;
} Node;

Node *new_node(int type, Node *lhs, Node *rhs);
Node *new_node_num(int value);
Node *new_node_identifier(char *name);

int consume(int type);
Node *add();
Node *mul();
Node *term();
Node *assign();
Node *statement();
Node *ret();

void program();

typedef struct
{
    Vector *code;
    Map *identifiers;
} ParseResult;

ParseResult parse(TokenizeResult *tokenize_result);
