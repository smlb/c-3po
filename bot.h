/*
 * =====================================================================================
 *
 *    Description:  
 *
 *        Version:  0.1.0
 *        Created:  08/26/2014
 *       Revision:  N/A
 *       Compiler:  gcc
 *
 *        Authors:  b4dtr1p, smlb, Th3Zer0
 *   Organization:  Freedom Fighters
 *
 * =====================================================================================
 */


#ifndef __IRCBOT_H
#define __IRCBOT_H

// Semantic Versioning
#define version_major 0
#define version_minor 1
#define version_patch 0

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

// bot IRC action
void bot_join(struct IRC *bot, const char *chan);
void bot_part(struct IRC *bot, const char *chan);
void bot_nick(struct IRC *bot, const char *data);
void bot_quit(struct IRC *bot);
void bot_away(struct IRC *bot);
void bot_topic(struct IRC *bot, const char *channel, const char *data);
void bot_action(struct IRC *bot, const char *channel, const char *data);
void bot_msg(struct IRC *bot, const char *channel, const char *data);
void bot_help(struct IRC *bot, char* cmd);

// built-in error reporting
void c_error(FILE *out, const char *fmt, ...);

#endif
