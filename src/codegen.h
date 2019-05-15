#pragma once

#include <stdio.h>

#include "parser.h"
#include "utils.h"
#include "map.h"
#include "vector.h"

void codegen(FILE *target, Map *parse_result);
