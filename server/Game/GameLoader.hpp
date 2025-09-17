#include <string>
#include "ConfigurationManager.hpp"
#include "GameLoop.hpp"

namespace Orchestrator {
    class GameLoader
    {
        private:
            // Core Services
            ConfigurationManager* _configManager;
            GameLoop* _gameServer;
            // EntityManager* _entityManager;
            
            // // Game Managers
            // SessionManager* _sessionManager;
            // CombatManager* _combatManager;
            // WaveManager* _waveManager;
            // PhysicsManager* _physicsManager;
            // FormationManager* _formationManager;
            // EntityFactory* _entityFactory;
            // TimingManager* _timingManager;
            
            // // Communication & Main Server
            // GameMediator* _gameMediator;
            // NetworkManager* _networkManager;
            
        public:
            GameLoader();
            ~GameLoader();
            void initialize(std::string ConfigPath);
            void startServer();
    };
}