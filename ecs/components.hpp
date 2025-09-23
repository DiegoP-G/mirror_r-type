#pragma once
#include "ecs.hpp"
#include <SFML/Graphics.hpp>
#include <cstring>
#include <vector>

// Vector2D for positions and movements
struct Vector2D {
  float x, y;

  Vector2D() : x(0.0f), y(0.0f) {}
  Vector2D(float x, float y) : x(x), y(y) {}

  Vector2D &operator+=(const Vector2D &v) {
    x += v.x;
    y += v.y;
    return *this;
  }

  Vector2D operator*(float scalar) const {
    return Vector2D(x * scalar, y * scalar);
  }

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(Vector2D));
    std::memcpy(data.data(), this, sizeof(Vector2D));
    return data;
  }

  static Vector2D deserialize(const uint8_t *data) {
    Vector2D vec;
    std::memcpy(&vec, data, sizeof(Vector2D));
    return vec;
  }
};

// Rectangle for collision and rendering
struct Rectangle {
  float x, y, w, h;

  Rectangle() : x(0), y(0), w(0), h(0) {}
  Rectangle(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

  bool intersects(const Rectangle &other) const {
    return !(x + w < other.x || other.x + other.w < x || y + h < other.y ||
             other.y + other.h < y);
  }

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(Rectangle));
    std::memcpy(data.data(), this, sizeof(Rectangle));
    return data;
  }

  static Rectangle deserialize(const uint8_t *data) {
    Rectangle rect;
    std::memcpy(&rect, data, sizeof(Rectangle));
    return rect;
  }
};

// Component for position, scale, and rotation
struct TransformComponent : public Component {
  Vector2D position;
  Vector2D scale;
  float rotation;

  TransformComponent() : position(0, 0), scale(1, 1), rotation(0) {}
  TransformComponent(float x, float y)
      : position(x, y), scale(1, 1), rotation(0) {}
  TransformComponent(float x, float y, float sx, float sy, float r)
      : position(x, y), scale(sx, sy), rotation(r) {}

  void update(float deltaTime) override {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data;
    auto posData = position.serialize();
    auto scaleData = scale.serialize();

    data.insert(data.end(), posData.begin(), posData.end());
    data.insert(data.end(), scaleData.begin(), scaleData.end());
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&rotation),
                reinterpret_cast<const uint8_t *>(&rotation) + sizeof(float));
    return data;
  }

  static TransformComponent deserialize(const uint8_t *data) {
    TransformComponent comp;
    comp.position = Vector2D::deserialize(data);
    comp.scale = Vector2D::deserialize(data + sizeof(Vector2D));
    std::memcpy(&comp.rotation, data + 2 * sizeof(Vector2D), sizeof(float));
    return comp;
  }
};

struct PlayerComponent : public Component {
  int score = 0;
  int lives = 3;
  float shootCooldown = 0.0f;
  int playerID;
  bool isLocal;

  PlayerComponent(int playerID = 0, bool isLocal = true)
      : playerID(playerID), isLocal(isLocal) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(int) * 3 + sizeof(float) + sizeof(bool));
    size_t offset = 0;

    std::memcpy(data.data() + offset, &score, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &lives, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &shootCooldown, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &playerID, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &isLocal, sizeof(bool));
    return data;
  }

  static PlayerComponent deserialize(const uint8_t *data) {
    PlayerComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.score, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.lives, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.shootCooldown, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.playerID, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.isLocal, data + offset, sizeof(bool));
    return comp;
  }
};

// Component for movement
struct VelocityComponent : public Component {
  Vector2D velocity;
  float maxSpeed;

  VelocityComponent() : velocity(0, 0), maxSpeed(5.0f) {}
  VelocityComponent(float x, float y) : velocity(x, y), maxSpeed(5.0f) {}
  VelocityComponent(float x, float y, float max)
      : velocity(x, y), maxSpeed(max) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data;
    auto velData = velocity.serialize();
    data.insert(data.end(), velData.begin(), velData.end());
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&maxSpeed),
                reinterpret_cast<const uint8_t *>(&maxSpeed) + sizeof(float));
    return data;
  }

  static VelocityComponent deserialize(const uint8_t *data) {
    VelocityComponent comp;
    comp.velocity = Vector2D::deserialize(data);
    std::memcpy(&comp.maxSpeed, data + sizeof(Vector2D), sizeof(float));
    return comp;
  }
};

// Component for Laser Warning
struct LaserWarningComponent : public Component {
  float appearanceTime;
  float warningTime;
  float activeTime;
  bool isActive;
  bool warningShown;
  float width;
  float height;

