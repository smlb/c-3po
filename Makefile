########################################################################
# C-3PO - A simple IRC bot written in C
#
# All files in this archive are subject to the GNU General Public License.
#
# Version: 0.0.1
# Date: 26/08/2014 11:57:43
# Author: FreedomFighter (see contributor.txt)
#
# ----------------------------------------------------------------------
########################################################################

# Compiler used
CC = gcc
# How can i remove file?
RM = 'rm'
# Object file
OBJ = main.o bot.o curl.o op.o
BINOBJ = $(OBJ:%.o=bin/%.o)
# Header file
DEPS = bot.h
# Folder with object file
CFLAGS=-Wall -lm -lcurl -std=gnu99 -I. -I/usr/include/curl
OBJDIR = bin

# Compile Header and C-Source into Object file
## $@ is the parameter to the left of :
## $< is the first parameter to the right of :
%.o: %.c $(DEPS)
	-mkdir -p $(OBJDIR)
	$(CC) -c $< -o bin/$@ $(CFLAGS)

# Compile the executable
## $^ is the parameter to the right of :
c-3po: $(OBJ)
	$(CC) $(BINOBJ) -o bin/c-3po $(CFLAGS)

all: c-3po

# Delete compiled file
clean:
	-$(RM) bin/* 2>/dev/null
	
