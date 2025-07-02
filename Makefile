CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
LDFLAGS = -Iincludes

SRC_DIR = srcs
OBJ_DIR = obj
BIN_DIR = bin

TARGET = $(BIN_DIR)/webserv

SRCS = $(SRC_DIR)/main.cpp \
		$(SRC_DIR)/parsing/Config.cpp \
		$(SRC_DIR)/parsing/Parser.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

dev:
	$(MAKE) LDFLAGS="$(LDFLAGS) -DDEBUG"

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(TARGET)

re: fclean all

redev: fclean dev

.PHONY: all dev clean fclean re
