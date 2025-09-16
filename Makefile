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
				$(SRC_DIR)/handler/RequestDispatcher.cpp $(SRC_DIR)/handler/StaticFileHandler.cpp \
				$(SRC_DIR)/handler/CgiHandler.cpp \
				$(SRC_DIR)/handler/UploadHandler.cpp \
				$(SRC_DIR)/handler/RedirectHandler.cpp \
				$(SRC_DIR)/handler/FormHandler.cpp \
				$(SRC_DIR)/server/Server.cpp $(SRC_DIR)/server/ServerManager.cpp \
				$(SRC_DIR)/utils/utils.cpp 
				

OBJ = $(OBJ_DIR)/main.o $(OBJ_DIR)/config/ConfigParser.o \
		$(OBJ_DIR)/config/Route.o $(OBJ_DIR)/config/ServerConfig.o \
		$(OBJ_DIR)/handler/RequestDispatcher.o $(OBJ_DIR)/http/Request.o \
		$(OBJ_DIR)/http/Response.o $(OBJ_DIR)/server/Server.o \
		$(OBJ_DIR)/server/ServerManager.o $(OBJ_DIR)/handler/CgiHandler.o \
		$(OBJ_DIR)/handler/RedirectHandler.o $(OBJ_DIR)/handler/FormHandler.o \
		$(OBJ_DIR)/handler/StaticFileHandler.o \
		$(OBJ_DIR)/handler/UploadHandler.o $(OBJ_DIR)/utils/utils.o

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

.PHONY: all clean fclean re test
