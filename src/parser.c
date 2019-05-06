#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

Vector *tokens;
Vector *code;

int pos = 0;

char *input()
{
    return ((Token *)tokens->data[pos])->input;
}

Node *new_node(NODE_TYPE type, Node *lhs, Node *rhs)
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
     * term: "(" assign ")"
     */
    if (consume('('))
    {
        Node *node = assign();

        if (!consume(')'))
        {
            error("開きカッコに対応する閉じカッコがありません: %s", input());
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
        node->condition = assign();
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
        node->init_expression = assign();
        if (consume(';'))
        {
            node->condition = assign();
            if (consume(';'))
            {
                node->loop_expression = assign();
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
        node->condition = assign();
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

Node *statement()
{
    /**
     * statement: if_statement
     * statement: while_statement
     * statement: for_statement
     * statement: "return" assign ";"
     * statement: asign ";"
     */

    Node *node = NULL;
    if (consume(TK_IF))
    {
        return if_statement();
    }
    else if (consume(TK_FOR))
    {
        return for_statement();
    }
    else if (consume(TK_WHILE))
    {
        return while_statement();
    }
    else if (consume(TK_RETURN))
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
        error("式が ; で閉じられていません: %s\n", input());
    }

    return node;
}

Node *assign()
{
    /**
     * assign: equality
     * assign: equality "=" assign
     */
    Node *node = equality();
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

ParseResult parse(TokenizeResult *tokenize_result)
{
    tokens = tokenize_result->tokens;
    code = new_vector(5);
    program();

    ParseResult result = {code, tokenize_result->identifiers};

    return result;
}
