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

Token tokens[100];

void tokenize(char *p){
	int i = 0;
	while(*p){
		if(isspace(*p)){
			p++;
			continue;
		}

		if(*p == '+' || *p == '-'){
			tokens[i].type = *p;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}

		if(isdigit(*p)){
			tokens[i].type = TK_NUM;
			tokens[i].input = p;
			tokens[i].val = strtol(p, &p, 10);
			i++;
			continue;
		}

		fprintf(stderr, "トークナイズできません: %s\n", p);
		exit(1);
	}

	tokens[i].type = TK_EOF;
	tokens[i].input = p;
}

// エラー出力
void error(int i){
	fprintf(stderr, "予期しないトークン: %s\n", tokens[i].input);
	exit(1);
}

int main(int argc, char** argv){
	if(argc != 2){
		fprintf(stderr, "引数は1つのみです\n");
		return 1;
	}

	tokenize(argv[1]);
	
	// アセンブリの表示
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	if(tokens[0].type != TK_NUM) error(0);
	printf("	mov rax, %d\n", tokens[0].val);
	
	int i = 1;
	while(tokens[i].type != TK_EOF){
		if(tokens[i].type == '+'){
			i++;
			if(tokens[i].type != TK_NUM) error(i);
			printf("	add rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		if(tokens[i].type == '-'){
			i++;
			if(tokens[i].type != TK_NUM) error(i);
			printf("	sub rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		error(i);
	}

	printf("	ret\n");
	return 0;
}