  LaserWarningComponent(float w = 10.0f, float h = 300.0f, float app = 1.0f,
                        float warn = 1.0f, float active = 1.0f)
      : width(w), height(h), appearanceTime(app), warningTime(warn),
        activeTime(active), isActive(false), warningShown(false) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(float) * 5 + sizeof(bool) * 2);
    size_t offset = 0;

    std::memcpy(data.data() + offset, &appearanceTime, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &warningTime, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &activeTime, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &isActive, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(data.data() + offset, &warningShown, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(data.data() + offset, &width, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &height, sizeof(float));
    return data;
  }

  static LaserWarningComponent deserialize(const uint8_t *data) {
    LaserWarningComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.appearanceTime, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.warningTime, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.activeTime, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.isActive, data + offset, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(&comp.warningShown, data + offset, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(&comp.width, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.height, data + offset, sizeof(float));
    return comp;
  }
};

// Component for centering entities on screen
struct CenteredComponent : public Component {
  float offsetX;
  float offsetY;
  float offsetX;
  float offsetY;

  CenteredComponent(float x = 0, float y = 0) : offsetX(x), offsetY(y) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(float) * 2);
    std::memcpy(data.data(), &offsetX, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &offsetY, sizeof(float));
    return data;
  }

  static CenteredComponent deserialize(const uint8_t *data) {
    CenteredComponent comp;
    std::memcpy(&comp.offsetX, data, sizeof(float));
    std::memcpy(&comp.offsetY, data + sizeof(float), sizeof(float));
    return comp;
  }
};

// Component for rendering with SFML
struct SpriteComponent : public Component {
  sf::Texture *texture; // Note: Cannot serialize SDL_Texture pointer
  Rectangle srcRect;
  int width, height;
  bool isVisible;
  uint8_t r, g, b, a;

  SpriteComponent()
      : texture(nullptr), isVisible(true), width(32), height(32), r(255),
        g(255), b(255), a(255) {}

  SpriteComponent(int w, int h, uint8_t red = 255, uint8_t green = 255,
                  uint8_t blue = 255)
      : texture(nullptr), isVisible(true), width(w), height(h), r(red),
        g(green), b(blue), a(255) {}

  void render() override {}

  // Serialization (excluding texture pointer)
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data;
    auto rectData = srcRect.serialize();
    data.insert(data.end(), rectData.begin(), rectData.end());

    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&width),
                reinterpret_cast<const uint8_t *>(&width) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&height),
                reinterpret_cast<const uint8_t *>(&height) + sizeof(int));
    data.push_back(static_cast<uint8_t>(isVisible));
    data.push_back(r);
    data.push_back(g);
    data.push_back(b);
    data.push_back(a);
    return data;
  }

  static SpriteComponent deserialize(const uint8_t *data) {
    SpriteComponent comp;
    comp.srcRect = Rectangle::deserialize(data);
    size_t offset = sizeof(Rectangle);

    std::memcpy(&comp.width, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.height, data + offset, sizeof(int));
    offset += sizeof(int);
    comp.isVisible = static_cast<bool>(data[offset++]);
    comp.r = data[offset++];
    comp.g = data[offset++];
    comp.b = data[offset++];
    comp.a = data[offset];
    comp.texture = nullptr; // Texture must be recreated
    return comp;
  }
};

// Component for collision
struct ColliderComponent : public Component {
  Rectangle hitbox;
  bool isActive;
  bool isTrigger;

  ColliderComponent() : isTrigger(false), isActive(true) {}
  ColliderComponent(float w, float h)
      : hitbox(0, 0, w, h), isTrigger(false), isActive(true) {}
  ColliderComponent(float w, float h, bool isAct)
      : hitbox(0, 0, w, h), isTrigger(false), isActive(isAct) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data;
    auto rectData = hitbox.serialize();
    data.insert(data.end(), rectData.begin(), rectData.end());
    data.push_back(static_cast<uint8_t>(isActive));
    data.push_back(static_cast<uint8_t>(isTrigger));
    return data;
  }

  static ColliderComponent deserialize(const uint8_t *data) {
    ColliderComponent comp;
    comp.hitbox = Rectangle::deserialize(data);
    size_t offset = sizeof(Rectangle);
    comp.isActive = static_cast<bool>(data[offset++]);
    comp.isTrigger = static_cast<bool>(data[offset]);
    return comp;
  }
};

// Component for player/enemy health
struct HealthComponent : public Component {
  float health;
  float maxHealth;

  HealthComponent(float h) : health(h), maxHealth(h) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(float) * 2);
    std::memcpy(data.data(), &health, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &maxHealth, sizeof(float));
    return data;
  }

  static HealthComponent deserialize(const uint8_t *data) {
    HealthComponent comp(0);
    std::memcpy(&comp.health, data, sizeof(float));
    std::memcpy(&comp.maxHealth, data + sizeof(float), sizeof(float));
    return comp;
  }
};

