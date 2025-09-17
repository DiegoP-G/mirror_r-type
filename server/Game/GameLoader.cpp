#include <string>
#include "GameLoader.hpp"
#include "ConfigurationManager.hpp"
#include "GameLoop.hpp"

void Orchestrator::GameLoader::initialize(std::string ConfigPath)
{
    _configManager = new ConfigurationManager;
    _configManager->loadConfiguration(ConfigPath);
    _gameLoop = new GameLoop(
        // entityManager, 
        // gameMediator, 
        // networkManager,
        // timingManager
    );
    
    // entityManager = new EntityManager();
    
    // sessionManager = new SessionManager(entityManager);
    // combatManager = new CombatManager(entityManager);
    // waveManager = new WaveManager(entityManager, configManager);
    // physicsManager = new PhysicsManager(entityManager);
    // formationManager = new FormationManager(entityManager);
    // entityFactory = new EntityFactory(entityManager, configManager);
    // timingManager = new TimingManager();
    
    // gameMediator = new GameMediator(
    //     sessionManager,
    //     combatManager,
    //     waveManager,
    //     physicsManager,
    //     formationManager
    // );
    
    // networkManager = new NetworkManager(gameMediator);
    
    
    // // Step 6: Initialize base entities
    // entityFactory->createInitialEntities();
}

void Orchestrator::GameLoader::startServer()
{
    _gameLoop->loop();
}