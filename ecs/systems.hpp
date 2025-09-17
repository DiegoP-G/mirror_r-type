#pragma once
#include "../game/graphics.hpp"
#include "components.hpp"
#include "ecs.hpp"
#include <cmath>

// System for movement
class MovementSystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities =
        entityManager
            .getEntitiesWithComponents<TransformComponent, VelocityComponent>();

    for (auto &entity : entities) {
      auto &transform = entity->getComponent<TransformComponent>();
      auto &velocity = entity->getComponent<VelocityComponent>();

      transform.position += velocity.velocity * deltaTime;
    }
  }
};

// System for rendering
class RenderSystem {
public:
  void update(EntityManager &entityManager) {
    if (!g_graphics)
      return;

    auto entities =
        entityManager
            .getEntitiesWithComponents<TransformComponent, SpriteComponent>();

    for (auto &entity : entities) {
      auto &transform = entity->getComponent<TransformComponent>();
      auto &sprite = entity->getComponent<SpriteComponent>();

      if (sprite.isVisible) {
        if (sprite.texture) {
          g_graphics->drawTexture(sprite.texture,
                                  static_cast<int>(transform.position.x),
                                  static_cast<int>(transform.position.y),
                                  sprite.width, sprite.height);
        } else {
          // Draw colored rectangle if no texture
          g_graphics->drawRect(static_cast<int>(transform.position.x),
                               static_cast<int>(transform.position.y),
                               sprite.width, sprite.height, sprite.r, sprite.g,
                               sprite.b, sprite.a);
        }
      }
    }
  }
};

// System for collision detection
class CollisionSystem {
public:
  void update(EntityManager &entityManager) {
    auto entities =
        entityManager
            .getEntitiesWithComponents<TransformComponent, ColliderComponent>();

    // Check for collisions between all relevant entities
    for (size_t i = 0; i < entities.size(); i++) {
      if (!entities[i]->isActive())
        continue; // Skip inactive entities

      auto &transform1 = entities[i]->getComponent<TransformComponent>();
      auto &collider1 = entities[i]->getComponent<ColliderComponent>();

      // Update collider position based on transform
      collider1.hitbox.x = transform1.position.x;
      collider1.hitbox.y = transform1.position.y;

      for (size_t j = i + 1; j < entities.size(); j++) {
        if (!entities[j]->isActive())
          continue; // Skip inactive entities

        auto &transform2 = entities[j]->getComponent<TransformComponent>();
        auto &collider2 = entities[j]->getComponent<ColliderComponent>();

        // Update collider position based on transform
        collider2.hitbox.x = transform2.position.x;
        collider2.hitbox.y = transform2.position.y;

        // Check collision
        if (collider1.hitbox.intersects(collider2.hitbox)) {
          // Handle collision (could emit events, modify health, etc.)
          handleCollision(entities[i], entities[j]);
        }
      }
    }
  }

private:
  void handleCollision(Entity *a, Entity *b) {
    // For Flappy Bird: Check if bird hits pipe
    bool aIsJumper = a->hasComponent<JumpComponent>();
    bool bIsJumper = b->hasComponent<JumpComponent>();

    // Bird (has JumpComponent) hits anything else = game over
    if (aIsJumper || bIsJumper) {
      Entity *jumper = aIsJumper ? a : b;
      jumper->destroy(); // Destroy the jumping entity (bird)
      return;
    }

    // Original R-Type collision logic for backwards compatibility
    bool aIsPlayer =
        !a->hasComponent<EnemyComponent>() && a->hasComponent<InputComponent>();
    bool bIsPlayer =
        !b->hasComponent<EnemyComponent>() && b->hasComponent<InputComponent>();
    bool aIsEnemy = a->hasComponent<EnemyComponent>();
    bool bIsEnemy = b->hasComponent<EnemyComponent>();
    bool aIsProjectile = a->hasComponent<ProjectileComponent>();
    bool bIsProjectile = b->hasComponent<ProjectileComponent>();

    // Player hit by enemy
    if ((aIsPlayer && bIsEnemy) || (bIsPlayer && aIsEnemy)) {
      Entity *player = aIsPlayer ? a : b;
      if (player->hasComponent<HealthComponent>()) {
        auto &health = player->getComponent<HealthComponent>();
        health.health -= 10.0f; // Example damage
        if (health.health <= 0) {
          player->destroy();
        }
      }
    }

    // Projectile hit something
    if (aIsProjectile || bIsProjectile) {
      Entity *projectile = aIsProjectile ? a : b;
      Entity *target = aIsProjectile ? b : a;

      auto &projComp = projectile->getComponent<ProjectileComponent>();

      // Don't hit your owner
      if (target->getID() == projComp.owner) {
        return;
      }

      // Damage target if it has health
      if (target->hasComponent<HealthComponent>()) {
        auto &health = target->getComponent<HealthComponent>();
        health.health -= projComp.damage;
        if (health.health <= 0) {
          target->destroy();
        }
      }

      // Destroy projectile on hit
      projectile->destroy();
    }
  }
};

// System for handling input
class InputSystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities =
        entityManager
            .getEntitiesWithComponents<InputComponent, VelocityComponent>();
    for (auto &entity : entities) {
      auto &input = entity->getComponent<InputComponent>();
      auto &velocity = entity->getComponent<VelocityComponent>();

      // Reset velocity
      velocity.velocity.x = 0.0f;
      velocity.velocity.y = 0.0f;

      // Update velocity based on input
      const float PLAYER_SPEED = 200.0f; // Adjust speed as needed
      if (input.up) {
        velocity.velocity.y = -PLAYER_SPEED;
      }
      if (input.down) {
        velocity.velocity.y = PLAYER_SPEED;
      }
      if (input.left) {
        velocity.velocity.x = -PLAYER_SPEED;
      }
      if (input.right) {
        velocity.velocity.x = PLAYER_SPEED;
      }

      // Fire a bullet when space is pressed
      if (input.fire) {
        fire(entityManager, entity);
        input.fire = false; // Reset fire input
      }
    }
  }

