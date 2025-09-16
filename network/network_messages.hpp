#pragma once
#include <cstdint>
#include <vector>
#include <cstring>

// Network message types
enum class MessageType : uint8_t {
    PLAYER_INPUT = 1,
    ENTITY_UPDATE = 2,
    ENTITY_CREATE = 3,
    ENTITY_DESTROY = 4,
    COLLISION_EVENT = 5,
    GAME_STATE = 6,
    PLAYER_JOIN = 7,
    PLAYER_LEAVE = 8
};

// Network packet header
struct PacketHeader {
    MessageType type;
    uint32_t entityID;
    float timestamp;
    uint16_t dataSize;
    
    PacketHeader() : type(MessageType::PLAYER_INPUT), entityID(0), timestamp(0.0f), dataSize(0) {}
    PacketHeader(MessageType t, uint32_t id, float time, uint16_t size) 
        : type(t), entityID(id), timestamp(time), dataSize(size) {}
};

// Network message structure
struct NetworkMessage {
    PacketHeader header;
    std::vector<uint8_t> data;
    
    NetworkMessage() = default;
    NetworkMessage(MessageType type, uint32_t entityID, float timestamp) {
        header.type = type;
        header.entityID = entityID;
        header.timestamp = timestamp;
        header.dataSize = 0;
    }
    
    void setData(const void* sourceData, size_t size) {
        data.resize(size);
        std::memcpy(data.data(), sourceData, size);
        header.dataSize = static_cast<uint16_t>(size);
    }
    
    template<typename T>
    void setData(const T& value) {
        setData(&value, sizeof(T));
    }
    
    template<typename T>
    T getData() const {
        T result;
        if (data.size() >= sizeof(T)) {
            std::memcpy(&result, data.data(), sizeof(T));
        }
        return result;
    }
    
    // Serialize message to byte array
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer;
        buffer.resize(sizeof(PacketHeader) + data.size());
        
        std::memcpy(buffer.data(), &header, sizeof(PacketHeader));
        if (!data.empty()) {
            std::memcpy(buffer.data() + sizeof(PacketHeader), data.data(), data.size());
        }
        
        return buffer;
    }
    
    // Deserialize from byte array
    static NetworkMessage deserialize(const uint8_t* buffer, size_t bufferSize) {
        NetworkMessage message;
        
        if (bufferSize >= sizeof(PacketHeader)) {
            std::memcpy(&message.header, buffer, sizeof(PacketHeader));
            
            if (message.header.dataSize > 0 && bufferSize >= sizeof(PacketHeader) + message.header.dataSize) {
                message.data.resize(message.header.dataSize);
                std::memcpy(message.data.data(), buffer + sizeof(PacketHeader), message.header.dataSize);
            }
        }
        
        return message;
    }
};

// Player input data structure
struct PlayerInputData {
    bool jump;
    float timestamp;
    
    PlayerInputData(bool j = false, float t = 0.0f) : jump(j), timestamp(t) {}
};

// Entity update data structure
struct EntityUpdateData {
    float x, y;
    float velocityX, velocityY;
    
    EntityUpdateData(float px = 0.0f, float py = 0.0f, float vx = 0.0f, float vy = 0.0f) 
        : x(px), y(py), velocityX(vx), velocityY(vy) {}
};

// Collision event data
struct CollisionEventData {
    uint32_t entityA;
    uint32_t entityB;
    
    CollisionEventData(uint32_t a = 0, uint32_t b = 0) : entityA(a), entityB(b) {}
};

