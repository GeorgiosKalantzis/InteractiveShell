SHELL := /bin/bash


# ==================================================
# COMMANDS

CC = gcc
RM = rm -f

# ==================================================
# DIRECTORIES

SRC = src
BIN = bin

# ==================================================
# TARGETS

EXEC = myshell


# ==================================================
# COMPILATION

all: $(EXEC)

%: $(SRC)/%.c
	$(CC) $< -o $(BIN)/$@

clean:
	$(RM) $(SRC)/*~ *~

purge: clean
	$(RM) $(addprefix $(BIN)/, $(EXEC))
