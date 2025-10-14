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

constexpr uint8_t OPCODE_PLAYER_STATE = 0x20;
// [CLIENT → SERVER, UDP]
// Sends the current player state.
// Payload: [ x (float) | y (float) | angle (float) | speed (float) ]

constexpr uint8_t OPCODE_WORLD_UPDATE = 0x21;
// [SERVER → CLIENT, UDP]
// Sends the updated world state (positions of other players, entities, etc).
// Payload: compressed list of visible entities.

constexpr uint8_t OPCODE_PLAYER_UPDATE = 0x23;
//  [SERVER → CLIENT, UDP]
//  Updates the state of a specific player (e.g., position, health, etc).
//  Payload: [ player_id (4 bytes) | x (float) | y (float) | angle (float) | speed (float) | health (int) ]
//  Purpose: notifies clients about changes to a particular player.

constexpr uint8_t OPCODE_ENEMIES_UPDATE = 0x24;
// [SERVER → CLIENT, UDP]
// Sends updates about enemy entities in the game.
// Payload: compressed list of enemy entities (positions, types, health, etc).
// Purpose: notifies clients about the current state of all visible enemies.

constexpr uint8_t OPCODE_PROJECTILES_UPDATE = 0x25;
// [SERVER → CLIENT, UDP]
// Sends updates about projectiles in the game.
// Payload: compressed list of projectile entities (positions, velocities, types, etc).
// Purpose: notifies clients about the current state of all visible projectiles.

constexpr uint8_t OPCODE_PROJECTILES_UPDATE_LZ4 = 0x2C;
// [SERVER → CLIENT, UDP]
// Payload: [ original_size (uint32 LE) | lz4_block ]

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

// === Updates de mouvement (UDP) ===
constexpr uint8_t OPCODE_MOVEMENT_UPDATE = 0x29;
// [SERVER → CLIENT, UDP]
// Updates des positions et velocities de toutes les entités
// Payload: liste compressée (EntityID, position, velocity)

constexpr uint8_t OPCODE_MOVEMENT_UPDATE_LZ4 = 0x2A;
// [SERVER → CLIENT, UDP]
// Payload: [ original_size (uint32 LE) | lz4_block ]

constexpr uint8_t OPCODE_HEALTH_UPDATE = 0x40;
// [SERVER → CLIENT, UDP]
// Updates la vie de toutes les entités
// Payload: liste compressée (EntityID, healtcomponent)

constexpr uint8_t OPCODE_HEALTH_UPDATE_LZ4 = 0x41;

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