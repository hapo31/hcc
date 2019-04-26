#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"
#include "hcc.h"

Vector *tokens;
Vector *code;

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

Node *new_node_identifier(char name)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_IDENT;
    node->name = name;
    return node;
}

int pos = 0;

int consume(int type)
{
    if (((Token *)tokens->data[pos])->type != type)
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
        Node *node = assign();
        if (!consume(')'))
        {
            error("開きカッコに対応する閉じカッコがありません: %s", ((Token *)tokens->data[pos])->input);
        }
        return node;
    }

    // fprintf(stderr, "type: %d\n", ((Token *)tokens->data[pos])->type);
    if (((Token *)tokens->data[pos])->type == TK_NUM)
    {
        return new_node_num(((Token *)tokens->data[pos++])->value);
    }

    if (((Token *)tokens->data[pos])->type == TK_IDENT)
    {
        return new_node_identifier(((Token *)tokens->data[pos++])->identifier);
    }

    error("数値でも開きカッコでもないトークンです: %s", ((Token *)tokens->data[pos])->input);
}

void program()
{
    int i = 0;
    while (((Token *)tokens->data[pos])->type != TK_EOF)
    {
        Node *node = statement();
        push_vector(code, node);
        ++i;
    }
}

Node *statement()
{
    Node *node = assign();
    if (!consume(';'))
    {
        error("式が ; で閉じられていません: %s\n", ((Token *)tokens->data[pos])->input);
    }

    return node;
}

Node *assign()
{
    Node *node = add();
    while (consume('='))
    {
        node = new_node('=', node, assign());
    }

    return node;
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

        Token *token = malloc(sizeof(Token));
        push_vector(tokens, token);
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';' || *p == '=')
        {
            token->type = *p;
            token->input = p;
            i++;
            p++;
            continue;
        }

        if (*p >= 'a' && *p <= 'z')
        {
            token->type = TK_IDENT;
            token->identifier = *p;
            token->input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p))
        {
            token->type = TK_NUM;
            token->value = strtol(p, &p, 10);
            token->input = p;
            i++;
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
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません。\n");
        return 1;
    }

    tokens = new_vector(10);
    code = new_vector(5);

    // トークン列に分解
    tokenize(argv[1]);

    // 抽象構文木を作成
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main: \n");

    // 抽象構文木からアセンブラを生成
    gen(code->data[0]);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("    pop rax\n");
    printf("    ret\n");

    return 0;
}