// Component for player input
struct InputComponent : public Component {
  bool up, down, left, right, fire;

  InputComponent()
      : up(false), down(false), left(false), right(false), fire(false) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(5);
    data[0] = static_cast<uint8_t>(up);
    data[1] = static_cast<uint8_t>(down);
    data[2] = static_cast<uint8_t>(left);
    data[3] = static_cast<uint8_t>(right);
    data[4] = static_cast<uint8_t>(fire);
    return data;
  }

  static InputComponent deserialize(const uint8_t *data) {
    InputComponent comp;
    comp.up = static_cast<bool>(data[0]);
    comp.down = static_cast<bool>(data[1]);
    comp.left = static_cast<bool>(data[2]);
    comp.right = static_cast<bool>(data[3]);
    comp.fire = static_cast<bool>(data[4]);
    return comp;
  }
};

// Component for projectiles
struct ProjectileComponent : public Component {
  float damage;
  float lifeTime;
  float remainingLife;
  EntityID owner;

  ProjectileComponent(float d, float lt, EntityID o)
      : damage(d), lifeTime(lt), remainingLife(lt), owner(o) {}

  void update(float deltaTime) override { remainingLife -= deltaTime; }

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(float) * 3 + sizeof(EntityID));
    size_t offset = 0;

    std::memcpy(data.data() + offset, &damage, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &lifeTime, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &remainingLife, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &owner, sizeof(EntityID));
    return data;
  }

  static ProjectileComponent deserialize(const uint8_t *data) {
    ProjectileComponent comp(0, 0, 0);
    size_t offset = 0;

    std::memcpy(&comp.damage, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.lifeTime, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.remainingLife, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.owner, data + offset, sizeof(EntityID));
    return comp;
  }
};

// Component for enemies
struct EnemyComponent : public Component {
  int type;
  int shootingType;
  float attackCooldown;
  float currentCooldown;

  EnemyComponent(int t, float ac, int st)
      : type(t), shootingType(st), attackCooldown(ac), currentCooldown(0) {}

  void update(float deltaTime) override {
    if (currentCooldown > 0) {
      currentCooldown -= deltaTime;
    }
  }

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(int) * 2 + sizeof(float) * 2);
    size_t offset = 0;

    std::memcpy(data.data() + offset, &type, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &shootingType, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &attackCooldown, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &currentCooldown, sizeof(float));
    return data;
  }

  static EnemyComponent deserialize(const uint8_t *data) {
    EnemyComponent comp(0, 0, 0);
    size_t offset = 0;

    std::memcpy(&comp.type, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.shootingType, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.attackCooldown, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.currentCooldown, data + offset, sizeof(float));
    return comp;
  }
};

// Flappy Bird specific components
struct BirdComponent : public Component {
  float jumpStrength;
  bool isDead;

  BirdComponent(float jump = -400.0f) : jumpStrength(jump), isDead(false) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(float) + sizeof(bool));
    std::memcpy(data.data(), &jumpStrength, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &isDead, sizeof(bool));
    return data;
  }

  static BirdComponent deserialize(const uint8_t *data) {
    BirdComponent comp;
    std::memcpy(&comp.jumpStrength, data, sizeof(float));
    std::memcpy(&comp.isDead, data + sizeof(float), sizeof(bool));
    return comp;
  }
};

struct PipeComponent : public Component {
  bool isTopPipe;
  float gapHeight;
  bool hasScored;

  PipeComponent(bool top = false, float gap = 150.0f)
      : isTopPipe(top), gapHeight(gap), hasScored(false) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(bool) * 2 + sizeof(float));
    size_t offset = 0;

    std::memcpy(data.data() + offset, &isTopPipe, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(data.data() + offset, &gapHeight, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &hasScored, sizeof(bool));
    return data;
  }

  static PipeComponent deserialize(const uint8_t *data) {
    PipeComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.isTopPipe, data + offset, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(&comp.gapHeight, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.hasScored, data + offset, sizeof(bool));
    return comp;
  }
};

struct GravityComponent : public Component {
  float gravity;
  float terminalVelocity;

  GravityComponent(float g = 800.0f, float tv = 600.0f)
      : gravity(g), terminalVelocity(tv) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(float) * 2);
    std::memcpy(data.data(), &gravity, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &terminalVelocity, sizeof(float));
    return data;
  }

  static GravityComponent deserialize(const uint8_t *data) {
    GravityComponent comp;
    std::memcpy(&comp.gravity, data, sizeof(float));
    std::memcpy(&comp.terminalVelocity, data + sizeof(float), sizeof(float));
    return comp;
  }
};

