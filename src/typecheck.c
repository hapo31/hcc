#include <stddef.h>
#include <assert.h>

#include "typecheck.h"
#include "parser.h"

void node_type_check(Node *node);
TypeNode *get_term_type(Node *term);

Map *typecheck(Map *parse_result)
{
    for (int i = 0; i < parse_result->len; ++i)
    {
        Function *function = (Function *)parse_result->data->data[i];
        node_type_check(function->top_level_code);
    }
}

void node_type_check(Node *node)
{
    if (node == NULL ||
        node->type == ND_DEF_VAR ||
        node->type == ND_DEF_FUNCTION ||
        node->type == ND_EMPTY)
    {
        return;
    }

    if (node->type == ND_BLOCK)
    {
        for (int i = 0; i < node->block_items->len; ++i)
        {
            node_type_check((Node *)node->block_items->data[i]);
        }
    }

    if (node->type == ND_IF)
    {
        node_type_check(node->condition);
        node_type_check(node->else_);
        node_type_check(node->then);
    }

    if (node->type == ND_WHILE)
    {
        node_type_check(node->condition);
        node_type_check(node->then);
    }

    if (node->type == ND_DEREF)
    {
        TypeNode *lhs_type = get_term_type(node->lhs);
        if (lhs_type->type != NT_PTR)
        {
            error("デリファレンス(*)はポインタ型に対して使われる必要があります");
        }
    }

    if (node->type == '=')
    {
        node_type_check(node->lhs);
        node_type_check(node->rhs);
    }

    if (node->type == '*' || node->type == '/')
    {
        TypeNode *rhs_type = get_term_type(node->rhs);
        TypeNode *lhs_type = get_term_type(node->lhs);

        if (rhs_type->type == NT_PTR || lhs_type->type != NT_PTR)
        {
            error("%c はポインタに対して適用出来ません", node->type);
        }
    }

    node_type_check(node->lhs);
    node_type_check(node->rhs);
}

TypeNode *get_term_type(Node *term)
{

    if (term->node_type->type == NT_VOID)
    {
        error("void 型は演算出来ません");
    }

    if (term->type == ND_IDENT || term->type == ND_NUM)
    {
        return term->node_type;
    }

    Node *lhs = term->lhs;
    Node *rhs = term->rhs;

    TypeNode *lhs_type = NULL;
    TypeNode *rhs_type = NULL;

    // 両辺とも NULL ならバグっている
    assert(lhs != NULL || rhs != NULL);

    if (lhs == NULL || lhs->node_type->type == NT_UNKOWN)
    {
        return get_term_type(rhs);
    }

    if (rhs == NULL || rhs->node_type->type == NT_UNKOWN)
    {
        return get_term_type(lhs);
    }

    return rhs->node_type->type > lhs->node_type->type ? rhs->node_type : lhs->node_type;
}
