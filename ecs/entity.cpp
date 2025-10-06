#include "entity.hpp"
#include "ComponentFactory.hpp"

std::vector<uint8_t> Entity::serializeComponent(ComponentID compId) const
{
    if (compId < components.size() && components[compId])
    {
        return components[compId]->serialize();
    }
    return {};
}

bool Entity::deserializeComponent(ComponentID compId, const uint8_t *data, size_t dataSize)
{
    auto &registry = ComponentFactory::getRegistry();
    auto it = registry.find(compId);

    if (it == registry.end())
    {
        std::cerr << "⚠️ Composant inconnu, ID=" << (int)compId << std::endl;
        return false;
    }

    if (components.size() <= compId)
        components.resize(compId + 1);

    auto comp = it->second(data, dataSize);

    comp->entity = this;

    components[compId] = std::move(comp);
    componentMask.set(compId);

    components[compId]->init();
    return true;
}

Entity::Entity(EntityManager &manager, EntityID id) : manager(manager), id(id)
{
}

void Entity::update(float deltaTime)
{
    for (auto &c : components)
        if (c)
            c->update(deltaTime);
}

std::vector<uint8_t> Entity::serialize() const
{
    std::vector<uint8_t> data;

    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&id),
                reinterpret_cast<const uint8_t *>(&id) + sizeof(EntityID));

    uint32_t maskValue = componentMask.to_ulong();
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&maskValue),
                reinterpret_cast<const uint8_t *>(&maskValue) + sizeof(uint32_t));

    uint8_t componentCount = 0;
    for (ComponentID i = 0; i < MAX_COMPONENTS; ++i)
    {
        if (componentMask[i] && i < components.size() && components[i])
            componentCount++;
    }
    data.push_back(componentCount);

    // std::cout << "[Entity::serialize] Entity ID: " << id << std::endl;
    // std::cout << "[Entity::serialize] Component Mask: " << std::bitset<32>(maskValue) << std::endl;
    // std::cout << "[Entity::serialize] Component Count: " << (int)componentCount << std::endl;

    for (ComponentID i = 0; i < MAX_COMPONENTS; ++i)
    {
        if (componentMask[i] && i < components.size() && components[i])
        {
            data.push_back(static_cast<uint8_t>(i));

            std::vector<uint8_t> componentData = components[i]->serialize();

            // std::cout << "[Entity::serialize]   Component ID " << (int)i
            //           << " -> " << componentData.size() << " bytes" << std::endl;

            uint16_t dataSize = static_cast<uint16_t>(componentData.size());
            data.insert(data.end(), reinterpret_cast<const uint8_t *>(&dataSize),
                        reinterpret_cast<const uint8_t *>(&dataSize) + sizeof(uint16_t));

            data.insert(data.end(), componentData.begin(), componentData.end());
        }
    }

    // std::cout << "[Entity::serialize] Total serialized size: " << data.size() << " bytes\n" << std::endl;

    return data;
}

size_t Entity::deserialize(const uint8_t *data, size_t maxSize)
{
    size_t offset = 0;

    if (offset + sizeof(EntityID) > maxSize)
        return offset;

    EntityID serializedId;
    std::memcpy(&serializedId, data + offset, sizeof(EntityID));
    offset += sizeof(EntityID);

    this->id = serializedId;
    std::cout << "[Entity::deserialize] Entity ID: " << id << std::endl;

    if (offset + sizeof(uint32_t) > maxSize)
        return offset;

    uint32_t maskValue;
    std::memcpy(&maskValue, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    //  std::cout << "[Entity::deserialize] Component mask: " << std::bitset<32>(maskValue) << std::endl;

    if (offset + sizeof(uint8_t) > maxSize)
        return offset;

    uint8_t componentCount = data[offset++];
    //  std::cout << "[Entity::deserialize] Component count: " << (int)componentCount << std::endl;

    for (uint8_t i = 0; i < componentCount; ++i)
    {
        if (offset + sizeof(ComponentID) > maxSize)
        {
            std::cout << "[Entity::deserialize] ERROR: Not enough data for component ID" << std::endl;
            break;
        }
        ComponentID compId = data[offset++];
        //       std::cout << "[Entity::deserialize] Reading component ID: " << (int)compId << std::endl;

        if (offset + sizeof(uint16_t) > maxSize)
        {
            std::cout << "[Entity::deserialize] ERROR: Not enough data for component size" << std::endl;
            break;
        }
        uint16_t dataSize;
        std::memcpy(&dataSize, data + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        //   std::cout << "[Entity::deserialize]   Component data size: " << dataSize << " bytes" << std::endl;

        if (offset + dataSize > maxSize)
        {
            std::cout << "[Entity::deserialize] ERROR: Not enough data for component payload (need " << dataSize
                      << ", have " << (maxSize - offset) << ")" << std::endl;
            break;
        }
        bool success = deserializeComponent(compId, data + offset, dataSize);
        std::cout << "[Entity::deserialize]   Deserialization " << (success ? "SUCCESS" : "FAILED") << std::endl;
        offset += dataSize;
    }

    //   std::cout << "[Entity::deserialize] Total bytes read: " << offset << " / " << maxSize << "\n" << std::endl;
    return offset;
}

bool Entity::isActive() const
{
    return active;
}

void Entity::render()
{
    for (auto &c : components)
        if (c)
            c->render();
}

void Entity::destroy()
{
    active = false;
}

EntityID Entity::getID() const
{
    return id;
}

ComponentMask Entity::getComponentMask() const
{
    return componentMask;
}