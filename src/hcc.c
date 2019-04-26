#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hcc.h"

Token tokens[100];

Node *new_node(int type, Node *lhs, Node *rhs)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = type;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int value)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_NUM;
    node->value = value;
    return node;
}

int pos = 0;

int consume(int type)
{
    if (tokens[pos].type != type)
    {
        return 0;
    }
    ++pos;
    return 1;
}

Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume('+'))
        {
            node = new_node('+', node, mul());
        }
        else if (consume('-'))
        {
            node = new_node('-', node, mul());
        }
        else
        {
            return node;
        }
    }
}

Node *mul()
{
    Node *node = term();

    for (;;)
    {
        if (consume('*'))
        {
            node = new_node('*', node, term());
        }
        else if (consume('/'))
        {
            node = new_node('/', node, term());
        }
        else
        {
            return node;
        }
    }
}

Node *term()
{
    if (consume('('))
    {
        Node *node = add();
        if (!consume(')'))
        {
            error("開きカッコに対応する閉じカッコがありません。: %s", tokens[pos].input);
        }
        return node;
    }

    if (tokens[pos].type == TK_NUM)
    {
        return new_node_num(tokens[pos++].value);
    }

    error("数値でも開きカッコでもないトークンです: %s", tokens[pos].input);
}

void gen(Node *node)
{
    if (node->type == ND_NUM)
    {
        printf("    push %d\n", node->value);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->type)
    {
    case '+':
        printf("    add rax, rdi\n");
        break;
    case '-':
        printf("    sub rax, rdi\n");
        break;
    case '*':
        printf("    mul rdi\n");
        break;
    case '/':
        printf("    mov rdx, 0\n");
        printf("    div rdi\n");
        break;

    default:
        break;
    }

    printf("    push rax\n");
}

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

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
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

    // トークン列に分解
    tokenize(argv[1]);
    // 抽象構文木を作成
    Node *node = add();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main: \n");

    // 抽象構文木からアセンブラを生成
    gen(node);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("    pop rax\n");
    printf("    ret\n");

    return 0;
}