
typedef enum
{
  TK_NUM = 256,
  TK_IDENT,
  TK_RETURN,
  TK_EOF,
} TOKEN_TYPE;

typedef enum
{
  ND_NUM = 256,
  ND_IDENT,
  ND_RETURN,
} NODE_TYPE;

typedef struct
{
  TOKEN_TYPE type;
  // type が TK_NUM のとき、数値
  int value;
  // type が TK_IDENT のとき、識別子の名前
  char *identifier;
  // デバッグ用
  char *input;
} Token;

typedef struct tagNode
{
  NODE_TYPE type;
  struct tagNode *lhs;
  struct tagNode *rhs;
  int value;
  char *name;
} Node;

Node *new_node(int type, Node *lhs, Node *rhs);
Node *new_node_num(int value);
Node *new_node_identifier(char *name);

int consume(int type);
Node *add();
Node *mul();
Node *term();
Node *assign();
Node *statement();
Node *ret();

void program();

void gen(Node *node);
void gen_lvalue(Node *node);
void error(char *fmt, ...);

int is_alpha_or_num(char c);
void tokenize(char *p);
