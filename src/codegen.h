#pragma once

#include <stdio.h>

#include "parser.h"
#include "utils.h"
#include "map.h"
#include "vector.h"

void initial(FILE *fp);
void prologue(FILE *fp);
void gen(FILE *fp, Node *node);
void gen_lvalue(FILE *fp, Node *node);
void epilogue(FILE *fp);
void codegen(FILE *target, ParseResult *parse_result);
