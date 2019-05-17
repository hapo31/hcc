#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "codegen.h"

void emit(const char *fmt, ...);
void initial();
void emit_global_functions();
void prologue();
void gen_parameter();
void gen_function(Function *function);
void gen(Node *node);
void gen_lvalue(Node *node);
void epilogue();

static FILE *output_fp;
static Map *context_function_list;
static Function *function;

const char *x86_64_args_registers[] = {
    "rdi",
    "rsi",
    "rdx",
    "rcx",
    "r8",
    "r9",
};

#define ARGS_REGISTER_SIZE (sizeof(x86_64_args_registers) / sizeof(char *))

int if_count = 0;
int else_count = 0;
int while_count = 0;
int for_count = 0;

void label(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(output_fp, fmt, ap);
    fprintf(output_fp, "\n");
}

void emit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(output_fp, "\t");
    vfprintf(output_fp, fmt, ap);
    fprintf(output_fp, "\n");
}

void initial()
{
    label(".intel_syntax noprefix");
    label(".global main");
}

void gen_function(Function *function)
{
    label("%s:", function->name);

    prologue();
    gen_parameter();
    gen(function->top_level_code);
    epilogue();
}

void prologue()
{
    // プロローグ
    // リターンアドレスをスタックに push し、ベースポインタの指すアドレスをスタックの先頭が指すアドレスとする
    emit("push rbp");
    emit("mov rbp, rsp");
    emit("sub rsp, %ld", (function->variable_list->len) * VAR_SIZE); // 変数はすべてVAR_SIZEとしておく
}

void gen_lvalue(Node *node)
{
    if (node->type != ND_IDENT)
    {
        error("代入の左辺値が変数ではありません。");
    }
    if (!contains_map(function->variable_list, node->name))
    {
        error("変数が定義されていません: %s", node->name);
    }
    int ident_index = (intptr_t)read_map(function->variable_list, node->name);
    int offset = (ident_index + 1) * VAR_SIZE;
    emit("mov rax, rbp");
    emit("sub rax, %d", offset);
    emit("push rax");
}

void gen_parameter()
{
    for (int i = 0; i < function->parameter_count; ++i)
    {
        int offset = ((intptr_t)read_map(function->variable_list, (char *)function->variable_list->keys->data[i]) + 1) * VAR_SIZE;
        if (i < ARGS_REGISTER_SIZE)
        {
            emit("mov [rbp-%ld], %s", offset, x86_64_args_registers[i]);
        }
        else
        {
            emit("mov r10, [rbp+%ld]", (i - ARGS_REGISTER_SIZE + 2) * VAR_SIZE);
            emit("mov [rbp-%ld], r10", offset);
        }
    }
}

