#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません。\n");
        return 1;
    }

    // トークン列に分解
    TokenizeResult tokenize_result = tokenize(argv[1]);

    // 抽象構文木を作成
    ParseResult parse_result = parse(&tokenize_result);

    // コード出力
    codegen(stdout, &parse_result);

    return 0;
}