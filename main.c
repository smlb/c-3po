/*
 *   If you are searching the license, try the license.txt file ;)    *
 * 
 *                      YOU SHALL NOT PASSS!                          *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>
#include "bot.h"

// Prototypes
int load_config(char *filename, char* s, char *p, char* n, char* c);
int save_config(char *filename, char *s, char *p, char *n, char *c);

// Global Struct variable (I know isn't a good idea)
struct IRC bot;

int main(int argc, char *argv[]){
	char version[10];
	sprintf(version,"%d.%d.%d", version_major, version_minor, version_patch);
	printf("C-3PO - %s\n  This software is under the GPL License\n", version);
	
	// 5 argument - start the bot
	if(argc == 5){
		bot_init(&bot, argv[1], argv[2], argv[3], argv[4]);
	}
	// 6 argument - save configuration and start the bot
	else if(argc == 6){
		save_config(argv[5], argv[1], argv[2], argv[3], argv[4]);
		bot_init(&bot, argv[1], argv[2], argv[3], argv[4]);
	}
	// 2 argument - load configuration and start the bot
	else if(argc == 2){
		char server[100];
		char port[5];
		char nick[15];
		char chan[20];
		int result = load_config(argv[argc-1], server, port, nick, chan);
		if(result == 0){
			bot_init(&bot, server, port, nick, chan);
		}
		else {
			return 1;
		}
	}
	// You had only one job.
	else {
		c_error(stderr,"Error: Only %d Argument, 2, 5 or 6 needed \nSintax: %s <ip> <port> <nick> <chan>\nSintax: %s <ip> <port> <nick> <chan> <config_file>\nSintax: %s <config_file>\n  Insert chan escaped with \\ (\\#free or \\#\\#beer)\n", argc, argv[0], argv[0], argv[0]);
		exit(1);
	}
	
	// Now print the bot configuration, tomorrow the world
	printf("Bot: %s %s %s %s\n", bot.server, bot.port, bot.nick, bot.chan);
	
	if(bot_connect(&bot) == -1)
		return -1;
		
	return 0;
}

// Load the configuration from file
int load_config(char *filename, char *s, char *p, char *n, char *c){
	FILE *f;
	char row[128+1];
	
	f = fopen (filename , "r");
	if(!f){
		c_error(stderr,"Error: Config file not found \n");
		return 1;
	}
	
	// Load the file line by line
	// Yes it's a strange for
	for(int i; fgets(row,128,f) != NULL; i++){  
		if(row[strlen(row)-1] == '\n'){
			row[strlen(row)-1] = '\0';
		}
		switch(i){
			case 0:	
				strcpy(s,row);
				break;
			case 1:
				strcpy(p,row);
				break;
			case 2:
				strcpy(n,row);
				break;
			case 3:
				strcpy(c,row);
				break;
		}
  }
	
  fclose(f);
  return 0;
}

// Save configuration on file
int save_config(char *filename, char *s, char *p, char *n, char *c){
	FILE *f;
	
	f = fopen(filename,"w");
	if(!f){
		c_error(stderr,"Error: Config file not found \n");
		return 1;
	}
	// Print configuration line by line
	fprintf(f,"%s\n%s\n%s\n%s\n", s, p, n, c);
	// Output to the user
	printf("Configuration saved on: %s\n", filename);
	
	fclose(f);
	return 0;
}


