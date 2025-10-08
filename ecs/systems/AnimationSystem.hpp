#pragma once

#include "../entityManager.hpp"


class AnimationSystem {
public:
    void update(EntityManager &entityManager, float deltaTime);

    void handleAnimation(Entity *&entity, InputComponent &input, float deltaTime);

};