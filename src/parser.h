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
    ND_DEF_VAR,        // 変数定義
    ND_PARAMETERS,     // 仮引数リスト
    ND_SEMI_EXPR_LIST, // ,区切りの式リスト
    ND_ARGS,           // 実引数
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
} NODE;

typedef enum
{
    NT_VOID,
    NT_INT
} NODE_TYPE;

typedef struct tagNode
{
    NODE type;
    NODE_TYPE node_type;
    struct tagNode *lhs;
    struct tagNode *rhs;
    int value;  // type == NUM のとき、その数値
    char *name; // type == TK_IDENT のとき、その識別子の名前
    Vector *block_items;
    Vector *parameters;

    Vector *args;

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
    NODE_TYPE type;
    char *name;
    size_t index;
} Variable;

typedef struct
{
    char *name;
    NODE_TYPE return_type;
    int parameter_count;
    Map *variable_list;
    Node *top_level_code;
} Function;

Map *parse(Vector *tokenize_result);