private:
  void fire(EntityManager &entityManager, Entity *player) {
    auto &transform = player->getComponent<TransformComponent>();

    // Create bullet entity
    auto &bullet = entityManager.createEntity();

    // Position the bullet at the player's position
    bullet.addComponent<TransformComponent>(
        transform.position.x +
            32.0f, // Offset to fire from the front of the player
        transform.position.y + 16.0f // Center height
    );

    // Add bullet components
    bullet.addComponent<VelocityComponent>(300.0f,
                                           0.0f); // Fast horizontal movement
    bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0); // Red bullet
    bullet.addComponent<ColliderComponent>(8.0f, 8.0f);    // Small hitbox
    bullet.addComponent<ProjectileComponent>(
        10.0f, 2.0f, player->getID()); // Damage, lifetime, owner
  }
};

// System for projectile lifetime management
class ProjectileSystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities =
        entityManager.getEntitiesWithComponents<ProjectileComponent>();

    for (auto &entity : entities) {
      auto &projectile = entity->getComponent<ProjectileComponent>();

      // Update lifetime
      projectile.remainingLife -= deltaTime;
      if (projectile.remainingLife <= 0) {
        entity->destroy();
      }
    }
  }
};

// System for enemy behavior
class EnemySystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities =
        entityManager
            .getEntitiesWithComponents<EnemyComponent, TransformComponent>();

    for (auto &entity : entities) {
      auto &enemy = entity->getComponent<EnemyComponent>();
      auto &transform = entity->getComponent<TransformComponent>();

      // Update enemy behavior based on type
      switch (enemy.type) {
      case 0: // Basic enemy - moves left
        if (entity->hasComponent<VelocityComponent>()) {
          entity->getComponent<VelocityComponent>().velocity =
              Vector2D(-50.0f, 0);
        }
        break;

      case 1: // Sine wave movement
        if (entity->hasComponent<VelocityComponent>()) {
          static float time = 0;
          time += deltaTime;
          entity->getComponent<VelocityComponent>().velocity =
              Vector2D(-50.0f, sinf(time * 2.0f) * 40.0f);
        }
        break;

      case 2: // Attack pattern
        enemy.currentCooldown -= deltaTime;
        if (enemy.currentCooldown <= 0) {
          enemyFire(entityManager, entity);
          enemy.currentCooldown = enemy.attackCooldown;
        }
        break;
      }

      // Destroy enemies that go off screen
      if (transform.position.x < -50.0f) {
        entity->destroy();
      }
    }
  }

private:
  void enemyFire(EntityManager &entityManager, Entity *enemy) {
    auto &transform = enemy->getComponent<TransformComponent>();

    // Create projectile entity
    auto &projectile = entityManager.createEntity();

    // Position the projectile at the enemy's position
    projectile.addComponent<TransformComponent>(
        transform.position.x -
            20.0f, // Offset to fire from the front of the enemy
        transform.position.y + 10.0f // Center height
    );

    // Add projectile components
    projectile.addComponent<VelocityComponent>(
        -200.0f, 0.0f);                         // Fast horizontal movement
    projectile.addComponent<SpriteComponent>(); // Add sprite (different from
                                                // player projectiles)
    projectile.addComponent<ColliderComponent>(10.0f, 5.0f); // Small hitbox
    projectile.addComponent<ProjectileComponent>(
        5.0f, 3.0f, enemy->getID()); // Damage, lifetime, owner
  }
};

// Flappy Bird specific systems - SHOULD BE GENERIC
class GravitySystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities =
        entityManager
            .getEntitiesWithComponents<VelocityComponent, GravityComponent>();

    for (auto &entity : entities) {
      auto &velocity = entity->getComponent<VelocityComponent>();
      auto &gravity = entity->getComponent<GravityComponent>();

      // Apply gravity
      velocity.velocity.y += gravity.gravity * deltaTime;

      // Cap velocity at terminal velocity
      if (velocity.velocity.y > gravity.terminalVelocity) {
        velocity.velocity.y = gravity.terminalVelocity;
      }
    }
  }
};

// GENERIC Jump System - replaces BirdSystem
class JumpSystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities = entityManager.getEntitiesWithComponents<
        VelocityComponent, InputComponent, JumpComponent>();

    for (auto &entity : entities) {
      auto &velocity = entity->getComponent<VelocityComponent>();
      auto &input = entity->getComponent<InputComponent>();
      auto &jump = entity->getComponent<JumpComponent>();

      // Jump when fire is pressed and can jump
      if (input.fire && jump.canJump) {
        velocity.velocity.y = jump.jumpStrength;
        input.fire = false; // Reset input
      }
    }
  }
};

// GENERIC Boundary System - replaces bird boundary check
class BoundarySystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities =
        entityManager.getEntitiesWithComponents<TransformComponent>();

    for (auto &entity : entities) {
      auto &transform = entity->getComponent<TransformComponent>();

      // Generic boundary checking - could be configurable
      if (transform.position.y < 0 || transform.position.y > 600 - 32) {
        // Mark entity as dead or destroy it
        entity->destroy();
      }
    }
  }
};

// GENERIC Cleanup System - replaces PipeSystem cleanup
class OffscreenCleanupSystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities =
        entityManager.getEntitiesWithComponents<TransformComponent>();

    for (auto &entity : entities) {
      auto &transform = entity->getComponent<TransformComponent>();

      // Generic offscreen cleanup
      if (transform.position.x < -100.0f) {
        entity->destroy();
      }
    }
  }
};