void gen(Node *node)
{
    if (node == NULL || node->type == ND_EMPTY)
    {
        return;
    }

    if (node->type == ND_CALL_FUCTION)
    {
        int args_len = node->args->len;
        if (args_len > 0)
        {
            Vector *args = node->args;

            // スタックポインタを 16byte 境界にアライメント
            // emit("mov rax, rsp");
            // emit("mov rdx, 0");
            // emit("mov rdi, 16");
            // emit("div rdi");
            // emit("sub rsp, rdx");

            // 引数が7個以上かつ奇数のとき、 rsp が 16byte アライメントにならないので
            // 追加でスタックポインタをさらに押し下げる
            // if (args_len >= 7 && args_len % 2 == 1)
            // {
            //     emit("sub rsp, 8");
            // }
            for (int i = args_len - 1; i >= 0; --i)
            {
                gen((Node *)args->data[i]);
                if (i < ARGS_REGISTER_SIZE)
                {
                    emit("pop %s", x86_64_args_registers[i]);
                }
                // 引数をスタックに乗せる操作をするはずだけど、
                // 今のコードだともうすでにスタックに計算結果が乗っている
            }
        }

        emit("call %s", node->name);
        // 引数のために積んだスタックの後始末
        if (args_len >= 7)
        {
            emit("add rsp, %d", (args_len - 6) * VAR_SIZE);
        }
        emit("push rax");

        return;
    }

    if (node->type == ND_BLOCK)
    {
        for (int i = 0; i < node->block_items->len; ++i)
        {
            Node *block_node = (Node *)node->block_items->data[i];
            gen(block_node);
            emit("pop rax");
        }
        emit("push rax");
        return;
    }

    if (node->type == ND_IF)
    {
        int local_if_count = if_count;
        ++if_count;
        gen(node->condition);
        emit("pop rax");
        emit("cmp rax, 0");
        // else 節があるかどうか
        if (node->else_ != NULL)
        {
            int else_count_local = else_count;
            ++else_count;
            emit("je .Lelse%d", else_count_local);
            gen(node->then);
            emit("jmp .Lendif%d", local_if_count);
            label(".Lelse%d:", else_count_local);
            gen(node->else_);
            label(".Lendif%d:", local_if_count);
        }
        else
        {
            emit("je .Lendif%d", local_if_count);
            gen(node->then);
            label(".Lendif%d:", local_if_count);
        }
        emit("push rax");
        return;
    }

    if (node->type == ND_WHILE)
    {
        int while_count_local = while_count;
        ++while_count;
        label(".Lwhile%d:", while_count_local);
        gen(node->condition);
        emit("pop rax");
        emit("cmp rax, 0");
        emit("je .Lendwhile%d", while_count_local);
        gen(node->then);
        emit("jmp .Lwhile%d", while_count_local);
        label(".Lendwhile%d:", while_count_local);
        return;
    }

    if (node->type == ND_FOR)
    {
        int for_count_local = for_count;
        ++for_count;
        gen(node->init_expression);
        label(".Lfor%d:", for_count_local);
        gen(node->condition);
        emit("pop rax");
        emit("cmp rax, 0");
        emit("je .Lforend%d", for_count_local);
        gen(node->then);
        gen(node->loop_expression);
        emit("jmp .Lfor%d", for_count_local);
        label(".Lforend%d:", for_count_local);
        return;
    }

    if (node->type == ND_RETURN)
    {
        gen(node->lhs);
        emit("pop rax");
        emit("mov rsp, rbp");
        emit("pop rbp");
        emit("ret");
        return;
    }

    if (node->type == ND_NUM)
    {
        emit("push %d", node->value);
        return;
    }

    if (node->type == ND_IDENT)
    {
        gen_lvalue(node);
        emit("pop rax");
        emit("mov rax, [rax]");
        emit("push rax");
        return;
    }

    if (node->type == '=')
    {
        gen_lvalue(node->lhs);
        gen(node->rhs);

        emit("pop rdi");
        emit("pop rax");
        emit("mov [rax], rdi");
        emit("push rdi");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    emit("pop rdi");
    emit("pop rax");

    switch (node->type)
    {
    case '+':
        emit("add rax, rdi");
        break;
    case '-':
        emit("sub rax, rdi");
        break;
    case '*':
        emit("mul rdi");
        break;
    case '/':
        emit("mov rdx, 0");
        emit("div rdi");
        break;
    case '%':
        emit("mov rdx, 0");
        emit("div rdi");
        emit("mov rax, rdx");
        break;
    case ND_EQ:
        emit("cmp rax, rdi");
        emit("sete al");
        emit("movzb rax, al");
        break;
    case ND_NE:
        emit("cmp rax, rdi");
        emit("setne al");
        emit("movzb rax, al");
        break;
    case ND_LE:
        emit("cmp rax, rdi");
        emit("setle al");
        emit("movzb rax, al");
        break;
    case ND_LT:
        emit("cmp rax, rdi");
        emit("setl al");
        emit("movzb rax, al");
        break;
    default:
        break;
    }

    emit("push rax");
}

void epilogue()
{
    // エピローグ

    emit("mov rsp, rbp");
    emit("pop rbp");
    // rax に演算結果が残っているので、それがこのプログラムの出力になる
    emit("ret");
}

void codegen(FILE *fp, Map *parse_result)
{
    output_fp = fp;
    context_function_list = parse_result;

    initial();

    for (int i = 0; i < context_function_list->len; ++i)
    {
        function = (Function *)context_function_list->data->data[i];
        // 関数を出力
        gen_function(function);
    }
}
