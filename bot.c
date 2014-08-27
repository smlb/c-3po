#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>
#include <math.h> 
#include "bot.h"

// Initialize the bot in the struct
void bot_init(struct IRC *bot, char *s, char *p, char *n, char *c){
	strcpy(bot->server, s);
	strcpy(bot->port, p);
	strcpy(bot->nick, n);
	strcpy(bot->chan, c);
}

// Send message over the connection <3
void bot_raw(struct IRC *bot, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(bot->sbuf, 512, fmt, ap);
	va_end(ap);
	printf("<< %s", bot->sbuf);
	write(bot->conn, bot->sbuf, strlen(bot->sbuf));
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
	bot->conn = socket(bot->res->ai_family, bot->res->ai_socktype, bot->res->ai_protocol);
	
	// Try to connect
	if(connect(bot->conn, bot->res->ai_addr, bot->res->ai_addrlen)<0){
		c_error(stderr,"Error: Connection Failed\n");
		return -1;
	}
	
	// From RFC: USER <username> <hostname> <servername> <realname>
	bot_raw(bot,"USER C-3PO_bot NewRepublic StarWars :C3PO\r\n");
	// From RFC: NICK <nickname>
  bot_raw(bot,"NICK %s\r\n", bot->nick);
	
	// All the program remain here waiting for channel-input
	while ((sl = read(bot->conn, bot->sbuf, 512))) {
		// Read the message char by char
		for (i = 0; i < sl; i++) {
			o++;
			buf[o] = bot->sbuf[i];
			// If the char is  endline \r\n
			if ((i > 0 && bot->sbuf[i] == '\n' && bot->sbuf[i - 1] == '\r') || o == 512) {
				buf[o + 1] = '\0';
				l = o;
				o = -1;
				
				// Log into the terminal
				printf(">> %s", buf);
				
				// When there is a PING, reply with PONG
				if (!strncmp(buf, "PING", 4)) {
					buf[1] = 'O';
					bot_raw(bot,buf);
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
						bot_raw(bot,"JOIN %s\r\n", bot->chan);
						bot_raw(bot,"PRIVMSG %s :Hello world!\r\n", bot->chan);
					} else if (!strncmp(command, "PRIVMSG", 7) || !strncmp(command, "NOTICE", 6)) {
						if (where == NULL || message == NULL) continue;
						if ((sep = strchr(user, '!')) != NULL) user[sep - user] = '\0';
						if (where[0] == '#' || where[0] == '&' || where[0] == '+' || where[0] == '!') target = where; else target = user;
						printf("[from: %s] [reply-with: %s] [where: %s] [reply-to: %s] %s", user, command, where, target, message);
						bot_parse_action(bot, user, command, where, target, message);
					}
				}
			}
		} 
	}
	return 0;
}

int bot_parse_action(struct IRC *bot, char *user, char *command, char *where, char *target, char *msg){
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
		bot_raw(bot,"PRIVMSG %s :pong\r\n", bot->chan);
	}
	else if(strcasecmp(argv[0], "count") == 0){
		bot_raw(bot,"PRIVMSG %s :%d\r\n", bot->chan, i);
	}
	else if(strcasecmp(argv[0], "quit") == 0){
		bot_raw(bot,"QUIT :C-3PO Bot\r\n", bot->chan, i);
	}
	else if((strcasecmp(argv[0], "google") == 0) && argv[1] != NULL){
		if(argv[2] != NULL){
			bot_raw(bot,"PRIVMSG %s :%s: http://lmgtfy.com/?q=%s\r\n", bot->chan, argv[2], argv[1]);
		} else {
			bot_raw(bot,"PRIVMSG %s :%s: http://lmgtfy.com/?q=%s\r\n", bot->chan, user, argv[1]);
		}	
	}
	else if((strcasecmp(argv[0], "ddg") == 0) && argv[1] != NULL){
		if(argv[2] != NULL){
			bot_raw(bot,"PRIVMSG %s :%s: http://duckduckgo.com/?q=%s\r\n", bot->chan, argv[2], argv[1]);
		} else {
			bot_raw(bot,"PRIVMSG %s :%s: http://duckduckgo.com/?q=%s\r\n", bot->chan, user, argv[1]);	
		}
	}
	else if(strcasecmp(argv[0], "sqrt") == 0) {
		double x = atof(argv[1]);
		bot_raw(bot,"PRIVMSG %s :%s: %g\r\n", bot->chan, user, sqrt(x));
	}
	else if((strcasecmp(argv[0], "archwiki") == 0) && argv[1] != NULL) {
		if(argv[2] != NULL) {
			bot_raw(bot,"PRIVMSG %s :%s: https://wiki.archlinux.org/index.php/%s\r\n", bot->chan, argv[2], argv[1]);
		} else { 
			bot_raw(bot,"PRIVMSG %s :%s: https://wiki.archlinux.org/index.php/%s\r\n", bot->chan, user, argv[1]);
		}
	}
	else if(strcasecmp(argv[0], "whoami") == 0) {
		bot_raw(bot,"PRIVMSG %s :I'm a bot, developed using Coffee and Beer. You are %s, I think you know...\r\n", bot->chan, user);
	}
	
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
