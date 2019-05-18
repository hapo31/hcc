#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "parser.h"

static Vector *tokens;

static Map *functions;
static Function *context_function;

static int pos = 0;

char *input();
Token *token(Vector *token, int i);

Node *new_node(NODE type, Node *lhs, Node *rhs);
Node *new_node_num(int value);
Node *new_node_identifier(char *name);
Node *new_node_call_function(char *name);
Node *new_if_node();
Node *new_for_node();
Node *new_while_node();
Node *new_block_node();

bool consume(int type);

bool current(int type);
bool next(int type);
bool prev(int type);

bool is_typename();

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

Node *new_node(NODE type, Node *lhs, Node *rhs)
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

Node *new_node_identifier(char *name)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_IDENT;
    node->name = name;
    return node;
}

Node *new_node_call_function(char *name)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = ND_CALL_FUCTION;
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
        return new_node_identifier(identifier);
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
     * statement: type_name ident ";"
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
    else if (is_type_name() && next(TK_IDENT))
    {
        char *identifier = token(tokens, pos + 1)->identifier;
        size_t len = strlen(identifier);
        if (contains_map(context_function->variable_list, identifier))
        {
            error("変数名が重複しています: %s", token(tokens, pos + 1));
        }
        Variable *var = (Variable *)malloc(sizeof(Variable));
        var->name = (char *)malloc(sizeof(char) * (len + 1));
        strncpy(var->name, identifier, len);
        // 型の種類を取得
        var->type = type_name();
        put_map(context_function->variable_list, identifier, var);

        pos += 2;

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

Node *semicoron_list()
{
    /**
     * semicoron_list: expression
     * semicoron_list: expression "," semicoron_list
     * semicoron_list: ε
     */

    Node *node = new_node(ND_SEMI_EXPR_LIST, NULL, NULL);
    node->block_items = new_vector(1);

    do
    {
        push_vector(node->block_items, expression());
    } while (consume(','));

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
        // この辺に引数の型の構文

        // 引数名
        push_vector(vec, new_node_identifier(token(tokens, pos)->identifier));
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

    switch (token(tokens, pos)->type)
    {
    case TK_INT:
        return NT_INT;
        break;

    default:
        break;
    }
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
    // 仮引数の数を保存
    function->parameter_count = parameters_->len;

    Map *variable_list = function->variable_list;
    // 引数定義を変数定義に変換する
    for (int i = 0; i < parameters_->len; ++i)
    {
        Node *param = parameters_->data[i];
        put_map(variable_list, param->name, (void *)(intptr_t)i);
    }

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

    Node *node = new_node_call_function(((Token *)tokens->data[pos - 1])->identifier);

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
     * function_definition: identifier "(" parameter_list ")" "{" statement "}" program
     */

    while (((Token *)tokens->data[pos])->type != TK_EOF)
    {
        // NODE_TYPE function_return_type = NT_VOID;
        // // トークンが型名かどうかをチェック
        // // 今は TK_INT だけだけど、型が増えたらenumが範囲に収まっているかという式にする
        // if (current(TK_INT))
        // {
        // }
        if (consume(TK_IDENT))
        {
            if (consume('('))
            {
                Function *function = function_def();
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
