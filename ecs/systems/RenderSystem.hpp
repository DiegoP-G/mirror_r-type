#pragma once

#include "../GraphicsManager.hpp"
#include "../components/AnimatedSpriteComponent.hpp"
#include "../entityManager.hpp"

#include "../components/CenteredComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/Vector2D.hpp"
#include "getActualPosition.hpp"



class RenderSystem
{
  public:
    void update(EntityManager &entityManager);

  private:
    void renderAnimatedSprite(AnimatedSpriteComponent &animComp, float x, float y);
};