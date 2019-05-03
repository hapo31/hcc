#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

Vector *tokens;
Vector *code;

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
     * mul: unary
     * mul: mul "*" unary
     * mul: mul "/" unary
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
        return new_node(ND_GT, node, add());
    }
    else if (consume(TK_GE))
    {
        return new_node(ND_GE, node, add());
    }
    else
    {
        return node;
    }
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
