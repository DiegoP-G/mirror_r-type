#pragma once
#include "GameMediator.hpp"
#include <memory>

namespace Orchestrator
{
class GameLoop
{
  private:
    std::shared_ptr<GameMediator> _GameMediator;

  public:
    GameLoop();
    ~GameLoop();
    void loop();
};
}; // namespace Orchestrator