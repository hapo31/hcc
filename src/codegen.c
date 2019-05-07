#include "codegen.h"

#include <stdio.h>
#include <stdarg.h>

void emit(const char *fmt, ...);
void initial(FILE *fp);
void prologue(FILE *fp);
void gen(Node *node);
void gen_lvalue(Node *node);
void epilogue(FILE *fp);

FILE *output_fp;
Map *identifiers;
Vector *code;
int if_count = 0;
int else_count = 0;
int while_count = 0;
int for_count = 0;

void emit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(output_fp, fmt, ap);
    fprintf(output_fp, "\n");
}

void initial(FILE *fp)
{
    emit(".intel_syntax noprefix\n");
    emit(".global main\n");
    emit("main: \n");
}

void prologue(FILE *fp)
{
    // プロローグ
    // リターンアドレスをスタックに push し、ベースポインタの指すアドレスをスタックの先頭が指すアドレスとする
    emit("    push rbp\n");
    emit("    mov rbp, rsp\n");
    emit("    sub rsp, %ld\n", identifiers->len * VAR_SIZE); // 変数はすべてVAR_SIZEとしておく
}

void gen_lvalue(Node *node)
{
    if (node->type != ND_IDENT)
    {
        error("代入の左辺値が変数ではありません。");
    }
    int ident_index = (int)read_map(identifiers, node->name);
    int offset = ident_index * VAR_SIZE;
    emit("    mov rax, rbp\n");
    emit("    sub rax, %d\n", offset);
    emit("    push rax\n");
}

void gen(Node *node)
{
    if (node->type == ND_IF)
    {
        gen(node->condition);
        emit("    pop rax\n");
        emit("    cmp rax, 0\n");
        // else 節があるかどうか
        if (node->else_ != NULL)
        {
            emit("    je .Lelse%d\n", else_count);
            gen(node->then);
            emit("    jmp .Lendif%d\n", if_count);
            emit(".Lelse%d:\n", else_count);
            gen(node->else_);
            emit(".Lendif%d:\n", if_count);
            ++else_count;
        }
        else
        {
            emit("    je .Lendif%d\n", if_count);
            gen(node->then);
            emit(".Lendif%d:\n", if_count);
        }

        ++if_count;
        return;
    }

    if (node->type == ND_WHILE)
    {
        emit(".Lwhile%d:\n", while_count);
        gen(node->condition);
        emit("    pop rax\n");
        emit("    cmp rax, 0\n");
        emit("    je .Lendwhile%d\n", while_count);
        gen(node->then);
        emit("    jmp .Lwhile%d\n", while_count);
        emit(".Lendwhile%d:\n", while_count);
        ++while_count;
        return;
    }

    if (node->type == ND_FOR)
    {
        gen(node->init_expression);
        emit(".Lfor%d:\n", for_count);
        gen(node->condition);
        emit("    pop rax\n");
        emit("    cmp rax, 0\n");
        emit("    je .Lforend%d\n", for_count);
        gen(node->then);
        gen(node->loop_expression);
        emit("    jmp .Lfor%d\n", for_count);
        emit(".Lforend%d:\n", for_count);

        ++for_count;
        return;
    }

    if (node->type == ND_RETURN)
    {
        gen(node->lhs);
        emit("    pop rax\n");
        emit("    mov rsp, rbp\n");
        emit("    pop rbp\n");
        emit("    ret\n");
        return;
    }

    if (node->type == ND_NUM)
    {
        emit("    push %d\n", node->value);
        return;
    }

    if (node->type == ND_IDENT)
    {
        gen_lvalue(node);
        emit("    pop rax\n");
        emit("    mov rax, [rax]\n");
        emit("    push rax\n");
        return;
    }

    if (node->type == '=')
    {
        gen_lvalue(node->lhs);
        gen(node->rhs);

        emit("    pop rdi\n");
        emit("    pop rax\n");
        emit("    mov [rax], rdi\n");
        emit("    push rdi\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    emit("    pop rdi\n");
    emit("    pop rax\n");

    switch (node->type)
    {
    case '+':
        emit("    add rax, rdi\n");
        break;
    case '-':
        emit("    sub rax, rdi\n");
        break;
    case '*':
        emit("    mul rdi\n");
        break;
    case '/':
        emit("    mov rdx, 0\n");
        emit("    div rdi\n");
        break;
    case '%':
        emit("    mov rdx, 0\n");
        emit("    div rdi\n");
        emit("    mov rax, rdx\n");
        break;
    case ND_EQ:
        emit("    cmp rax, rdi\n");
        emit("    sete al\n");
        emit("    movzb rax, al\n");
        break;
    case ND_NE:
        emit("    cmp rax, rdi\n");
        emit("    setne al\n");
        emit("    movzb rax, al\n");
        break;
    case ND_LE:
        emit("    cmp rax, rdi\n");
        emit("    setle al\n");
        emit("    movzb rax, al\n");
        break;
    case ND_LT:
        emit("    cmp rax, rdi\n");
        emit("    setl al\n");
        emit("    movzb rax, al\n");
        break;
    default:
        break;
    }

    emit("    push rax\n");
}

void epilogue(FILE *fp)
{
    // エピローグ

    emit("    mov rsp, rbp\n");
    emit("    pop rbp\n");
    // rax に演算結果が残っているので、それがこのプログラムの出力になる
    emit("    ret\n");
}

void codegen(FILE *fp, ParseResult *parse_result)
{
    output_fp = fp;
    code = parse_result->code;
    identifiers = parse_result->identifiers;
    initial(fp);

    prologue(fp);

    for (int i = 0; i < code->len; ++i)
    {
        // 抽象構文木からアセンブラを生成
        gen(code->data[i]);

        // スタックに式の評価結果が乗っているので pop しておく
        emit("    pop rax\n");
    }

    epilogue(fp);
}
