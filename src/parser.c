#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "parser.h"

static Vector *tokens;

static Map *functions;
static Function *context_function;

static size_t pos = 0;

char *input();
Token *token(Vector *token, int i);

Variable *new_variable(TypeNode *type_node, char *name, size_t index);

Node *new_node(NODE type, Node *lhs, Node *rhs);
Node *new_node_num(int value);
Node *new_node_identifier(char *name, TypeNode *type);
Node *new_node_call_function(char *name, TypeNode *type);
Node *new_if_node();
Node *new_for_node();
Node *new_while_node();
Node *new_block_node();

bool consume(int type);

bool current(int type);
bool next(int type);
bool prev(int type);

bool is_type_name();
bool is_variable_def();

Node *add();
Node *mul();
Node *unary();
Node *term();
Node *equality();
Node *relational();
Node *for_statement();
Node *if_statement();
Node *while_statement();
Node *block_items();
Node *statement();
Node *expression();
NODE_TYPE type_name();
TypeNode *type();
Vector *parameters();
Vector *arg_list();
Function *function_def();
Node *function_call();
Node *ret();

void program();

char *input()
{
    return ((Token *)tokens->data[pos])->input;
}

Token *token(Vector *token, int i)
{
    return (Token *)tokens->data[i];
}

Variable *new_variable(TypeNode *type_node, char *name, size_t index)
{
    Variable *var = (Variable *)malloc(sizeof(Variable));
    size_t name_len = strlen(name);
    var->type_node = type_node;
    var->name = (char *)malloc(sizeof(char) * (name_len + 1));
    var->index = index;
    strncpy(var->name, name, name_len);

    return var;
}

Node *new_node(NODE type, Node *lhs, Node *rhs)
{
    Node *node = (Node *)malloc(sizeof(Node));
    if (rhs != NULL)
    {
        // 右辺値の型を引き継ぐ
        node->node_type = rhs->node_type;
    }
    node->type = type;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int value)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->node_type = (TypeNode *)malloc(sizeof(TypeNode));
    node->node_type->type = NT_INT;
    node->type = ND_NUM;
    node->value = value;
    return node;
}

Node *new_node_identifier(char *name, TypeNode *type)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_IDENT;
    node->node_type = type;
    node->name = name;
    return node;
}

Node *new_node_call_function(char *name, TypeNode *type)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_CALL_FUCTION;
    node->node_type = type;
    node->name = name;
    return node;
}

Node *new_if_node()
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_IF;
    node->condition = NULL;
    node->then = NULL;
    node->else_ = NULL;

    return node;
}

Node *new_for_node()
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_FOR;
    node->then = NULL;
    node->init_expression = NULL;
    node->loop_expression = NULL;

    return node;
}

Node *new_while_node()
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_WHILE;
    node->condition = NULL;
    node->then = NULL;

    return node;
}

Node *new_block_node()
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_BLOCK;
    node->block_items = new_vector(1);

    return node;
}

bool consume(int type)
{
    if (((Token *)tokens->data[pos])->type != type)
    {
        return false;
    }
    ++pos;
    return true;
}

bool current(int type)
{
    return ((Token *)tokens->data[pos])->type == type;
}

bool next(int type)
{
    return pos + 1 < tokens->len && ((Token *)tokens->data[pos + 1])->type == type;
}

bool prev(int type)
{
    return pos - 1 >= 0 && ((Token *)tokens->data[pos - 1])->type == type;
}

bool is_type_name()
{
    TOKEN_TYPE type = token(tokens, pos)->type;
    return TK_INT <= type && type >= TK_INT;
}

