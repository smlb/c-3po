#ifndef __OPBOT_H
#define __OPBOT_H

#include "bot.h"

int parse_op(struct IRC *bot, char* msg);
int add_op(struct IRC *bot, char* nick);
int rm_op(struct IRC *bot, char* nick);
void print_op(struct IRC *bot);
int is_op(struct IRC *bot, char* nick);

#endif
