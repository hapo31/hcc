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
    ND_DEREF,          // デリファレンス(*)演算子
    ND_ADDR,           // アドレス(&)演算子
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
    NT_PTR,
    NT_INT
} NODE_TYPE;

typedef struct TypeNode
{
    NODE_TYPE type;
    struct TypeNode *ptr_of;
} TypeNode;

typedef struct Node
{
    NODE type;
    TypeNode *node_type;
    struct Node *lhs;
    struct Node *rhs;
    int value;  // type == NUM のとき、その数値
    char *name; // type == TK_IDENT のとき、その識別子の名前
    Vector *block_items;
    Vector *parameters;

    Vector *args;

    struct Node *then;

    // for "if" statement
    struct Node *condition;
    struct Node *else_;
    // for "for" statement
    struct Node *init_expression;
    struct Node *loop_expression;

} Node;

typedef struct
{
    TypeNode *type_node;
    char *name;
    size_t index;
} Variable;

typedef struct
{
    char *name;
    TypeNode *return_type;
    size_t parameter_count;
    Map *variable_list;
    Node *top_level_code;
} Function;

Map *parse(Vector *tokenize_result);
