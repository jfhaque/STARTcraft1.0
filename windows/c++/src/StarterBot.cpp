#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"

StarterBot::StarterBot()
{
    
}

// Called when the bot starts!
void StarterBot::onStart()
{
    // Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(10);
    BWAPI::Broodwar->setFrameSkip(0);
    
    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

    // Call MapTools OnStart
    m_mapTools.onStart();
}

// Called whenever the game ends and tells you if you won or not
void StarterBot::onEnd(bool isWinner) 
{
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    // Update our MapTools information
    m_mapTools.onFrame();
    
    
    // Send our idle workers to mine minerals so they don't just stand there
    sendIdleWorkersToMinerals();
    
    auto workersOwned = Tools::CountUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker(), BWAPI::Broodwar->self()->getUnits());
    auto zealotsOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->self()->getUnits());
    auto gatewaysOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits());
    // Train more workers so we can gather more income
    
    if(!m_enemyFound)
    {
        scoutStartingPositions();
        isEnemyFound();
    }
    
    if(m_enemyFound && zealotsOwned >15)
    {
        startZealotRush();
    }

    if(workersOwned ==8 && gatewaysOwned <=2) 
    {
        createAPylonAndGateways();
        //scoutStartingPositions();
    }  
    else if(workersOwned <8 || (gatewaysOwned == 3 && workersOwned<15))
    {
        trainAdditionalWorkers();
    }
    if ((Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed()) <=4 && Tools::GetTotalSupply(true)>7)
    {
        buildAdditionalSupply();
    }
    if(workersOwned>=10)
    {
        trainZealots(gatewaysOwned);
    }
    
    // Build more supply if we are going to run out soon
    //buildAdditionalSupply();
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 20), "Junaid Haque, Fatema Haque");
    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}

void StarterBot::createAPylonAndGateways()
{
    auto pylonsOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits());
    if (BWAPI::Broodwar->self()->minerals() >= 100 && pylonsOwned == 0)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Enum::Protoss_Pylon);
        /*Tools::BuildBuilding(BWAPI::UnitTypes::Enum::Protoss_Gateway);
        Tools::BuildBuilding(BWAPI::UnitTypes::Enum::Protoss_Gateway);*/
    }

    auto gatewaysOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits());
    if (pylonsOwned >= 1 && BWAPI::Broodwar->self()->minerals() >= 150 && gatewaysOwned <= 3)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Enum::Protoss_Gateway, true);
    }
}
// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle() && unit != m_scout)
        {
            // Get the closest mineral to this worker unit
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestMineral) { unit->rightClick(closestMineral); }
        }
    }
}

void StarterBot::trainZealots(int gatewaysOwned)
{
    if(gatewaysOwned >0)
    {
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits)
        {
            if(unit->getType() == BWAPI::UnitTypes::Protoss_Gateway && !unit->isTraining())
            {
                unit->train(BWAPI::UnitTypes::Protoss_Zealot);
            }
        }
    }
}

void StarterBot::startZealotRush()
{
    auto& units = BWAPI::Broodwar->self()->getUnits();
    auto& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();
    BWAPI::Unit targetEnemy = nullptr;

    for (auto& enemy : enemyUnits)
    {
        if (enemy->isVisible() && enemy->isAttacking())
        {
            targetEnemy = enemy;
            break;
        }
    }

    for( auto& unit : units)
    {
        if(unit->isIdle() && unit->getType()==BWAPI::UnitTypes::Protoss_Zealot && unit->isCompleted())
        {
            unit->move(m_enemyBasePosition, true);
        }
        if(targetEnemy!= nullptr && unit->getType() == BWAPI::UnitTypes::Protoss_Zealot)
        {
            unit->attack(targetEnemy);
        }
    }
}

// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersWanted = 20;
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    if (workersOwned < workersWanted)
    {
        // get the unit pointer to my depot
        const BWAPI::Unit myDepot = Tools::GetDepot();

        // if we have a valid depot unit and it's currently not training something, train a worker
        // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
        if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }
    }
}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{
    // Get the amount of supply supply we currently have unused
    const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

    // If we have a sufficient amount of supply, we don't need to do anything
    if (unusedSupply >= 4) { return; }

    // Otherwise, we are going to build a supply provider
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

    const bool startedBuilding = Tools::BuildBuilding(supplyProviderType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
    }
}

void StarterBot::scoutStartingPositions()
{
    BWAPI::Unit scout = Tools::GetUnitOfType(BWAPI::Broodwar->self()->getRace().getWorker());
    auto& startLocations = BWAPI::Broodwar->getStartLocations();
    
    if (scout != NULL)
    {
        for (auto& loc : startLocations)
        {
            if (BWAPI::Broodwar->isExplored(loc)) { continue; }
            if (!m_scout || m_scout->isIdle())
            {
                BWAPI::Position pos(loc);
                m_enemyBasePosition = pos;
                scout->move(pos);
                m_scout = scout;
                break;
            }
        }
    }
}

void StarterBot:: isEnemyFound()
{
    auto& units = BWAPI::Broodwar->enemy()->getUnits();
    for(auto& unit : units)
    {
        BWAPI::Race enemyRace = unit->getType().getRace();
        if (enemyRace == BWAPI::Races::Zerg
            || enemyRace == BWAPI::Races::Protoss
            || enemyRace == BWAPI::Races::Terran)
        {
            
            m_enemyRace = enemyRace;
            m_randomEnemy = Tools::GetClosestUnitTo(m_scout, units);
            m_enemyFound = true;
        }
        
    } 
}

// Draw some relevent information to the screen to help us debug the bot
void StarterBot::drawDebugInformation()
{
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");
    Tools::DrawUnitCommands();
    Tools::DrawUnitBoundingBoxes();
}

// Called whenever a unit is destroyed, with a pointer to the unit
void StarterBot::onUnitDestroy(BWAPI::Unit unit)
{
	
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void StarterBot::onUnitMorph(BWAPI::Unit unit)
{
	
}

// Called whenever a text is sent to the game by a user
void StarterBot::onSendText(std::string text) 
{ 
    if (text == "/map")
    {
        m_mapTools.toggleDraw();
    }
}

// Called whenever a unit is created, with a pointer to the destroyed unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void StarterBot::onUnitCreate(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{
	
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void StarterBot::onUnitHide(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void StarterBot::onUnitRenegade(BWAPI::Unit unit)
{ 
	
}

bool StarterBot::isScout(BWAPI::Unit unit)
{
    if(unit==m_scout)
    {
        return true;
    }
    return false;
}