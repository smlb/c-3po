#ifndef __IRC_H
#define __IRC_H

// Simple struct with bot configuration ;)
struct IRC{
	char server[100];
	char port[5];
	char nick[15];
	char chan[20];
	// Connection
	struct addrinfo hints, *res;
	int conn;
	char sbuf[512];
}; 

// All bot-related function should start with 'bot_'
void bot_init(struct IRC *bot, char *s, char *p, char *n, char *c);
int bot_connect(struct IRC *bot);
void bot_pong(struct IRC *bot, char *buff);
void bot_raw(struct IRC *bot, char *fmt, ...);
int bot_parse_action(struct IRC *bot, char *user, char *command, char *where, char *target, char *message);

void c_error(FILE *out, const char *fmt, ...);

#endif
