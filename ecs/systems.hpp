#pragma once
#include "../client/GraphicsManagerSFML.hpp"
#include "components.hpp"
#include "ecs.hpp"
#include <random>
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


Vector2D getActualPosition(Entity* entity) {
    auto& transform = entity->getComponent<TransformComponent>();
    Vector2D position = transform.position;
    
    // Utiliser hasComponent au lieu d'un try-catch
    if (entity->hasComponent<CenteredComponent>()) {
        auto& centered = entity->getComponent<CenteredComponent>();
        position.x -= centered.offsetX;
        position.y -= centered.offsetY;
    }
    
    return position;
}

// System for rendering
class RenderSystem {
public:
  void update(EntityManager &entityManager) {
    if (!g_graphics) return;

    auto entities = entityManager.getEntitiesWithComponents<TransformComponent, SpriteComponent>();

    for (auto &entity : entities) {
      auto &transform = entity->getComponent<TransformComponent>();
      auto &sprite = entity->getComponent<SpriteComponent>();

      if (!sprite.isVisible) continue;

      Vector2D position = getActualPosition(entity);

      // If we have a texture, use it; otherwise draw a colored rectangle
      if (sprite.texture) {
        g_graphics->drawTexture(*sprite.texture, 
                               static_cast<int>(position.x), 
                               static_cast<int>(position.y),
                               sprite.width, sprite.height);
      } else {
        // Draw colored rectangle
        g_graphics->drawRect(static_cast<int>(position.x), 
                           static_cast<int>(position.y),
                           sprite.width, sprite.height,
                           sprite.r, sprite.g, sprite.b, sprite.a);
      }
    }
  }
};


// System for game rules and logic
class GameLogicSystem {
    private:
        float enemySpawnTimer = 0.0f;
        const float ENEMY_SPAWN_INTERVAL = 2.0f;
        const float ENEMY_SPEED = -200.0f;
        std::mt19937 rng;
        int score = 0;
        int stageCount = 0; // 0 = normal, 1 = stage 1, 2 = stage 2, 3 = 1vs 1;
        int stageStatus = 0; // 0 = not started, 1 = in progress, 2 = ended;
    public:
        GameLogicSystem() : rng(std::random_device{}()) {}

        void update(EntityManager& entityManager, float deltaTime) {
            // Spawn enemies
            enemySpawnTimer += deltaTime;
            // if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
            //     spawnEnemy(entityManager);
            //     enemySpawnTimer = 0.0f;
            // }   
            if (stageStatus == 0) {
                stageStatus = 1;
                stageCount = 1;
                spawnLaser1(entityManager);
            }

            // Update score based on passed enemies
            updateScore(entityManager);
            
            // Check for game over conditions
            checkGameOverConditions(entityManager);
        }
        
        int getScore() const { return score; }
        
private:
    void spawnLaser1(EntityManager& entityManager) {
        auto& laser = entityManager.createEntity();
        float width = 900.0f;
        float height = 20.0f;

        laser.addComponent<TransformComponent>(width / 2, 100.0f); // Centered horizontally
        laser.addComponent<CenteredComponent>(width / 2, 0); // Centered
        laser.addComponent<SpriteComponent>(width, height, 255, 0, 0); // Full height
        laser.addComponent<LaserWarningComponent>(width, height, 5.0f, 1.0f, 3.0f); // Warning for 2s, active for 3s
    }

    void spawnEnemy(EntityManager& entityManager) {
        std::uniform_real_distribution<float> heightDist(50.0f, 550.0f);
        float enemyHeight = heightDist(rng);
        
        auto& enemy = entityManager.createEntity();
        enemy.addComponent<TransformComponent>(800.0f, enemyHeight);
        enemy.addComponent<VelocityComponent>(ENEMY_SPEED, 0.0f);
        enemy.addComponent<SpriteComponent>(80, static_cast<int>(600 - enemyHeight), 0, 255, 0);
        enemy.addComponent<ColliderComponent>(80.0f, 600 - enemyHeight);
        enemy.addComponent<EnemyComponent>(0, 0.0f); // Type 0 = basic enemy
    }
    
