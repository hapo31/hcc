#pragma once

// 64bit なので 8byte
// レジスタが64bitなので、スタックに確保した変数領域も64bit取らないと悲しいこと(メモリの破壊)が起こる
// ていうか早く変数の型を決められるようになりたい…。
#define VAR_SIZE (8)

void error(char *fmt, ...);

int is_alpha_or_num(char c);
