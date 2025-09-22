#pragma once
#include "GameMediator.hpp"
#include <memory>

namespace Orchestrator
{
class GameLoop
{
  private:
  GameMediator _gameMediator;

  public:
    GameLoop();
    ~GameLoop();
    void loop();
};
}; // namespace Orchestrator