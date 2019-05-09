#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tokenizer.h"
#include "parser.h"
#include "codegen.h"
#include "vector.h"

char *read_file(const char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        return NULL;
    }
    FILE *fp = fdopen(fd, "r");
    if (fp == NULL)
    {
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        return NULL;
    }

    int c;
    int i = 0;
    int filesize = st.st_size;
    char *buf = malloc(sizeof(char) * (filesize + 1));

    while ((c = fgetc(fp)) != EOF)
    {
        buf[i] = (char)c;
        ++i;
    }

    fclose(fp);
    buf[i] = '\0';

    return buf;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usase:\n");
        printf("\t./hcc [source_file]\n");
        printf("\t./hcc -i [source_code]\n");
        return 1;
    }

    char *src = NULL;
    if (argc == 3 && strcmp(argv[1], "-i") == 0)
    {
        src = argv[2];
    }
    else
    {
        src = read_file(argv[1]);
    }

    // トークン列に分解
    TokenizeResult tokenize_result = tokenize(src);

    // 抽象構文木を作成
    ParseResult parse_result = parse(&tokenize_result);

    // コード出力
    codegen(stdout, &parse_result);

    return 0;
}