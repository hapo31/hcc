#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "codegen.h"

size_t get_variable_offset(char *name);
void label(const char *fmt, ...);
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
static Function *context_function;

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

size_t get_variable_offset(char *name)
{
    Variable *var = (Variable *)read_map(context_function->variable_list, name);
    size_t ident_index = var->index;
    size_t offset = (ident_index + 1) * VAR_SIZE;

    return offset;
}

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
    if (context_function->top_level_code == NULL)
    {
        return;
    }
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
    emit("sub rsp, %ld", (context_function->variable_list->len) * VAR_SIZE); // 変数はすべてVAR_SIZEとしておく
}

void gen_lvalue(Node *node)
{
    if (node->type != ND_IDENT && node->type != ND_DEREF)
    {
        error("代入の左辺値が変数または間接演算子ではありません");
    }
    emit("mov rax, rbp");
    if (node->type == ND_DEREF)
    {
        emit("sub rax, %d", get_variable_offset(node->lhs->name));
    }
    else
    {
        emit("sub rax, %d", get_variable_offset(node->name));
    }
    emit("push rax");
}

void gen_parameter()
{
    for (int i = 0; i < context_function->parameter_count; ++i)
    {
        char *name = (char *)context_function->variable_list->keys->data[i];
        size_t offset = get_variable_offset(name);
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
    if (node == NULL || node->type == ND_DEF_VAR || node->type == ND_EMPTY)
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
            // 変数定義の行は無視する
            if (block_node->type != ND_DEF_VAR)
            {
                gen(block_node);
                emit("pop rax");
            }
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
        if (node->lhs->type == ND_DEREF)
        {
            emit("mov rax, [rax]");
        }
        emit("mov [rax], rdi");
        emit("push rdi");
        return;
    }

    if (node->type == ND_DEREF)
    {
        gen(node->lhs);
        emit("pop rax");
        emit("mov rax, [rax]");
        emit("push rax");
        return;
    }

    if (node->type == ND_ADDR)
    {
        if (node->rhs->type != ND_IDENT)
        {
            error("& 演算子は変数に対して使われる必要があります: %s");
        }
        char *name = node->rhs->name;
        size_t offset = get_variable_offset(name);
        emit("lea r10, [rbp - %ld]", offset);
        emit("push r10");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    // r10 は 右辺
    emit("pop r10");
    // r11 は 左辺
    emit("pop r11");

    // 左辺がポインタ型かつ右辺が整数値の場合
    if (node->lhs->node_type->type == NT_PTR &&
        node->rhs->node_type->type == NT_INT)
    {
        emit("mov rax, r10");
        emit("mov r8, %d", node->lhs->node_type->size);
        emit("mul r8");
        emit("mov r10, rax");
    }
    // 右辺がポインタ型かつ左辺が整数値の場合
    else if (node->lhs->node_type->type == NT_INT &&
             node->rhs->node_type->type == NT_PTR)
    {
        emit("mov rax, r11");
        emit("mov r8, %d", node->rhs->node_type->size);
        emit("mul r8");
        emit("mov r11, rax");
    }

    // 左辺は rax, 右辺は rdi に入れる
    emit("mov rax, r11");
    emit("mov rdi, r10");

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

    initial();

    for (int i = 0; i < parse_result->len; ++i)
    {
        context_function = (Function *)parse_result->data->data[i];
        // 関数を出力
        gen_function(context_function);
    }
}
