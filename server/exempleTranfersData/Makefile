##
## EPITECH PROJECT, 2024
## My_FTP
## File description:
## Makefile
##

SRC_TRANSFER_DATA :=	$(wildcard ../transferData/*.cpp)
OBJ_TRANSFER_DATA := 	$(SRC_TRANSFER_DATA:.cpp=.o)

SERVER := server_exemple
SRC_SERVER := 	serverExemple.cpp \


OBJ_SERVER := 	$(SRC_SERVER:.cpp=.o)


CLIENT := client_exemple
SRC_CLIENT := 	clientExemple.cpp	\


OBJ_CLIENT := 	$(SRC_CLIENT:.cpp=.o)


CXXFLAGS := -Wall -Wextra -std=c++17 -ItransferData -Iserver -Iclient -g3

all: $(SERVER) $(CLIENT)

$(SERVER): $(OBJ_SERVER) $(OBJ_TRANSFER_DATA)
	g++ -o $(SERVER) $(OBJ_SERVER) $(OBJ_TRANSFER_DATA) $(SERVER_INCLUDE)

$(CLIENT): $(OBJ_CLIENT) $(OBJ_TRANSFER_DATA)
	g++ -o $(CLIENT) $(OBJ_CLIENT) $(OBJ_TRANSFER_DATA) $(GRAPHIC_LIB_FLAGS) \
	$(CLIENT_INCLUDE)

clean:
	rm -f $(OBJ_SERVER)
	rm -f $(OBJ_CLIENT)
	rm -f $(OBJ_TRANSFER_DATA)

fclean: clean
	rm -f $(SERVER)
	rm -f $(CLIENT)

re: fclean all

.PHONY: fclean clean all re