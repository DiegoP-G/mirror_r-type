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


SERVER_UDP := server_exemple_UDP
SRC_SERVER_UDP := 	serverExempleUDP.cpp \


OBJ_SERVER_UDP := 	$(SRC_SERVER_UDP:.cpp=.o)


CLIENT_UDP := client_exemple_UDP
SRC_CLIENT_UDP := 	clientExempleUDP.cpp	\


OBJ_CLIENT_UDP := 	$(SRC_CLIENT_UDP:.cpp=.o)

CLIENT_UDP_TCP := client_exemple_UDP_TCP
SRC_CLIENT_UDP_TCP := clientUDPTCP.cpp	\

OBJ_CLIENT_UDP_TCP := 	$(SRC_CLIENT_UDP_TCP:.cpp=.o)


CXXFLAGS := -Wall -Wextra -std=c++17 -ItransferData -Iserver -Iclient -g3

all: $(SERVER) $(CLIENT) $(SERVER_UDP) $(CLIENT_UDP) $(CLIENT_UDP_TCP)

$(SERVER): $(OBJ_SERVER) $(OBJ_TRANSFER_DATA)
	g++ -o $(SERVER) $(OBJ_SERVER) $(OBJ_TRANSFER_DATA) $(SERVER_INCLUDE)

$(CLIENT): $(OBJ_CLIENT) $(OBJ_TRANSFER_DATA)
	g++ -o $(CLIENT) $(OBJ_CLIENT) $(OBJ_TRANSFER_DATA) $(GRAPHIC_LIB_FLAGS) \
	$(CLIENT_INCLUDE)

$(SERVER_UDP): $(OBJ_SERVER_UDP) $(OBJ_TRANSFER_DATA)
	g++ -o $(SERVER_UDP) $(OBJ_SERVER_UDP) $(OBJ_TRANSFER_DATA) $(SERVER_INCLUDE)

$(CLIENT_UDP): $(OBJ_CLIENT_UDP) $(OBJ_TRANSFER_DATA)
	g++ -o $(CLIENT_UDP) $(OBJ_CLIENT_UDP) $(OBJ_TRANSFER_DATA) $(CLIENT_INCLUDE)

$(CLIENT_UDP_TCP): $(OBJ_CLIENT_UDP_TCP) $(OBJ_TRANSFER_DATA)
	g++ -o $(CLIENT_UDP_TCP) $(OBJ_CLIENT_UDP_TCP) $(OBJ_TRANSFER_DATA) $(CLIENT_INCLUDE)


clean:
	rm -f $(OBJ_SERVER)
	rm -f $(OBJ_CLIENT)
	rm -f $(OBJ_TRANSFER_DATA)

fclean: clean
	rm -f $(SERVER)
	rm -f $(CLIENT)

re: fclean all

.PHONY: fclean clean all re