bool is_variable_def()
{
    if (!is_type_name())
    {
        return false;
    }
    size_t typename_pos = pos + 1;
    TOKEN_TYPE token_type = token(tokens, typename_pos)->type;
    char *debug = input();
    while (token_type == '*')
    {
        ++typename_pos;
        token_type = token(tokens, typename_pos)->type;
        if (token_type == TK_EOF)
        {
            error("ポインタの型名が変です: %s ", debug);
        }
    }

    return token(tokens, typename_pos)->type == TK_IDENT;
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
     * mul: unary
     * mul: mul "*" unary
     * mul: mul "/" unary
     * mul: mul "%" unary
     */

    Node *node = unary();

    do
    {
        if (consume('*'))
        {
            node = new_node('*', node, unary());
        }
        else if (consume('/'))
        {
            node = new_node('/', node, unary());
        }
        else if (consume('%'))
        {
            node = new_node('%', node, unary());
        }
        else
        {
            return node;
        }
    } while (1);
}

Node *unary()
{
    /**
     * unary: term
     * unary: "+" term
     * unary: "-" term
     */

    if (consume('+'))
    {
        return term();
    }
    else if (consume('-'))
    {
        return new_node('-', new_node_num(0), term());
    }
    else
    {
        return term();
    }
}

Node *term()
{
    /**
     * term: num
     * term: ident
     * term: function_call
     * term: "*" term
     * term: "&" term
     * term: "(" expression ")"
     */
    if (consume('('))
    {
        Node *node = expression();

        if (!consume(')'))
        {
            error("開きカッコに対応する閉じカッコがありません: %s", input());
        }
        return node;
    }
    else if (consume('*'))
    {
        Node *node = term();
        Node *ptr_node = new_node(ND_DEREF, node, NULL);
        // 右辺の型をデリファレンスする
        ptr_node->node_type = node->node_type->ptr_of;
        return ptr_node;
    }
    else if (consume('&'))
    {
        Node *node = term();
        Node *target = new_node(ND_ADDR, NULL, node);
        target->node_type = (TypeNode *)malloc(sizeof(TypeNode));
        target->node_type->type = NT_PTR;
        target->node_type->ptr_of = node->node_type;

        return target;
    }
    else if (consume(TK_NUM))
    {
        return new_node_num(((Token *)tokens->data[pos - 1])->value);
    }
    else if (consume(TK_IDENT))
    {
        if (current('('))
        {
            return function_call();
        }

        char *identifier = ((Token *)tokens->data[pos - 1])->identifier;

        // 変数が定義されているかどうか
        if (!contains_map(context_function->variable_list, identifier))
        {
            error("変数が定義されていません: %s", identifier);
        }
        Variable *var = (Variable *)read_map(context_function->variable_list, identifier);
        return new_node_identifier(identifier, var->type_node);
    }

    error("数値でも開きカッコでもないトークンです: %s", input());
}

Node *equality()
{
    /**
     * equality: relational
     * equality: equality "==" relational
     * equality: equality "!=" relational
     */
    Node *node = relational();
    if (consume(TK_EQ))
    {
        return new_node(ND_EQ, node, relational());
    }
    else if (consume(TK_NE))
    {
        return new_node(ND_NE, node, relational());
    }
    else
    {
        return node;
    }
}

Node *relational()
{
    /**
     * relational: add
     * relational: relational "<"  add
     * relational: relational "<=" add
     * relational: relational ">"  add
     * relational: relational ">=" add
     */

    Node *node = add();

    if (consume(TK_LT))
    {
        return new_node(ND_LT, node, add());
    }
    else if (consume(TK_LE))
    {
        return new_node(ND_LE, node, add());
    }
    else if (consume(TK_GT))
    {
        // > は、左右の項を入れ替えて < 扱いにする
        return new_node(ND_LT, add(), node);
    }
    else if (consume(TK_GE))
    {
        // >= は、左右の項を入れ替えて <= 扱いにする
        return new_node(ND_LE, add(), node);
    }
    else
    {
        return node;
    }
}

Node *if_statement()
{
    /**
     * if_statement: "if" "(" cond ")" statement
     * if_statement: "if" "(" cond ")" statement "else" else_statement
     */
    Node *node = new_if_node();
    if (consume('('))
    {
        node->condition = expression();
        if (!consume(')'))
        {
            error("if文が)で閉じられていません: %s\n", input());
        }

        node->then = statement();

        if (consume(TK_ELSE))
        {
            node->else_ = statement();
        }
    }
    else
    {
        error("if文のあとに(がありません: %s\n", input());
    }

    return node;
}

