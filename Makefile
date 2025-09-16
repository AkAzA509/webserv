### COMPILATION OPTIONS ###
CXX       = c++
CXXFLAGS  = -Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address,leak  
CXXFLAGSDEV  = -Wall -Wextra -Werror -std=c++98 -fsanitize=address,leak -g3
LDFLAGS   = -Iincludes

### DIRECTORIES ###
SRC_DIR   = srcs
OBJ_DIR   = obj
BIN_DIR   = bin

### BINARIES ###
TARGET    = $(BIN_DIR)/webserv
TARGETDEV = $(BIN_DIR)/webserv_debug

### SOURCES & OBJECTS ###
SRCS      = $(SRC_DIR)/main.cpp \
            $(SRC_DIR)/parsing/Config.cpp \
            $(SRC_DIR)/parsing/Parser.cpp \
            $(SRC_DIR)/parsing/Server.cpp \
			$(SRC_DIR)/parsing/Logger.cpp \
			$(SRC_DIR)/exec/sig.cpp \
			$(SRC_DIR)/exec/listen.cpp \
			$(SRC_DIR)/exec/Request.cpp \
			$(SRC_DIR)/exec/Response.cpp \
			$(SRC_DIR)/exec/utils.cpp

OBJS      = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/prod/%.o, $(SRCS))
OBJSDEV   = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/dev/%.o, $(SRCS))

### COLORS ###
GREEN     = \033[0;32m
BLUE      = \033[0;34m
YELLOW    = \033[0;33m
PURPLE    = \033[0;35m
RED       = \033[0;31m
RESET     = \033[0m

### RULES ###

all: $(TARGET)

dev: $(TARGETDEV)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "$(PURPLE)→ Linking production binary...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	@echo "$(GREEN)✔ Production build done!$(RESET)"

$(TARGETDEV): $(OBJSDEV)
	@mkdir -p $(BIN_DIR)
	@echo "$(PURPLE)→ Linking development binary...$(RESET)"
	@$(CXX) $(CXXFLAGSDEV) $(LDFLAGS) -DDEBUG -o $@ $^
	@echo "$(BLUE)✔ Development build with DEBUG done!$(RESET)"

# Compile production objects
$(OBJ_DIR)/prod/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo -ne "\r🛠️ $(YELLOW) [PROD] Compiling $< → $@"
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

# Compile development objects
$(OBJ_DIR)/dev/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo -ne "\r🛠️  $(YELLOW) [DEV] Compiling $< → $@"
	@$(CXX) $(CXXFLAGSDEV) $(LDFLAGS) -DDEBUG -c $< -o $@

clean:
	@echo "$(RED)🧹 Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(RED)🔥 Full clean: removing binaries (except .dlq) and objects...$(RESET)"
	@find $(BIN_DIR) -type f ! -name '*.dlq' -delete

re: fclean all

redev: fclean dev

.PHONY: all dev clean fclean re redev
