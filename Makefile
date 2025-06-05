NAME        := webserv
CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++98 -Iincludes
RM          := rm -f

SRC_DIR     := src
OBJ_DIR     := obj
INC_DIR     := includes

SRC         := $(SRC_DIR)/main.cpp \
				$(SRC_DIR)/http/Request.cpp $(SRC_DIR)/http/Response.cpp \
				$(SRC_DIR)/config/ServerConfig.cpp $(SRC_DIR)/config/Route.cpp \
				$(SRC_DIR)/config/ConfigParser.cpp \
				$(SRC_DIR)/handler/RequestDispatcher.cpp \
				$(SRC_DIR)/handler/StaticFileHandler.cpp 


OBJ         := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

# Link the final executable
$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $^ -o $@

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM) -r $(OBJ_DIR)

fclean: clean
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
