#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "op.h"
#include "bot.h"

int parse_op(struct IRC *bot, char* msg){
	int i=0;
	char **ap, *argv[10];
	
	for(ap = argv; (*ap = strsep(&msg, " \t")) != NULL;){
		if(**ap != '\0'){
			if(++ap >= &argv[10]){
				break;
			}
		}
	}
	// Count the argument
	while(argv[i] != NULL){
		if(argv[i][0]=='@'){
			add_op(bot,&argv[i][1]);
		}
		i++;
	}
	printf("User: %d - OP: %d\n",i,bot->opn);	
	bot_raw(bot, "PRIVMSG %s :User: %d - OP: %d\r\n", bot->chan,i,bot->opn);
	return i;
}

int add_op(struct IRC *bot, char* nick){
	bot->op = realloc(bot->op, (bot->opn+1) * NICKNAME_LIMIT * sizeof(char));
	if(bot->op == NULL) {
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	int offset = bot->opn * NICKNAME_LIMIT * sizeof(char);
	strcpy(&(bot->op[offset]),nick);
	bot->opn++;
	offset = bot->opn * NICKNAME_LIMIT * sizeof(char);
	bot->op[offset-1] = 0;
	return 1;
}

int rm_op(struct IRC *bot, char* nick){
	int index = is_op(bot,nick);
	if(index>-1){
		// scorro dal index in poi
		for(int i=index;i<(bot->opn);i++){
			int offset = i * NICKNAME_LIMIT * sizeof(char);
			int offset2 = (i+1) * NICKNAME_LIMIT * sizeof(char);
			strncpy(&(bot->op[offset]),&(bot->op[offset2]),NICKNAME_LIMIT);
		}
		bot->opn--;
		bot->op = realloc(bot->op, bot->opn * NICKNAME_LIMIT * sizeof(char));
		if(bot->op == NULL) {
			printf("not enough memory (realloc returned NULL)\n");
			return 0;
		}
		return 1;
	}
	return 0;
}

void print_op(struct IRC *bot){
	for(int i=0;i<(bot->opn);i++){
		int offset = i * NICKNAME_LIMIT * sizeof(char);
		printf("%s\n",&(bot->op[offset]));
	}
	printf("%d\n",bot->opn);
}

int is_op(struct IRC *bot, char* nick){
	for(int i=0;i<(bot->opn);i++){
		int offset = i * NICKNAME_LIMIT * sizeof(char);
		if(strcmp(&(bot->op[offset]),nick)==0){
			return i;
		}
	}
	return -1;
}


