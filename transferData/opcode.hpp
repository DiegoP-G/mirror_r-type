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

#define OPCODE_CODE_UDP 0x00
// [SERVER → CLIENT, TCP]
// Sent by the server to transmit the unique "UDP code" that allows the client
// to authenticate itself on the UDP socket.
// Payload: [ code_udp (4 bytes) ]
// Purpose: links the TCP connection with the UDP endpoint.

#define OPCODE_CLOSE_CONNECTION 0x01
// [BIDIRECTIONAL]
// Tells the receiver to gracefully close the connection.
// Payload: none
// Purpose: clean disconnect, either client-initiated or server-initiated.

#define OPCODE_INCOMPLETE_DATA 0x02
// [SERVER → CLIENT, TCP]
// Informs the client that the last packet was incomplete or invalid.
// Payload: [ missing_length (2 bytes) ]
// Purpose: client should resend the missing data.

// ---- Authentication / Identification ----

#define OPCODE_UDP_AUTH 0x10
// [CLIENT → SERVER, UDP]
// First UDP message sent by the client to authenticate its datagrams.
// Payload: [ code_udp (4 bytes) ]
// Purpose: lets the server associate the client's UDP (IP:port) with its TCP identity.

// ---- Game data ----

#define OPCODE_PLAYER_STATE 0x20
// [CLIENT → SERVER, UDP]
// Sends the current player state.
// Payload: [ x (float) | y (float) | angle (float) | speed (float) ]

#define OPCODE_WORLD_UPDATE 0x21
// [SERVER → CLIENT, UDP]
// Sends the updated world state (positions of other players, entities, etc).
// Payload: compressed list of visible entities.

// ---- Chat / Messaging ----

#define OPCODE_CHAT_MESSAGE 0x30
// [CLIENT → SERVER, TCP]
// Sends a text message (global or private).
// Payload: [ target (1 byte: 0 = global, 1 = private) | msg (string) ]

#define OPCODE_CHAT_BROADCAST 0x31
// [SERVER → CLIENT, TCP]
// Broadcast chat message to all players.
// Payload: [ sender_id (4 bytes) | msg (string) ]
