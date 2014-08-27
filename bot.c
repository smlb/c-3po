#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <math.h> 
#include <time.h>
#include "bot.h"

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
						//printf("[from: %s] [reply-with: %s] [where: %s] [reply-to: %s] %s", user, command, where, target, message);
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

	if(strstr(msg,"<3") || strstr(msg,"love")){
		bot_raw(bot,"PRIVMSG %s :%s: so much LOVE <3 <3\r\n", bot->chan, user);
		bot_action(bot,bot->chan,"feeling lovely");
	}
	if(strstr(msg,"fuck")){
		bot_raw(bot,"PRIVMSG %s :%s: don't say bad words!\r\n", bot->chan, user);
		bot_action(bot,bot->chan,"is angry");
	}
	
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
		i++;
	}
	// the command is in the first argument
	if(argv[0] == NULL)
		return 1;
	
	// Fuck That
	if(strcasecmp(argv[0], "ping") == 0){
		bot_raw(bot,"PRIVMSG %s :pong\r\n", bot->chan);
	}
	else if(strcasecmp(argv[0], "help") == 0){
		char h1[] = "!help !ping !quit !google !ddg !sqrt !archwiki !whoami !attack !lookup !away <3";
		bot_raw(bot,"PRIVMSG %s :%s\r\n", bot->chan, h1);
		char h2[] = "Type !help <cmd> for information about that command.";
		bot_raw(bot,"PRIVMSG %s :%s\r\n", bot->chan, h2);
	}
	else if(strcasecmp(argv[0], "count") == 0){
		bot_raw(bot,"PRIVMSG %s :%d\r\n", bot->chan, i);
	}
	else if(strcasecmp(argv[0], "quit") == 0){
		bot_quit(bot);
	}
	else if(strcasecmp(argv[0], "away") == 0){
		bot_away(bot);
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
	else if((strcasecmp(argv[0], "attack") == 0) && argv[1] != NULL) {
		srand(time(NULL));
		int critical;
		critical = (rand()%10)/8;
		if(critical){
			bot_raw(bot,"PRIVMSG %s :\001ACTION attacks %s! %d damage (it's super effective).\001\r\n", bot->chan, argv[1], rand()%20 + 21);
		} else {
			bot_raw(bot,"PRIVMSG %s :\001ACTION attacks %s! %d damage.\001\r\n", bot->chan, argv[1], rand()%20 + 1);
		}
	}
	else if((strcasecmp(argv[0], "lookup") == 0) && argv[1] != NULL) {
		struct hostent* hp;
		struct in_addr **addr_list;
		if((hp = gethostbyname(argv[1])) == NULL){
			bot_raw(bot,"PRIVMSG %s :%s: The host %s is unreachable.\r\n", bot->chan, user, argv[1]);
		} else {
			addr_list = (struct in_addr **)hp->h_addr_list;
			bot_raw(bot,"PRIVMSG %s :%s: IP: %s\r\n", bot->chan, user, inet_ntoa(*addr_list[0]));
		}
  }
  else if(strcasecmp(argv[0], "life") == 0) {
    bot_raw(bot, "PRIVMSG %s :%s: 42\r\n", bot->chan, user);
  }
  else if(strcasecmp(argv[0], "rms") == 0) {
    bot_raw(bot, "PRIVMSG %s :Stallman approves.\r\n", bot->chan);
  }
  else if(strcasecmp(argv[0], "random") == 0) {
    srand(time(NULL));
    bot_raw(bot, "PRIVMSG %s :%s: here you are a $RANDOM number -> %d\r\n", bot->chan, user, rand());
  }
  else if(strcasecmp(argv[0], "privacy") == 0 ) {
    if(argv[1] != NULL) {
      bot_raw(bot, "PRIVMSG %s :%s: https://eff.org | https://prism-break.org | https://torproject.org | http://stallman.org\r\n", bot->chan, argv[1]);
    } else {
      bot_raw(bot, "PRIVMSG %s :%s: https://eff.org | https://prism-break.org | https://torproject.org | http://stallman.org\r\n", bot->chan, user);
    }
  }
	return 0;
}

// bot_join: For joining a channel
void bot_join(struct IRC *bot, const char *chan){
	bot_raw(bot,"JOIN %s\r\n", chan);
}
// bot_part: For leaving a channel
void bot_part(struct IRC *bot, const char *chan){
	bot_raw(bot,"PART %s\r\n", chan);
}
// bot_nick: For changing your nick
void bot_nick(struct IRC *bot, const char *data){
	bot_raw(bot,"NICK %s\r\n", data);
}
// bot_away: For quitting IRC
void bot_away(struct IRC *bot){
	bot_raw(bot,"AWAY :C-3PO Bot\r\n");
}
// bot_quit: For quitting IRC
void bot_quit(struct IRC *bot){
	bot_raw(bot,"QUIT :C-3PO Bot\r\n");
}
// bot_topic: For setting or removing a topic
void bot_topic(struct IRC *bot, const char *channel, const char *data){
	bot_raw(bot,"TOPIC %s :%s\r\n", channel, data);
}
// bot_action: For executing an action (.e.g /me is hungry)
void bot_action(struct IRC *bot, const char *channel, const char *data){
	bot_raw(bot,"PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
}
// bot_msg: For sending a channel message or a query
void bot_msg(struct IRC *bot, const char *channel, const char *data){
	bot_raw(bot,"PRIVMSG %s :%s\r\n", channel, data);
}
