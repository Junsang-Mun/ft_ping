.PHONY: all clean fclean re debug

BLACK := $(shell tput -Txterm setaf 0)
RED := $(shell tput -Txterm setaf 1)
GREEN := $(shell tput -Txterm setaf 2)
YELLOW := $(shell tput -Txterm setaf 3)
LIGHTPURPLE := $(shell tput -Txterm setaf 4)
PURPLE := $(shell tput -Txterm setaf 5)
BLUE := $(shell tput -Txterm setaf 6)
WHITE := $(shell tput -Txterm setaf 7)
RESET := $(shell tput -Txterm sgr0)

NAME = ft_ping

RM = rm -f
MKDIR = mkdir -p

CC = cc
CFLAGS = -Wall -Wextra -Werror
# LDFLAGS = -L$(LIBFT_DIR) -lft -lreadline -lncurses
SRC_DIR = src/
INC_DIR = inc/
OBJ_DIR = obj/

SRC_FILES = ping.c \
			packet.c \
			icmp.c

INC_FILES = ping.h

SRCS = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJS = $(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(SRCS))

ifneq "$(findstring debug, $(MAKECMDGOALS))" ""
	CFLAGS += -g3
	SRC_FILES += debug.c
	SRC_FILES := $(filter-out icmp.c, $(SRC_FILES))
endif

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "$(GREEN)Executable file created!$(RESET)"

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@echo "$(PURPLE)Compiling:$(RESET) $<"
	@$(MKDIR) $(dir $@)
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@echo "$(YELLOW)Cleaning object files...$(RESET)"
	@$(RM) -r $(OBJ_DIR)
	@echo "$(GREEN)DONE!$(RESET)"

fclean: clean
	@echo "$(YELLOW)Deleting executable file...$(RESET)"
	@$(RM) $(NAME)
	@echo "$(GREEN)DONE!$(RESET)"

re: fclean all

debug: fclean all
