#pragma once
#include "vector.h"
#include "map.h"

typedef enum
{
    TK_NUM = 256,
    TK_IDENT,  // 識別子
    TK_LABEL,  // ラベル
    TK_IF,     // if
    TK_ELSE,   // else
    TK_FOR,    // for
    TK_WHILE,  // while
    TK_RETURN, // return
    TK_EQ,     // ==
    TK_NE,     // !=
    TK_GE,     // <=
    TK_LE,     // >=
    TK_LT,     // >
    TK_GT,     // <
    TK_GOTO,   // goto
    TK_INT,    // int
    TK_EOF,
} TOKEN_TYPE;

typedef struct
{
    TOKEN_TYPE type;
    // type が TK_NUM のとき、数値
    int value;
    // type が TK_IDENT のとき、識別子の名前
    char *identifier;
    // デバッグ用
    char *input;
} Token;

Vector *tokenize(char *p);
