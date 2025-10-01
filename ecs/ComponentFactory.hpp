#pragma once
#include "IComponent.hpp"
#include "using.hpp"
#include <functional>
#include <memory>

class ComponentFactory
{
  public:
    using Deserializer = std::function<std::unique_ptr<IComponent>(const uint8_t *, size_t)>;

    static std::unordered_map<ComponentID, Deserializer> &getRegistry()
    {
        static std::unordered_map<ComponentID, Deserializer> registry;
        return registry;
    }

    template <typename T> static void registerComponent()
    {
        ComponentID id = getComponentTypeID<T>();
        getRegistry()[id] = [](const uint8_t *data, size_t size) -> std::unique_ptr<IComponent> {
            T comp = T::deserialize(data, size);
            return std::make_unique<T>(comp);
        };
    }
};
