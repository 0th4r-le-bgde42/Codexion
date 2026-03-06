# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/03/04 10:17:50 by ldauber           #+#    #+#              #
#    Updated: 2026/03/06 09:58:05 by ldauber          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# COLORS
DEF_COLOR = \033[0m
GREEN = \033[32m
YELLOW = \033[33m
MAGENTA = \033[35m
CYAN = \033[36m
BLUE = \033[34m
BOLD = \033[1m
ITALIC = \033[3m
REVERSE = \033[7m

# PARAMS
NAME = codexion
CC = cc
CFLAGS = -Wall -Werror -Wextra -pthread
RM = rm -rf
CLEAR = clear

# SOURCES
SRC = codexion.c heap.c routine.c dongles.c init.c utils.c
OBJ = $(SRC:.c=.o)

# RULES
all:
	@if [ ! -f $(NAME) ]; then \
		$(MAKE) header --no-print-directory; \
		$(MAKE) $(NAME) --no-print-directory; \
	else \
		printf "$(GREEN) $(NAME) is already up to date!$(DEF_COLOR)"; \
	fi

header:
	@printf "$(BOLD)$(CYAN)Starting compilation of $(NAME)...$(DEF_COLOR)\n\n"

$(NAME): $(OBJ)
	@printf "$(YELLOW)Linking objects...$(DEF_COLOR)\n"
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
	@sleep 0.3
	@printf "$(BOLD)$(GREEN)$(NAME)$(DEF_COLOR)$(GREEN) is ready!$(DEF_COLOR)\n"

%.o: %.c
	@printf "$(CYAN)Compiling: $(DEF_COLOR) %-20s $(YELLOW)[...]$(DEF_COLOR)\r" $<
	@$(CC) $(CFLAGS) -I . -c $< -o $@
	@sleep 0.3
	@printf "$(CYAN)Compiling: $(DEF_COLOR) %-20s $(GREEN)[DONE]$(DEF_COLOR)\n" $<

clean:
	@$(RM) $(OBJ)
	@printf "$(YELLOW)Cleaning objects... $(BOLD)$(GREEN)[DONE]$(DEF_COLOR)\n"

fclean: clean
	@$(RM) $(NAME)
	@printf "$(MAGENTA)Executable $(NAME) removed.$(DEF_COLOR)\n"

re: fclean all

run:
	@printf "$(BLUE)Numbers of codeurs (default 4): $(DEF_COLOR)"; read NBR; \
	printf "$(BLUE)Time to burnout(default 410): $(DEF_COLOR)"; read BURN; \
	printf "$(BLUE)Time to compile(default 200): $(DEF_COLOR)"; read COMP; \
	printf "$(BLUE)Time to debug(default 100): $(DEF_COLOR)"; read DEB; \
	printf "$(BLUE)Time to refactor(default 100): $(DEF_COLOR)"; read REF; \
	printf "$(BLUE)Required compilations(default 2): $(DEF_COLOR)"; read REQUCOMP; \
	printf "$(BLUE)Dongles cooldown(default 0): $(DEF_COLOR)"; read COOL; \
	printf "$(BLUE)Scheduler type (fifo/edf)(default fifo): $(DEF_COLOR)"; read SCHED; \
	ARGS="$${NBR:-4} $${BURN:-410} $${COMP:-200} $${DEB:-100} $${REF:-100} $${REQUCOMP:-2} $${COOL:-0} $${SCHED:-fifo}"; \
	printf "\n$(ITALIC)$(BOLD)$(REVERSE)./$(NAME) $$ARGS$(DEF_COLOR)\n"; \
	./$(NAME) $$ARGS

.PHONY: all clean fclean re header run