Node *for_statement()
{
    /**
     * for "(" init_expression ";" cond ";" loop_expression ")" statement
     */
    Node *node = new_for_node();
    if (consume('('))
    {
        node->init_expression = expression();
        if (consume(';'))
        {
            node->condition = expression();
            if (consume(';'))
            {
                node->loop_expression = expression();
                if (consume(')'))
                {
                    node->then = statement();

                    return node;
                }
            }
        }
    }
    error("for文に誤りがあります: %s\n", input());

    // ここには来ない
    return NULL;
}

Node *while_statement()
{
    /**
     * while "(" cond ")" statement
     */

    Node *node = new_while_node();
    if (consume('('))
    {
        node->condition = expression();
        if (!consume(')'))
        {
            error("while文が)で閉じられていません: %s\n", input());
        }
        node->then = statement();
    }
    else
    {
        error("while文のあとに(がありません: %s\n", input());
    }

    return node;
}

Node *block_items()
{
    /**
     * block_items: statement
     * block_items: statement block_items
     */
    Node *node = new_block_node();

    while (((Token *)tokens->data[pos])->type != '}')
    {
        if (((Token *)tokens->data[pos])->type == TK_EOF)
        {
            error("ブロックが閉じられていません: %s\n", input());
        }
        push_vector(node->block_items, statement());
    }
    return node;
}

Node *statement()
{
    /**
     * statement: "{" block_items "}"
     * statement: type ident ";"
     * statement: if_statement
     * statement: while_statement
     * statement: for_statement
     * statement: function_call
     * statement: "return" expression ";"
     * statement: expression ";"
     */

    Node *node = NULL;
    if (consume('{'))
    {
        node = block_items();
        if (consume('}'))
        {
            return node;
        }
        else
        {
            error("ブロックが閉じられていません: %s", input());
        }
    }
    else if (consume(TK_IF))
    {
        return if_statement();
    }
    else if (consume(TK_WHILE))
    {
        return while_statement();
    }
    else if (consume(TK_FOR))
    {
        return for_statement();
    }
    else if (consume(TK_RETURN))
    {
        node = ret();
        node->lhs = expression();
    }
    // 変数定義
    else if (is_variable_def())
    {
        TypeNode *identifier_type = type();
        char *identifier = token(tokens, pos)->identifier;
        if (contains_map(context_function->variable_list, identifier))
        {
            error("変数名が重複しています: %s", identifier);
        }
        Variable *var = new_variable(identifier_type, identifier, context_function->variable_list->len);
        put_map(context_function->variable_list, identifier, var);

        ++pos;
        node = new_node(ND_DEF_VAR, NULL, NULL);
    }
    else
    {
        node = expression();
    }

    if (!consume(';'))
    {
        error("式が ; で閉じられていません: %s\n", input());
    }

    return node;
}

Node *expression()
{
    /**
     * expression: equality
     * expression: equality "=" expression
     */

    Node *node = equality();

    while (consume('='))
    {
        node = new_node('=', node, expression());
    }

    return node;
}

Vector *arg_list()
{
    /**
     * arg_list: ε
     * arg_list: expression
     * arg_list: expression "," arg_list
     */
    Vector *args = new_vector(1);
    while (!current(')'))
    {
        push_vector(args, expression());
        if (consume(','))
        {
            continue;
        }
    }
    return args;
}

Vector *parameters()
{
    /**
     * parameter_list: ε
     * parameter_list: parameter
     * parameter_list: parameter "," parameter_list
     */
    Vector *vec = new_vector(1);

    if (consume(')'))
    {
        return vec;
    }

    do
    {
        TypeNode *param_type = type();
        Variable *var = new_variable(param_type, token(tokens, pos)->identifier, 0);
        push_vector(vec, var);
        ++pos;
    } while (consume(','));
    if (consume(')'))
    {
        return vec;
    }
    else
    {
        error("仮引数リストが)で閉じられていません: %s", input());
    }
}

