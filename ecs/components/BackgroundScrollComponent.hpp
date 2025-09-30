#pragma once
#include "../components.hpp"

class BackgroundScrollComponent : public Component
{
    public:
        float scrollSpeed = 0.0f;
        bool active = false;

        BackgroundScrollComponent() = default;
        BackgroundScrollComponent(float scrollSpeed, bool active);
};
