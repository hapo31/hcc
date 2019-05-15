#pragma once

#include "vector.h"
#include "map.h"
#include "tokenizer.h"
#include "utils.h"

typedef enum
{
    ND_NUM = 256,
    ND_EMPTY,          // 空
    ND_IDENT,          // 識別子
    ND_CALL_FUCTION,   // 関数呼び出し
    ND_DEF_FUNCTION,   // 関数定義
    ND_PARAMETERS,     // 仮引数リスト
    ND_SEMI_EXPR_LIST, // ,区切りの式リスト
    ND_IF,             // if
    ND_ELSE,           // else
    ND_FOR,            // for
    ND_WHILE,          // while
    ND_RETURN,         // return
    ND_BLOCK,          // {}
    ND_EQ,             // ==
    ND_NE,             // !=
    ND_GE,             // <=
    ND_LE,             // >=
    ND_LT,             // >
    ND_GT,             // <
} NODE_TYPE;

typedef struct tagNode
{
    NODE_TYPE type;
    struct tagNode *lhs;
    struct tagNode *rhs;
    int value;  // type == NUM のとき、その数値
    char *name; // type == TK_IDENT のとき、その識別子の名前
    Vector *block_items;
    Vector *parameters;

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
    char *name;
    Map *variable_list;
    Node *top_level_code;
} Function;

Map *parse(Vector *tokenize_result);