// Generic jump component instead of BirdComponent
struct JumpComponent : public Component {
  float jumpStrength;
  bool canJump;

  JumpComponent(float jump = -400.0f) : jumpStrength(jump), canJump(true) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(float) + sizeof(bool));
    std::memcpy(data.data(), &jumpStrength, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &canJump, sizeof(bool));
    return data;
  }

  static JumpComponent deserialize(const uint8_t *data) {
    JumpComponent comp;
    std::memcpy(&comp.jumpStrength, data, sizeof(float));
    std::memcpy(&comp.canJump, data + sizeof(float), sizeof(bool));
    return comp;
  }
};

struct GameStateComponent : public Component {
  int score;
  bool gameOver;
  bool started;

  GameStateComponent() : score(0), gameOver(false), started(false) {}

  // Serialization
  std::vector<uint8_t> serialize() const {
    std::vector<uint8_t> data(sizeof(int) + sizeof(bool) * 2);
    size_t offset = 0;

    std::memcpy(data.data() + offset, &score, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &gameOver, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(data.data() + offset, &started, sizeof(bool));
    return data;
  }

  static GameStateComponent deserialize(const uint8_t *data) {
    GameStateComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.score, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.gameOver, data + offset, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(&comp.started, data + offset, sizeof(bool));
    return comp;
  }
};

struct AnimatedSpriteComponent : public Component {
  sf::Sprite sprite;
  const sf::Texture *texture; // Use a pointer to the texture
  int currentFrame = 2;       // Default frame
  const int frameWidth;       // Width of each frame
  const int frameHeight;      // Height of each frame
  float animationInterval;    // Time between frame changes
  Vector2D scale = {1.0f, 1.0f};
  sf::Clock animationClock;
  enum Direction { Default, Up, Down } currentDirection = Default;

  AnimatedSpriteComponent(const sf::Texture &tex, int frameWidth,
                          int frameHeight, float interval,
                          Vector2D scale = {1.0f, 1.0f})
      : texture(&tex), frameWidth(frameWidth), frameHeight(frameHeight),
        animationInterval(interval), scale(scale) {
    sprite.setTexture(*texture); // Use the texture pointer
    sprite.setScale(scale.x, scale.y);
    setFrame(currentFrame); // Set default frame
  }

  void setFrame(int frame) {
    sprite.setTextureRect(
        sf::IntRect(frameWidth * frame, 0, frameWidth, frameHeight));
  }

  void updateAnimation(Direction newDirection) {
    if (newDirection != currentDirection) {
      currentDirection = newDirection;
    }

    if (animationClock.getElapsedTime().asSeconds() >= animationInterval) {
      if (currentDirection == Up && currentFrame < 4) {
        currentFrame++;
      } else if (currentDirection == Down && currentFrame > 0) {
        currentFrame--;
      } else if (currentDirection == Default) {
        if (currentFrame > 2)
          currentFrame--;
        else if (currentFrame < 2)
          currentFrame++;
      }
      setFrame(currentFrame);
      animationClock.restart();
    }
  }

  // Override init method from Component
  void init() override {}
};

struct AnimatedSpriteComponent {
  sf::Sprite sprite;
  sf::Texture texture;
  int currentFrame = 2;            // Default frame
  const int frameWidth = 33;       // Width of each frame
  const int frameHeight = 17.5;    // Height of each frame
  float animationInterval = 0.05f; // Time between frame changes
  sf::Clock animationClock;
  enum Direction { Default, Up, Down } currentDirection = Default;

  AnimatedSpriteComponent(const sf::Texture &tex, int frameWidth,
                          int frameHeight, float interval)
      : texture(tex), frameWidth(frameWidth), frameHeight(frameHeight),
        animationInterval(interval) {
    sprite.setTexture(texture);
    setFrame(currentFrame); // Set default frame
  }

  void setFrame(int frame) {
    sprite.setTextureRect(
        sf::IntRect(frameWidth * frame, 0, frameWidth, frameHeight));
  }

  void updateAnimation(Direction newDirection) {
    if (newDirection != currentDirection) {
      currentDirection = newDirection;
    }

    if (animationClock.getElapsedTime().asSeconds() >= animationInterval) {
      if (currentDirection == Up && currentFrame < 4) {
        currentFrame++;
      } else if (currentDirection == Down && currentFrame > 0) {
        currentFrame--;
      } else if (currentDirection == Default) {
        if (currentFrame > 2)
          currentFrame--;
        else if (currentFrame < 2)
          currentFrame++;
      }
      setFrame(currentFrame);
      animationClock.restart();
    }
  }
};