    void updateScore(EntityManager& entityManager) {
        // Score logic here - e.g., increment score when enemy passes player
        auto enemies = entityManager.getEntitiesWithComponents<EnemyComponent, TransformComponent>();
        for (auto& enemy : enemies) {
            auto& transform = enemy->getComponent<TransformComponent>();
            auto& enemyComp = enemy->getComponent<EnemyComponent>();
            
            // If enemy passed player position and wasn't counted yet
            // if (transform.position.x < 80.0f && !enemyComp.counted) {
            //     enemyComp.counted = true;
            //     score++;
            // }
        }
    }
    
    void checkGameOverConditions(EntityManager& entityManager) {
        // Additional game over logic here
    }
};

class LaserWarningSystem {
public:
    void update(EntityManager& entityManager, float deltaTime) {
        auto entities = entityManager.getEntitiesWithComponents<LaserWarningComponent>();
        for (auto& entity : entities) {
            auto& laser = entity->getComponent<LaserWarningComponent>();
            auto& sprite = entity->getComponent<SpriteComponent>();
            try {
                auto& transform = entity->getComponent<TransformComponent>();
            } catch (const std::runtime_error& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                continue;
            }
            if (laser.appearanceTime > 0.0f) {
                laser.appearanceTime -= deltaTime;
                sprite.isVisible = false;
                continue; // Wait until appearance time is over
            }
            if (!laser.isActive) {
                if (!laser.warningShown) {
                    laser.warningShown = true;
                    sprite.isVisible = true;
                    sprite.r = 200; // Red color for warning
                    sprite.g = 0;
                    sprite.b = 0;
                    sprite.height = laser.height / 4;
                    sprite.width = laser.width;
                }
                laser.warningTime -= deltaTime;
                if (laser.warningTime <= 0.0f) {
                    laser.isActive = true;
                    sprite.r = 255;
                    sprite.height = laser.height;
                    
                    // Activer le laser (collision, dégâts, sprite différent, etc.)
                }
            } else {
                // Phase active
                laser.activeTime -= deltaTime;
                if (laser.activeTime <= 0.0f) {
                    // Désactiver ou détruire le laser
                    entity->destroy();
                }
            }
        }
    }
};


// System for collision detection
class CollisionSystem {
public:
void update(EntityManager &entityManager) {
    auto entities = entityManager.getEntitiesWithComponents<TransformComponent, ColliderComponent>();

    for (size_t i = 0; i < entities.size(); i++) {
      if (!entities[i]->isActive())
        continue;

      // Obtenir la position réelle avec le helper
      Vector2D position1 = getActualPosition(entities[i]);
      auto &collider1 = entities[i]->getComponent<ColliderComponent>();
      
      // Mettre à jour la hitbox avec la position calculée
      collider1.hitbox.x = position1.x;
      collider1.hitbox.y = position1.y;

      for (size_t j = i + 1; j < entities.size(); j++) {
        if (!entities[j]->isActive())
          continue;

        // De même pour la deuxième entité
        Vector2D position2 = getActualPosition(entities[j]);
        auto &collider2 = entities[j]->getComponent<ColliderComponent>();
        
        collider2.hitbox.x = position2.x;
        collider2.hitbox.y = position2.y;

        // Vérification de collision
        if (collider1.hitbox.intersects(collider2.hitbox)) {
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

// InputSystem - se contente de traduire les entrées en modifications de composants
class InputSystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, VelocityComponent>();
    for (auto &entity : entities) {
      auto &input = entity->getComponent<InputComponent>();
      auto &velocity = entity->getComponent<VelocityComponent>();

      // Reset velocity
      velocity.velocity.x = 0.0f;
      velocity.velocity.y = 0.0f;

      // Update velocity based on input
      const float PLAYER_SPEED = 200.0f;
      if (input.up) velocity.velocity.y = -PLAYER_SPEED;
      if (input.down) velocity.velocity.y = PLAYER_SPEED;
      if (input.left) velocity.velocity.x = -PLAYER_SPEED;
      if (input.right) velocity.velocity.x = PLAYER_SPEED;
    }
  }
};


// PlayerSystem - gère les comportements spécifiques au joueur
class PlayerSystem {
public:
  void update(EntityManager &entityManager, float deltaTime) {
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();
    for (auto &entity : entities) {
      auto &input = entity->getComponent<InputComponent>();
      
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
        transform.position.x + 32.0f, // Offset to fire from the front
        transform.position.y + 16.0f  // Center height
    );

    // Add bullet components
    bullet.addComponent<VelocityComponent>(300.0f, 0.0f);
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