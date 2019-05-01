#pragma once
#include "vector.h"
#include "map.h"

typedef enum
{
    TK_NUM = 256,
    TK_IDENT,
    TK_RETURN,
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

typedef struct
{
    Vector *tokens;
    Map *identifiers;
} TokenizeResult;

TokenizeResult tokenize(char *p);
