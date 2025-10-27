#pragma once
#include <cstdint>
// ===========================================================
//                   OPCODE DEFINITIONS
// ===========================================================
//
// Every message exchanged between client and server has the format:
//   [ OPCODE (1 byte) ] [ DATA ... ]
//
// - The OPCODE indicates the type of operation.
// - The DATA depends on the opcode.
//
// ===========================================================

// ---- Connection management ----

constexpr uint8_t OPCODE_CODE_UDP = 0x00;
// [SERVER → CLIENT, TCP]
// Sent by the server to transmit the unique "UDP code" that allows the client
// to authenticate itself on the UDP socket.
// Payload: [ code_udp (4 bytes) ]
// Purpose: links the TCP connection with the UDP endpoint.

constexpr uint8_t OPCODE_CLOSE_CONNECTION = 0x01;
// [BIDIRECTIONAL]
// Tells the receiver to gracefully close the connection.
// Payload: none
// Purpose: clean disconnect, either client-initiated or server-initiated.

constexpr uint8_t OPCODE_INCOMPLETE_DATA = 0x02;
// [SERVER → CLIENT, TCP]
// Informs the client that the last packet was incomplete or invalid.
// Payload: [ missing_length (2 bytes) ]
// Purpose: client should resend the missing data.

// ---- Authentication / Identification ----

constexpr uint8_t OPCODE_UDP_AUTH = 0x10;
// [CLIENT → SERVER, UDP]
// First UDP message sent by the client to authenticate its datagrams.
// Payload: [ code_udp (4 bytes) ]
// Purpose: lets the server associate the client's UDP (IP:port) with its TCP identity.

constexpr uint8_t OPCODE_PLAYER_ID = 0x11;
// [SERVER → SERVER, TCP]
// Give playerId to client
// Payload: [ code_udp (4 bytes) ]
// Purpose: lets the client stock his id.

// ---- Game data ----

constexpr uint8_t OPCODE_UPDATE_ENTITIES = 0x21;
// [SERVER → CLIENT, UDP]
// Sends the updated world state (positions of other players, entities, etc).
// Payload: compressed list of visible entities.

constexpr uint8_t OPCODE_PLAYER_INPUT = 0x26;
// [CLIENT → SERVER, UDP]
// Sends the client kb input.

constexpr uint8_t OPCODE_ENTITY_CREATE = 0x27;
// [SERVER → CLIENT, TCP]
// Création complète d'une entité avec tous ses composants
// Payload: entité sérialisée complète

constexpr uint8_t OPCODE_ENTITY_DESTROY = 0x28;
// [SERVER → CLIENT, TCP]
// Destruction d'une entité
// Payload: EntityID (4 bytes)

constexpr uint8_t OPCODE_CHAT_MESSAGE = 0x30;
// [CLIENT → SERVER, TCP]
// Sends a text message (global or private).
// Payload: [ target (1 byte: 0 = global, 1 = private) | msg (string) ]

constexpr uint8_t OPCODE_CHAT_BROADCAST = 0x31;
// [SERVER → CLIENT, TCP]
// Broadcast chat message to all players.
// Payload: [ sender_id (4 bytes) | msg (string) ]

// ---- Lobby / Pre-game info ----

constexpr uint8_t OPCODE_GAME_OVER = 0x32;
// [SERVER → CLIENT, TCP]
// Notify clients that the game is over.
// Payload: [ winner_id (4 bytes) ]
// Purpose: clients can display end-of-game screen with winner info.

constexpr uint8_t OPCODE_PLAYER_DEAD = 0x33;
// [SERVER → CLIENT, TCP]
// Notify clients that a specific player has died.
// Payload: [ player_id (4 bytes) ]
// Purpose: clients can update UI, respawn logic, etc.

constexpr uint8_t OPCODE_UPDATE_WAVE = 0x50;
// [SERVER → CLIENT, TCP]
// Update current wave counter
// Payload: WaveNumber (4 bytes)

constexpr uint8_t OPCODE_UPDATE_SCORE = 0x51;
// [SERVER → CLIENT, TCP]
// Update clients score
// Payload:  compressed list of all clients' score. (playerId: 4 byte, score 4 byte)

constexpr uint8_t OPCODE_LOBBY_INFO = 0x60;
// [SERVER → CLIENT, TCP]
// Sends information about the lobby, e.g., number of players ready.
// Payload: [ players_ready (1 byte) | total_players (1 byte) ]
// Purpose: client can display lobby info, like "players ready: X / Y".

constexpr uint8_t OPCODE_GAME_STATE_UPDATE = 0x61;
// [SERVER → CLIENT, TCP]
// Sends info about game state (INGAME or INLOBBY)

constexpr uint8_t OPCODE_KICK_NOTIFICATION = 0x62;
// [SERVER → CLIENT, TCP]
// Notifies the client that it has been kicked from the server.
// Payload: none

constexpr uint8_t OPCODE_BAN_NOTIFICATION = 0x63;
// [SERVER → CLIENT, TCP]
// Notifies the client's ip that it has been ban from the server.
// Payload: none

constexpr uint8_t OPCODE_CREATE_LOBBY = 0x70;
// [CLIENT → SERVER, TCP]
// Request to create a new lobby
// Payload: lobby_name (string)

constexpr uint8_t OPCODE_JOIN_LOBBY = 0x71;
// [CLIENT → SERVER, TCP]
// Request to join an existing lobby
// Payload: lobby_name (string)

constexpr uint8_t OPCODE_LOBBY_CREATED = 0x72;
// [SERVER → CLIENT, TCP]
// Confirmation that lobby was created
// Payload: lobby_name (string)

constexpr uint8_t OPCODE_LOBBY_JOINED = 0x73;
// [SERVER → CLIENT, TCP]
// Confirmation that player joined lobby
// Payload: lobby_name (string)

constexpr uint8_t OPCODE_LOBBY_ERROR = 0x74;
// [SERVER → CLIENT, TCP]
// Error joining/creating lobby
// Payload: error_message (string)

constexpr uint8_t OPCODE_LOGIN_REQUEST = 0x80;
// [CLIENT → SERVER, TCP]
// Send login credentials to server
// Payload: [ username (string) | password (string) ]

constexpr uint8_t OPCODE_LOGIN_RESPONSE = 0x81;
// [SERVER → CLIENT, TCP]
// Login response from server
// Payload: [ success (1 byte: 0 = failure, 1 = success) | message (string) ]

constexpr uint8_t OPCODE_SIGNIN_REQUEST = 0x82;
// [CLIENT → SERVER, TCP]
// Sends sign-in credentials to the server.
// Payload: [ username (string) | password (string) ]

constexpr uint8_t OPCODE_SIGNIN_RESPONSE = 0x83;
// [SERVER → CLIENT, TCP]
// Signin response from server
// Payload: [ success (1 byte: 0 = failure, 1 = success) | message (string) ]

constexpr uint8_t OPCODE_SERVER_PUB_KEY = 0x90;
// [SERVER → CLIENT, TCP]
// AES server key
// Payload: [ success (1 byte: 0 = failure, 1 = success) | message (string) ]

constexpr uint8_t OPCODE_CLIENT_IV_KEY = 0x92;
// [SERVER → CLIENT, TCP]
// Client sends generated AES key and key to server
// Payload: [ AES_KEY (256 bytes) || IV (16 bytes) ]