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
    /**
     * add: mul
     * add: add "+" mul
     * add: add "-" mul
     */
    Node *node = mul();
    do
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
    } while (1);
}

Node *mul()
{
    /**
     * mul: term
     * mul: mul "*" term
     * mul: mul "/" term
     */

    Node *node = term();

    do
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
    } while (1);
}

Node *term()
{
    /**
     * term: "(" term ")"
     * term: num
     * term: ident
     */
    if (consume('('))
    {
        Node *node = assign();
        if (!consume(')'))
        {
            error("開きカッコに対応する閉じカッコがありません: %s", ((Token *)tokens->data[pos])->input);
        }
        return node;
    }

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

Node *statement()
{
    /**
     * statement: "return" assign ";"
     * statement: asign ";"
     */

    Node *node = NULL;
    if (consume(TK_RETURN))
    {
        node = ret();
        node->lhs = assign();
    }
    else
    {
        node = assign();
    }
    if (!consume(';'))
    {
        error("式が ; で閉じられていません: %s\n", ((Token *)tokens->data[pos])->input);
    }

    return node;
}

Node *assign()
{
    /**
     * assign: add
     * assign: add "=" assign
     */
    Node *node = add();
    while (consume('='))
    {
        node = new_node('=', node, assign());
    }

    return node;
}

Node *ret()
{
    /**
     * return: "return"
     */
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_RETURN;
    return node;
}

void program()
{
    /**
     * program: statement program
     * program: ε
     */
    int i = 0;
    while (((Token *)tokens->data[pos])->type != TK_EOF)
    {
        Node *node = statement();
        push_vector(code, node);
        ++i;
    }
}

void gen_lvalue(Node *node)
{
    if (node->type != ND_IDENT)
    {
        error("代入の左辺値が変数ではありません。");
    }
    int offset = ('z' - node->name + 1) * 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n");
}

void gen(Node *node)
{

    if (node->type == ND_RETURN)
    {
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    }

    if (node->type == ND_NUM)
    {
        printf("    push %d\n", node->value);
        return;
    }

    if (node->type == ND_IDENT)
    {
        gen_lvalue(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    }

    if (node->type == '=')
    {
        gen_lvalue(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
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

int is_alpha_or_num(char c)
{
    if (c >= 'A' && c <= 'Z' ||
        c >= 'a' && c <= 'z' ||
        c >= '0' && c <= '9' ||
        c == '_')
    {
        return 1;
    }

    return 0;
}

void tokenize(char *p)
{
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
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';' || *p == '=')
        {
            token->type = *p;
            token->input = p;
            ++i;
            ++p;
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

        if (*p >= 'a' && *p <= 'z')
        {
            token->type = TK_IDENT;
            token->identifier = *p;
            token->input = p;
            ++i;
            ++p;
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

    // プロローグ
    // リターンアドレスをスタックに push し、ベースポインタの指すアドレスをスタックの先頭が指すアドレスとする
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n"); // 最初からa-zまでの変数の領域を確保しておくので 8 * 26 = 208

    for (int i = 0; i < code->len; ++i)
    {
        // 抽象構文木からアセンブラを生成
        gen(code->data[i]);

        // スタックに式の評価結果が乗っているので pop しておく
        printf("    pop rax\n");
    }

    // エピローグ

    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    // rax に演算結果が残っているので、それがこのプログラムの出力になる
    printf("    ret\n");

    return 0;
}