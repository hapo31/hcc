#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
    TK_NUM = 256,
    TK_EOF,
};

typedef struct
{
    int type;
    int value;
    char *input;
} Token;

Token tokens[100];

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void tokenize(char *p)
{
    int i = 0;
    while (*p)
    {
        // 空白なら読み飛ばす
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-')
        {
            tokens[i].type = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p))
        {
            tokens[i].type = TK_NUM;
            tokens[i].value = strtol(p, &p, 10);
            tokens[i].input = p;
            i++;
            continue;
        }

        error("トークナイズ失敗。: %s", p);
        exit(1);
    }

    // トークン列の最後に EOF を付与
    tokens[i].type = TK_EOF;
    tokens[i].input = p;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return 1;
    }

    tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main: \n");

    if (tokens[0].type != TK_NUM)
    {
        error("式の最初が数ではありません。");
    }
    printf("    mov rax, %d\n", tokens[0].value);

    int i = 1;

    while (tokens[i].type != TK_EOF)
    {
        // 足し算
        if (tokens[i].type == '+')
        {
            ++i;
            if (tokens[i].type != TK_NUM)
            {
                error("予期しないトークンです。:%s", tokens[i].input);
            }
            printf("    add rax, %d\n", tokens[i].value);
            ++i;
            continue;
        }

        // 引き算
        if (tokens[i].type == '-')
        {
            ++i;
            if (tokens[i].type != TK_NUM)
            {
                error("予期しないトークンです。:%s", tokens[i].input);
            }
            printf("    sub rax, %d\n", tokens[i].value);
            ++i;
            continue;
        }

        fprintf(stderr, "予期しない文字です。");
        return 1;
    }
    printf("    ret\n");

    return 0;
}