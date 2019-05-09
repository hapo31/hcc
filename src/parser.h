#pragma once

#include "vector.h"
#include "map.h"
#include "tokenizer.h"
#include "utils.h"

typedef enum
{
    ND_NUM = 256,
    ND_IDENT,        // 識別子
    ND_CALL_FUCTION, // 関数呼び出し
    ND_IF,           // if
    ND_ELSE,         // else
    ND_FOR,          // for
    ND_WHILE,        // while
    ND_RETURN,       // return
    ND_BLOCK,        // {}
    ND_EQ,           // ==
    ND_NE,           // !=
    ND_GE,           // <=
    ND_LE,           // >=
    ND_LT,           // >
    ND_GT,           // <
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

typedef struct
{
    Vector *code;
    Map *variables;
    Map *functions;
} ParseResult;

ParseResult parse(TokenizeResult *tokenize_result);
