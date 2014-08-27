/*
 * main.c
 * 
 * Copyright 2014 Freeedom Fighter <insert mail here>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 *                      YOU SHALL NOT PASSS!                          *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>

// Simple struct with bot configuration ;)
struct IRC{
	char server[100];
	char port[5];
	char nick[15];
	char chan[20];
	struct addrinfo hints, *res;
}; 

// Prototypes
int load_config(char *filename, char* s, char *p, char* n, char* c);
int save_config(char *filename, char *s, char *p, char *n, char *c);
void c_error(FILE *out, const char *fmt, ...);
// All bot-related function should start with 'bot_'
void bot_init(struct IRC *bot, char *s, char *p, char *n, char *c);
int bot_connect(struct IRC *bot);
void bot_pong(struct IRC *bot, char *buff);
void bot_raw(char *fmt, ...);
int bot_parse_action(char *user, char *command, char *where, char *target, char *message);

// Global Struct variable (I know isn't a good idea)
struct IRC bot;
int conn;
char sbuf[512];

int main(int argc, char *argv[]){
	printf("C-3PO - 0.0.1\n  This software is under the GPL License\n");
	
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

// Magic function to throw error to stderr or other file
void c_error(FILE *out, const char *fmt, ...){
	// With a va_list we can print string like printf argument
	// es -  "%s", string -
	va_list ap;
	va_start(ap, fmt);
	vfprintf(out, fmt, ap);
	va_end(ap);
}

// Initialize the bot in the struct
void bot_init(struct IRC *bot, char *s, char *p, char *n, char *c){
	strcpy(bot->server, s);
	strcpy(bot->port, p);
	strcpy(bot->nick, n);
	strcpy(bot->chan, c);
}

// Send message over the connection <3
void bot_raw(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(sbuf, 512, fmt, ap);
    va_end(ap);
    printf("<< %s", sbuf);
    write(conn, sbuf, strlen(sbuf));
}

// Connect the bot on the network
int bot_connect(struct IRC *bot){
	char *user, *command, *where, *message, *sep, *target;
	int i, j, l, sl, o = -1, start, wordcount;
	char buf[513];

	// So many socket things :(	
	memset(&(bot->hints), 0, sizeof bot->hints);
	bot->hints.ai_family = AF_INET;
	bot->hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(bot->server, bot->port, &(bot->hints), &(bot->res));
	conn = socket(bot->res->ai_family, bot->res->ai_socktype, bot->res->ai_protocol);
	
	// Try to connect
	if(connect(conn, bot->res->ai_addr, bot->res->ai_addrlen)<0){
		c_error(stderr,"Error: Connection Failed\n");
		return -1;
	}
	
	// From RFC: USER <username> <hostname> <servername> <realname>
	bot_raw("USER C-3PO_bot NewRepublic StarWars :C3PO\r\n");
	// From RFC: NICK <nickname>
  bot_raw("NICK %s\r\n", bot->nick);
	
	// All the program remain here waiting for channel-input
	while ((sl = read(conn, sbuf, 512))) {
		// Read the message char by char
		for (i = 0; i < sl; i++) {
			o++;
			buf[o] = sbuf[i];
			// If the char is  endline \r\n
			if ((i > 0 && sbuf[i] == '\n' && sbuf[i - 1] == '\r') || o == 512) {
				buf[o + 1] = '\0';
				l = o;
				o = -1;
				
				// Log into the terminal
				printf(">> %s", buf);
				
				// When there is a PING, reply with PONG
				if (!strncmp(buf, "PING", 4)) {
					buf[1] = 'O';
					bot_raw(buf);
				} else if (buf[0] == ':') {
					wordcount = 0;
					user = command = where = message = NULL;
					for (j = 1; j < l; j++) {
						if (buf[j] == ' ') {
							buf[j] = '\0';
							wordcount++;
							switch(wordcount) {
								case 1: user = buf + 1; break;
								case 2: command = buf + start; break;
								case 3: where = buf + start; break;
							}
							if (j == l - 1) continue;
							start = j + 1;
						} else if (buf[j] == ':' && wordcount == 3) {
							if (j < l - 1) message = buf + j + 1;
							break;
						}
					}
					
					if (wordcount < 2) continue;
					
					if (!strncmp(command, "001", 3) && bot->chan != NULL) {
						// From RFC: JOIN <channel> [key]  
						// Maybe we can support password protected channel (?)
						bot_raw("JOIN %s\r\n", bot->chan);
						bot_raw("PRIVMSG %s :Hello world!\r\n", bot->chan);
					} else if (!strncmp(command, "PRIVMSG", 7) || !strncmp(command, "NOTICE", 6)) {
						if (where == NULL || message == NULL) continue;
						if ((sep = strchr(user, '!')) != NULL) user[sep - user] = '\0';
						if (where[0] == '#' || where[0] == '&' || where[0] == '+' || where[0] == '!') target = where; else target = user;
						printf("[from: %s] [reply-with: %s] [where: %s] [reply-to: %s] %s", user, command, where, target, message);
						bot_parse_action(user, command, where, target, message);
					}
				}
			}
		} 
	}
	return 0;
}

int bot_parse_action(char *user, char *command, char *where, char *target, char *msg){
// Private message example
//	[from: Th3Zer0] [reply-with: PRIVMSG] [where: C-3PO_bot] [reply-to: Th3Zer0] ciao
// Channel message example
//	[from: Th3Zer0] [reply-with: PRIVMSG] [where: ##freedomfighter] [reply-to: ##freedomfighter] ciao
	if(*msg != '!')
		return 1;

	char *cmd;
	//char *arg[6];
	int i=0;
	char **ap, *argv[10];
	
	// Clean the string
	cmd = strtok(msg, "!");	
	cmd = strtok(cmd, "\n\r");
	// Split the argument
	for(ap = argv; (*ap = strsep(&cmd, " \t")) != NULL;){
		if(**ap != '\0'){
			if(++ap >= &argv[10]){
				break;
			}
		}
	}
	// Count the argument
	while(argv[i] != NULL){
		printf("%d ",i);
		i++;
	}
	// the command is in the first argument
	if(argv[0] == NULL)
		return 1;
	
	// Fuck That
	if(strcasecmp(argv[0], "ping") == 0){
		bot_raw("PRIVMSG %s :pong\r\n", bot.chan);
	}
	else if(strcasecmp(argv[0], "count") == 0){
		bot_raw("PRIVMSG %s :%d\r\n", bot.chan, i);
	}
	else if(strcasecmp(argv[0], "quit") == 0){
		bot_raw("QUIT :C-3PO Bot\r\n", bot.chan, i);
	}
	else if((strcasecmp(argv[0], "google") == 0) && argv[1] != NULL){
		if(argv[2] != NULL){
			bot_raw("PRIVMSG %s :%s: http://lmgtfy.com/?q=%s\r\n", bot.chan, argv[2], argv[1]);
		} else {
			bot_raw("PRIVMSG %s :%s: http://lmgtfy.com/?q=%s\r\n", bot.chan, user, argv[1]);
		}	
	}
	else if((strcasecmp(argv[0], "ddg") == 0) && argv[1] != NULL){
		if(argv[2] != NULL){
			bot_raw("PRIVMSG %s :%s: http://duckduckgo.com/?q=%s\r\n", bot.chan, argv[2], argv[1]);
		} else {
			bot_raw("PRIVMSG %s :%s: http://duckduckgo.com/?q=%s\r\n", bot.chan, user, argv[1]);	
		}
	}
	
	return 0;
}