NODE_TYPE type_name()
{
    /*
     * type_name: "int"
     */

    NODE_TYPE type = -1;
    switch (token(tokens, pos)->type)
    {
    case TK_INT:
        type = NT_INT;
        break;

    default:
        error("有効な型名ではありません: %s", input());
        break;
    }
    ++pos;

    return type;
}

TypeNode *type()
{
    /**
     * type: "int"
     * type: type "*"
     */

    TypeNode *type_node = (TypeNode *)malloc(sizeof(TypeNode));
    type_node->type = type_name();
    type_node->ptr_of = NULL;
    while (token(tokens, pos)->type == '*')
    {
        TypeNode *new_type_node = (TypeNode *)malloc(sizeof(TypeNode));
        new_type_node->type = NT_PTR;
        new_type_node->ptr_of = NULL;
        type_node->ptr_of = type_node;
        type_node = new_type_node;
        ++pos;
    }
    return type_node;
}

Function *function_def()
{
    /**
     * function_def: type_name function_name "(" parameters ")"
     */
    Function *function = (Function *)malloc(sizeof(Function));
    // コンテキストを保存
    context_function = function;

    function->variable_list = new_map();

    char *name = token(tokens, pos - 2)->identifier;

    if (!contains_map(functions, name))
    {
        function->name = name;
        put_map(functions, name, function);
    }
    else
    {
        error("関数定義が重複しています。: %s", input());
    }

    // 仮引数リストをパース
    Vector *parameters_ = parameters();

    // 関数宣言なら引数は無いものとしてとりあえず終わる
    if (consume(';'))
    {
        function->top_level_code = NULL;
        function->parameter_count = parameters_->len;
        return function;
    }

    // 仮引数の数を保存
    function->parameter_count = parameters_->len;

    Map *variable_list = function->variable_list;
    // 引数定義を変数定義に変換する
    for (int i = 0; i < parameters_->len; ++i)
    {
        Variable *param = parameters_->data[i];
        param->index = (size_t)i;
        put_map(variable_list, param->name, param);
    }

    // 関数定義
    if (consume('{'))
    {
        function->top_level_code = block_items();
        if (!consume('}'))
        {
            error("ブロックが閉じられていません: %s", input());
        }
    }
    else
    {
        error("関数定義が変です: %s", input());
    }

    return function;
}

Node *function_call()
{
    /**
     * function: ident "(" arg_list ")"
     *
     */

    char *name = ((Token *)tokens->data[pos - 1])->identifier;
    if (!contains_map(functions, name))
    {
        error("存在しない関数の呼び出しです: %s", name);
    }
    Function *function = (Function *)read_map(functions, name);

    Node *node = new_node_call_function(name, function->return_type);

    if (consume('('))
    {
        node->args = arg_list();
        if (!consume(')'))
        {
            error("関数呼び出しが ) で閉じられていません: %s", input());
        }
        return node;
    }

    error("関数呼び出しが変です: %s", input());
    return NULL;
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
     * program: function_definition program
     * program: ε
     *
     * function_declaration: identifier "(" parameter_list ")" ";"
     * function_definition: identifier "(" parameter_list ")" "{" statement "}" program
     */

    while (((Token *)tokens->data[pos])->type != TK_EOF)
    {
        TypeNode *function_return_type = NULL;
        if (is_type_name())
        {
            function_return_type = type();
        }
        if (consume(TK_IDENT))
        {
            if (consume('('))
            {
                Function *function = function_def();
                function->return_type = function_return_type;
                put_map(functions, function->name, function);
            }
        }
        else
        {
            error("トップレベルの関数定義が変です: %s", input());
        }
    }
}

Map *parse(Vector *tokenize_result)
{
    tokens = tokenize_result;
    functions = new_map();

    program();

    return functions;
}
