
enum
{
  TK_NUM = 256,
  TK_EOF,
};

enum
{
  ND_NUM = 256,
};

typedef struct
{
  int type;
  int value;
  char *input;
} Token;

typedef struct tagNode
{
  int type;
  struct tagNode *lhs;
  struct tagNode *rhs;
  int value;
} Node;

Node *new_node(int type, Node *lhs, Node *rhs);
Node *new_node_num(int value);

int consume(int type);
Node *add();
Node *mul();
Node *term();

void gen(Node *node);
void error(char *fmt, ...);
void tokenize(char *p);
