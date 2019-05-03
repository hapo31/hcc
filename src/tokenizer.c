#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "tokenizer.h"
#include "utils.h"

TokenizeResult tokenize(char *p)
{
    Vector *tokens = new_vector(100);
    Map *identifiers = new_map();
    int i = 0;

    while (*p)
    {
        // 空白なら読み飛ばす
        if (isspace(*p))
        {
            ++p;
            continue;
        }

        Token *token = malloc(sizeof(Token));
        push_vector(tokens, token);

        if (strncmp(p, "if", 2) == 0)
        {
            token->type = TK_IF;
            token->input = p;
            ++i;
            p += 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0)
        {
            token->type = TK_ELSE;
            token->input = p;
            ++i;
            p += 4;
            continue;
        }

        if (strncmp(p, "goto", 4) == 0)
        {
            token->type = TK_GOTO;
            token->input = p;
            ++i;
            p += 4;
            continue;
        }

        // returnキーワードのあとにアルファベット、数字、アンダースコアが来ていないかを調べる
        if (strncmp(p, "return", 6) == 0 && !is_alpha_or_num(*(p + 6)))
        {
            token->type = TK_RETURN;
            token->input = p;
            ++i;
            p += 6;
            continue;
        }

        if (strncmp(p, "==", 2) == 0)
        {
            token->type = TK_EQ;
            token->input = p;
            ++i;
            p += 2;
            continue;
        }

        if (strncmp(p, "!=", 2) == 0)
        {
            token->type = TK_NE;
            token->input = p;
            ++i;
            p += 2;
            continue;
        }

        if (strncmp(p, ">=", 2) == 0)
        {
            token->type = TK_GE;
            token->input = p;
            ++i;
            p += 2;
            continue;
        }

        if (strncmp(p, "<=", 2) == 0)
        {
            token->type = TK_LE;
            token->input = p;
            ++i;
            p += 2;
            continue;
        }

        if (*p == '>')
        {
            token->type = TK_GT;
            token->input = p;
            ++i;
            ++p;
            continue;
        }

        if (*p == '<')
        {
            token->type = TK_LT;
            token->input = p;
            ++i;
            ++p;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
            *p == '(' || *p == ')' || *p == ';' || *p == '=' ||
            *p == ':')
        {
            token->type = *p;
            token->input = p;
            ++i;
            ++p;
            continue;
        }

        // アルファベットかアンダースコアだったら識別子の開始とする
        if (*p >= 'a' && *p <= 'z' || *p >= 'A' && *p <= 'Z' || *p == '_')
        {
            token->type = TK_IDENT;
            char *head = p;
            int len = 0;
            while (is_alpha_or_num(*(p + len)))
            {
                ++len;
                if (*(p + len) == EOF)
                {
                    error("識別子が正しく終了していません: %s", head);
                }
            }
            token->identifier = malloc(sizeof(char) * (len + 1));
            strncpy(token->identifier, p, len);
            if (!contains_map(identifiers, token->identifier))
            {
                put_map(identifiers, token->identifier, (void *)identifiers->len);
            }
            token->input = p;
            ++i;
            p += len;
            continue;
        }

        if (isdigit(*p))
        {
            token->type = TK_NUM;
            token->value = strtol(p, &p, 10);
            token->input = p;
            ++i;
            continue;
        }

        error("トークナイズ失敗。: %s", p);
        exit(1);
    }

    Token *token = malloc(sizeof(Token));
    // トークン列の最後に EOF を付与
    token->type = TK_EOF;
    token->input = p;
    push_vector(tokens, token);

    TokenizeResult result = {tokens, identifiers};

    return result;
}