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

void Entity::deserializeComponent(ComponentID compId, const uint8_t *data, size_t dataSize)
{
    auto &registry = ComponentFactory::getRegistry();
    auto it = registry.find(compId);

    if (it == registry.end())
    {
        std::cerr << "⚠️ Composant inconnu, ID=" << (int)compId << std::endl;
        return;
    }

    // S'assurer que le vecteur est assez grand
    if (components.size() <= compId)
        components.resize(compId + 1);

    // Construire le composant via la lambda enregistrée
    auto comp = it->second(data, dataSize);

    // Attacher à l'entité
    comp->entity = this;

    // Stocker et activer le mask
    components[compId] = std::move(comp);
    componentMask.set(compId);

    // Initialiser
    components[compId]->init();
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

    // Écrire EntityID
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&id),
                reinterpret_cast<const uint8_t *>(&id) + sizeof(EntityID));

    // Écrire ComponentMask
    uint32_t maskValue = static_cast<uint32_t>(componentMask.to_ulong());
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&maskValue),
                reinterpret_cast<const uint8_t *>(&maskValue) + sizeof(uint32_t));

    // Compter les composants actifs
    uint8_t componentCount = 0;
    for (ComponentID i = 0; i < MAX_COMPONENTS; ++i)
    {
        if (componentMask[i] && i < components.size() && components[i])
        {
            componentCount++;
        }
    }
    data.push_back(componentCount);

    // Sérialiser chaque composant
    for (ComponentID i = 0; i < MAX_COMPONENTS; ++i)
    {
        if (componentMask[i] && i < components.size() && components[i])
        {
            data.push_back(i); // ComponentID

            // Obtenir les données sérialisées du composant
            auto componentData = serializeComponent(i);

            // Écrire la taille des données du composant
            uint16_t dataSize = static_cast<uint16_t>(componentData.size());
            data.insert(data.end(), reinterpret_cast<const uint8_t *>(&dataSize),
                        reinterpret_cast<const uint8_t *>(&dataSize) + sizeof(uint16_t));

            // Écrire les données du composant
            data.insert(data.end(), componentData.begin(), componentData.end());
        }
    }

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

    if (offset + sizeof(uint32_t) > maxSize)
        return offset;

    uint32_t maskValue;
    std::memcpy(&maskValue, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (offset + sizeof(uint8_t) > maxSize)
        return offset;

    uint8_t componentCount = data[offset++];

    for (uint8_t i = 0; i < componentCount; ++i)
    {
        if (offset + sizeof(ComponentID) > maxSize)
            break;
        ComponentID compId = data[offset++];

        if (offset + sizeof(uint16_t) > maxSize)
            break;
        uint16_t dataSize;
        std::memcpy(&dataSize, data + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);

        if (offset + dataSize > maxSize)
            break;

        // Désérialiser via la fabrique
        deserializeComponent(compId, data + offset, dataSize);
        offset += dataSize;
    }

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