#include "entity.hpp"
#include "allComponentsInclude.hpp"
#include <unordered_map>

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
    (void)dataSize;

    // S'assurer que le vecteur components est assez grand
    if (components.size() <= compId)
    {
        components.resize(compId + 1);
    }

    // Désérialiser et ajouter/mettre à jour le composant
    if (compId == getComponentTypeID<TransformComponent>())
    {
        auto comp = TransformComponent::deserialize(data);
        // Vérifier directement le vecteur au lieu de hasComponent
        if (components[compId] != nullptr)
        {
            *static_cast<TransformComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new TransformComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<PlayerComponent>())
    {
        auto comp = PlayerComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<PlayerComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new PlayerComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<VelocityComponent>())
    {
        auto comp = VelocityComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<VelocityComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new VelocityComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<LaserWarningComponent>())
    {
        auto comp = LaserWarningComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<LaserWarningComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new LaserWarningComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<CenteredComponent>())
    {
        auto comp = CenteredComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<CenteredComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new CenteredComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<SpriteComponent>())
    {
        auto comp = SpriteComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<SpriteComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new SpriteComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<ColliderComponent>())
    {
        auto comp = ColliderComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<ColliderComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new ColliderComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<HealthComponent>())
    {
        auto comp = HealthComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<HealthComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new HealthComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<InputComponent>())
    {
        auto comp = InputComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<InputComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new InputComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<ProjectileComponent>())
    {
        auto comp = ProjectileComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<ProjectileComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new ProjectileComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<EnemyComponent>())
    {
        auto comp = EnemyComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<EnemyComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new EnemyComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<BirdComponent>())
    {
        auto comp = BirdComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<BirdComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new BirdComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<PipeComponent>())
    {
        auto comp = PipeComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<PipeComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new PipeComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<GravityComponent>())
    {
        auto comp = GravityComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<GravityComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new GravityComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<JumpComponent>())
    {
        auto comp = JumpComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<JumpComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new JumpComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<GameStateComponent>())
    {
        auto comp = GameStateComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<GameStateComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new GameStateComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<AnimatedSpriteComponent>())
    {
        auto comp = AnimatedSpriteComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<AnimatedSpriteComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new AnimatedSpriteComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else if (compId == getComponentTypeID<BackgroundScrollComponent>())
    {
        auto comp = BackgroundScrollComponent::deserialize(data);
        if (components[compId] != nullptr)
        {
            *static_cast<BackgroundScrollComponent*>(components[compId].get()) = comp;
        }
        else
        {
            components[compId].reset(new BackgroundScrollComponent(comp));
            components[compId]->entity = this;
            componentMask.set(compId);
            components[compId]->init();
        }
    }
    else
    {
        std::unordered_map<ComponentID, std::string> componentNames;
        componentNames[getComponentTypeID<TransformComponent>()] = "TransformComponent";
        componentNames[getComponentTypeID<PlayerComponent>()] = "PlayerComponent";
        componentNames[getComponentTypeID<VelocityComponent>()] = "VelocityComponent";
        componentNames[getComponentTypeID<SpriteComponent>()] = "SpriteComponent";
        componentNames[getComponentTypeID<ColliderComponent>()] = "ColliderComponent";
        componentNames[getComponentTypeID<HealthComponent>()] = "HealthComponent";
        componentNames[getComponentTypeID<InputComponent>()] = "InputComponent";
        componentNames[getComponentTypeID<ProjectileComponent>()] = "ProjectileComponent";
        componentNames[getComponentTypeID<EnemyComponent>()] = "EnemyComponent";
        componentNames[getComponentTypeID<BirdComponent>()] = "BirdComponent";
        componentNames[getComponentTypeID<PipeComponent>()] = "PipeComponent";
        componentNames[getComponentTypeID<GravityComponent>()] = "GravityComponent";
        componentNames[getComponentTypeID<JumpComponent>()] = "JumpComponent";
        componentNames[getComponentTypeID<GameStateComponent>()] = "GameStateComponent";
        componentNames[getComponentTypeID<AnimatedSpriteComponent>()] = "AnimatedSpriteComponent";
        componentNames[getComponentTypeID<LaserWarningComponent>()] = "LaserWarningComponent";
        componentNames[getComponentTypeID<CenteredComponent>()] = "CenteredComponent";
        componentNames[getComponentTypeID<BackgroundScrollComponent>()] = "BackgroundScrollComponent";
        std::cerr << "Warning: Unknown component ID: " << (int)compId << std::endl;
        std::cerr << "Component is :" << componentNames[compId] << std::endl;
    }
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

    // Lire EntityID (mais ne pas le changer - il est défini par EntityManager)
    EntityID serializedId;
    std::memcpy(&serializedId, data + offset, sizeof(EntityID));
    offset += sizeof(EntityID);

    if (offset + sizeof(uint32_t) > maxSize)
        return offset;

    // Lire ComponentMask
    uint32_t maskValue;
    std::memcpy(&maskValue, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    componentMask = ComponentMask(maskValue);

    if (offset + sizeof(uint8_t) > maxSize)
        return offset;

    // Lire le nombre de composants
    uint8_t componentCount = data[offset++];

    // Désérialiser chaque composant
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

        // Désérialiser le composant basé sur l'ID
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