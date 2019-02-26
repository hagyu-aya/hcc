#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	TK_NUM = 256, // 整数
	TK_EOF // 終端
};

typedef struct{
	int type;
	int val; // type==TK_NUMの場合、数値
	char *input; // トークン文字列(エラーメッセージ用)
} Token;

typedef struct {
	Token* data;
	int capacity;
	int len;
} Vector;

Vector* new_vector(){
	Vector* vec = malloc(sizeof(Vector));
	vec->capacity = 16;
	vec->data = malloc(sizeof(Token) * 16);
	vec->len = 0;
	if(vec->data != NULL) return vec;
	printf("failed to initialize a vector\n");
	exit(1);
}

void vector_push(Vector* vec, Token elem){
	if(vec->capacity == vec->len){
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(Token) * vec->capacity);
	}
	vec->data[vec->len++] = elem;
}

Vector* tokens;
int pos = 0;

enum{
	ND_NUM = 256 // 整数
};

typedef struct Node{
	int type;
	struct Node* lhs;
	struct Node* rhs;
	int val; // type==ND_NUMの場合、数値
} Node;

void tokenize(char *p){
	tokens = new_vector();
	while(*p){
		if(isspace(*p)){
			p++;
			continue;
		}

		if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'){
			Token t;
			t.type = *p;
			t.input = p;
			vector_push(tokens, t);
			p++;
			continue;
		}

		if(isdigit(*p)){
			Token t;
			t.type = TK_NUM;
			t.input = p;
			t.val = strtol(p, &p, 10);
			vector_push(tokens, t);
			continue;
		}

		fprintf(stderr, "トークナイズできません: %s\n", p);
		exit(1);
	}
	Token t;
	t.type = TK_EOF;
	t.input = p;
	vector_push(tokens, t);
}

// エラー出力
void error(char* msg){
	fprintf(stderr, msg, tokens->data[pos].input);
	exit(1);
}

Node* new_node(int type, Node* lhs, Node* rhs){
	Node* node = malloc(sizeof(Node));
	node->type = type;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node* new_node_num(int val){
	Node* node = malloc(sizeof(Node));
	node->type = ND_NUM;
	node->val = val;
	return node;
}

int consume(int type){
	if(tokens->data[pos].type != type) return 0;
	pos++;
	return 1;
}

Node* mul(void);
Node* term(void);

Node* add(void){
	Node* node = mul();

	while(1){
		if(consume('+')) node = new_node('+', node, mul());
		else if(consume('-')) node = new_node('-', node, mul());
		else return node;
	}
}

Node* mul(void){
	Node* node = term();

	while(1){
		if(consume('*')) node = new_node('*', node, term());
		else if(consume('/')) node = new_node('/', node, term());
		else return node;
	}
}

Node* term(void){
	if(consume('(')){
		Node* node = add();
		if(!consume(')')) error("開きカッコに対応する閉じカッコがありません: %s\n");
		return node;
	}

	if(tokens->data[pos].type == TK_NUM) return new_node_num(tokens->data[pos++].val);
	error("数値、演算子、括弧のいずれでもないトークンです: %s");
}

void gen(Node* node){
	if(node->type == ND_NUM){
		printf("	push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);
	
	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch(node->type){
	case '+':
		printf("	add rax, rdi\n");
		break;
	case '-':
		printf("	sub rax, rdi\n");
		break;
	case '*':
		printf("	mul rdi\n");
		break;
	case '/':
		printf("	mov rdx, 0\n");
		printf("	div rdi\n");
	}

	printf("	push rax\n");
}

int main(int argc, char** argv){
	if(argc != 2){
		fprintf(stderr, "引数は1つのみです\n");
		return 1;
	}

	tokenize(argv[1]);
	Node *node = add();
	
	// アセンブリの表示
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	gen(node);

	printf("	pop rax\n");
	printf("	ret\n");
	return 0;
	
}