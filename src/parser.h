#pragma once

#include "vector.h"
#include "map.h"
#include "tokenizer.h"
#include "utils.h"

typedef enum
{
    ND_NUM = 256,
    ND_IDENT,  // 識別子
    ND_IF,     // if
    ND_ELSE,   // else
    ND_FOR,    // for
    ND_WHILE,  // while
    ND_RETURN, // return
    ND_BLOCK,  // {}
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_GE,     // <=
    ND_LE,     // >=
    ND_LT,     // >
    ND_GT,     // <
} NODE_TYPE;

typedef struct tagNode
{
    NODE_TYPE type;
    struct tagNode *lhs;
    struct tagNode *rhs;
    int value;
    char *name;
    Vector *block_items;

    struct tagNode *then;

    // for "if" statement
    struct tagNode *condition;
    struct tagNode *else_;
    // for "for" statement
    struct tagNode *init_expression;
    struct tagNode *loop_expression;

} Node;

Node *new_node(NODE_TYPE type, Node *lhs, Node *rhs);
Node *new_node_num(int value);
Node *new_node_identifier(char *name);

int consume(int type);
Node *add();
Node *mul();
Node *unary();
Node *term();
Node *assign();
Node *equality();
Node *relational();
Node *statement();
Node *ret();

void program();

typedef struct
{
    Vector *code;
    Map *identifiers;
} ParseResult;

ParseResult parse(TokenizeResult *tokenize_result);
