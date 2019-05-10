#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "codegen.h"

void emit(const char *fmt, ...);
void initial(FILE *fp);
void prologue(FILE *fp);
void gen(Node *node);
void gen_lvalue(Node *node);
void epilogue(FILE *fp);

FILE *output_fp;
Map *variables;
Map *functions;
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
    emit(".intel_syntax noprefix");
    emit(".global main");
    emit("main: ");
}

void prologue(FILE *fp)
{
    // プロローグ
    // リターンアドレスをスタックに push し、ベースポインタの指すアドレスをスタックの先頭が指すアドレスとする
    emit("    push rbp");
    emit("    mov rbp, rsp");
    emit("    sub rsp, %ld", variables->len * VAR_SIZE); // 変数はすべてVAR_SIZEとしておく
}

void gen_lvalue(Node *node)
{
    if (node->type != ND_IDENT)
    {
        error("代入の左辺値が変数ではありません。");
    }
    int ident_index = (intptr_t)read_map(variables, node->name);
    int offset = ident_index * VAR_SIZE;
    emit("    mov rax, rbp");
    emit("    sub rax, %d", offset);
    emit("    push rax");
}

void gen(Node *node)
{
    if (node == NULL)
    {
        error("internal error: node is null.\n");
    }

    if (node->type == ND_CALL_FUCTION)
    {
        // TODO: rsp を 16byte のアライメントに調整する必要があるらしい
        // ↓このコードはちゃんと動いてない
        // emit("    mov rax, rsp");
        // emit("    mov rdx, 16");
        // emit("    div rdx");
        // emit("    add rsp, rdx");
        emit("    call %s", node->name);
        return;
    }

    if (node->type == ND_BLOCK)
    {
        for (int i = 0; i < node->block_items->len; ++i)
        {
            Node *block_node = (Node *)node->block_items->data[i];
            gen(block_node);
            emit("    pop rax");
        }

        return;
    }

    if (node->type == ND_IF)
    {
        int local_if_count = if_count;
        ++if_count;
        gen(node->condition);
        emit("    pop rax");
        emit("    cmp rax, 0");
        // else 節があるかどうか
        if (node->else_ != NULL)
        {
            int else_count_local = else_count;
            ++else_count;
            emit("    je .Lelse%d", else_count_local);
            gen(node->then);
            emit("    jmp .Lendif%d", local_if_count);
            emit(".Lelse%d:", else_count_local);
            gen(node->else_);
            emit(".Lendif%d:", local_if_count);
        }
        else
        {
            emit("    je .Lendif%d", local_if_count);
            gen(node->then);
            emit(".Lendif%d:", local_if_count);
        }
        return;
    }

    if (node->type == ND_WHILE)
    {
        int while_count_local = while_count;
        ++while_count;
        emit(".Lwhile%d:", while_count_local);
        gen(node->condition);
        emit("    pop rax");
        emit("    cmp rax, 0");
        emit("    je .Lendwhile%d", while_count_local);
        gen(node->then);
        emit("    jmp .Lwhile%d", while_count_local);
        emit(".Lendwhile%d:", while_count_local);
        return;
    }

    if (node->type == ND_FOR)
    {
        int for_count_local = for_count;
        ++for_count;
        gen(node->init_expression);
        emit(".Lfor%d:", for_count_local);
        gen(node->condition);
        emit("    pop rax");
        emit("    cmp rax, 0");
        emit("    je .Lforend%d", for_count_local);
        gen(node->then);
        gen(node->loop_expression);
        emit("    jmp .Lfor%d", for_count_local);
        emit(".Lforend%d:", for_count_local);
        return;
    }

    if (node->type == ND_RETURN)
    {
        gen(node->lhs);
        emit("    pop rax");
        emit("    mov rsp, rbp");
        emit("    pop rbp");
        emit("    ret");
        return;
    }

    if (node->type == ND_NUM)
    {
        emit("    push %d", node->value);
        return;
    }

    if (node->type == ND_IDENT)
    {
        gen_lvalue(node);
        emit("    pop rax");
        emit("    mov rax, [rax]");
        emit("    push rax");
        return;
    }

    if (node->type == '=')
    {
        gen_lvalue(node->lhs);
        gen(node->rhs);

        emit("    pop rdi");
        emit("    pop rax");
        emit("    mov [rax], rdi");
        emit("    push rdi");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    emit("    pop rdi");
    emit("    pop rax");

    switch (node->type)
    {
    case '+':
        emit("    add rax, rdi");
        break;
    case '-':
        emit("    sub rax, rdi");
        break;
    case '*':
        emit("    mul rdi");
        break;
    case '/':
        emit("    mov rdx, 0");
        emit("    div rdi");
        break;
    case '%':
        emit("    mov rdx, 0");
        emit("    div rdi");
        emit("    mov rax, rdx");
        break;
    case ND_EQ:
        emit("    cmp rax, rdi");
        emit("    sete al");
        emit("    movzb rax, al");
        break;
    case ND_NE:
        emit("    cmp rax, rdi");
        emit("    setne al");
        emit("    movzb rax, al");
        break;
    case ND_LE:
        emit("    cmp rax, rdi");
        emit("    setle al");
        emit("    movzb rax, al");
        break;
    case ND_LT:
        emit("    cmp rax, rdi");
        emit("    setl al");
        emit("    movzb rax, al");
        break;
    default:
        break;
    }

    emit("    push rax");
}

void epilogue(FILE *fp)
{
    // エピローグ

    emit("    mov rsp, rbp");
    emit("    pop rbp");
    // rax に演算結果が残っているので、それがこのプログラムの出力になる
    emit("    ret");
}

void codegen(FILE *fp, ParseResult *parse_result)
{
    output_fp = fp;
    code = parse_result->code;
    variables = parse_result->variables;

    initial(fp);

    prologue(fp);

    for (int i = 0; i < code->len; ++i)
    {
        // 抽象構文木からアセンブラを生成
        gen(code->data[i]);

        // スタックに式の評価結果が乗っているので pop しておく
        emit("    pop rax");
    }

    epilogue(fp);
}
