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
   
    // Train more workers so we can gather more income
    
    if(!m_enemyFound)
    {
        scoutStartingPositions();
        isEnemyFound();
    }
    else if(m_enemyFound)
    {

        auto firstDistance= Tools::DistanceBetweenPositions(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), m_enemyBasePosition);
        auto secondDistance = Tools::DistanceBetweenPositions(m_scout->getPosition(), BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
        if(firstDistance-secondDistance >400 && !m_scoutFleeCompleted)
        {
            m_scout->stop();
            m_scoutFleeCompleted = true;
        }
        //if(m_enemyRace== BWAPI::Races::Zerg)
        //{
            photonCannonRush();
        //}
    }
    
    
    
    // Build more supply if we are going to run out soon
    //buildAdditionalSupply();
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 20), "Junaid Haque, Fatema Haque");
    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}

void StarterBot::photonCannonRush()
{
    auto workersOwned = Tools::CountUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker(), BWAPI::Broodwar->self()->getUnits());
    auto numberOfPylons = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits());
    auto numberOfForges = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Forge, BWAPI::Broodwar->self()->getUnits());

    auto numberOfPylonsCompleted = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits(),true);
    auto numberOfForgesCompleted = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Forge, BWAPI::Broodwar->self()->getUnits(),true);

    auto numberOfCannons = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::Broodwar->self()->getUnits());

    if(workersOwned<6)
    {
        trainAdditionalWorkers();
    }
    if(numberOfPylons < 1)
    {
        if(BWAPI::Broodwar->self()->minerals()>=100)
        {
            Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon);
        }
    }
    else if(numberOfForges<1)
    {
        if(BWAPI::Broodwar->self()->minerals() >= 150)
        {
            Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Forge);
        }
    }
    if(m_scout->isStuck())
    {
        auto xOffset = m_scout->getPosition().x > 1000 ? -100 : +100;
        auto yOffset = m_scout->getPosition().y > 400 ? -100 : +100;
        m_scout->move(BWAPI::Position(m_scout->getPosition().x + xOffset, m_scout->getPosition().y + yOffset));
    }

    if(!m_scout->canMove() && m_scoutsDead<3)
    {
        trainAdditionalWorkers();
        auto worker = Tools::GetUnitOfTypeClosestTo(BWAPI::Broodwar->self()->getRace().getWorker(), Tools::GetDepot()->getPosition());
        m_scout = worker;
        m_scout->move(m_enemyBasePosition);
        m_scoutsDead++;
    }
    if (numberOfCannons >= 6 || m_scoutsDead>2 || !m_enemyFound)
    {
        zealotRush();
    }
    else if(m_scout!=nullptr && m_scout->canBuild() && !m_scout->isConstructing() && numberOfCannons<6)
    {
        if(BWAPI::Broodwar->self()->minerals() >= 100 && numberOfForgesCompleted>=1 && numberOfPylons<3)
        {
            BWAPI::TilePosition buildPosition = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Pylon, m_scout->getTilePosition(), 64);
            if(BWAPI::Broodwar->canBuildHere(buildPosition, m_scout->getType(), m_scout))
            {
                bool isBuilding = m_scout->build(BWAPI::UnitTypes::Protoss_Pylon, buildPosition);
                if(!isBuilding)
                {
                    auto xOffset = m_scout->getPosition().x > 1000 ? -100 : +100;
                    auto yOffset = m_scout->getPosition().y > 400 ? -100 : +100;

                    m_scout->move(BWAPI::Position(m_scout->getPosition().x + xOffset, m_scout->getPosition().y + yOffset));
                }
            }
        }

        if(BWAPI::Broodwar->self()->minerals()>= 150 && numberOfPylons>=2 && numberOfForgesCompleted>=1)
        {
            BWAPI::TilePosition buildPosition = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Photon_Cannon, m_scout->getTilePosition(), 64);
            if (BWAPI::Broodwar->canBuildHere(buildPosition, m_scout->getType(), m_scout))
            {
                auto isBuilding = m_scout->build(BWAPI::UnitTypes::Protoss_Photon_Cannon, buildPosition);
                if (!isBuilding)
                {
                    auto xOffset = m_scout->getPosition().x > 1000 ? -100 : +100;
                    auto yOffset = m_scout->getPosition().y > 400 ? -100 : +100;

                    m_scout->move(BWAPI::Position(m_scout->getPosition().x + xOffset, m_scout->getPosition().y + yOffset));
                }
            }
        }
    }
    
}
void StarterBot::zealotRush()
{

    auto workersOwned = Tools::CountUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker(), BWAPI::Broodwar->self()->getUnits());
    auto zealotsOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->self()->getUnits());
    auto gatewaysOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits());
    auto pylonsOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits());

    if (m_enemyFound)
    {
        auto zealotsAtBase = 0;
        auto& units = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : units)
        {
            if (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot && unit->isIdle() && unit->getDistance(Tools::GetDepot()) < unit->getDistance(m_enemyBasePosition))
            {
                zealotsAtBase++;
            }
        }
        if ((zealotsAtBase >= 2 && m_rushCount == 0) || (m_rushCount > 0 && zealotsAtBase >= 10))
        {
            startZealotRush();
        }
    }
    //fleeZealot();
    zealotsAttack();
    if ((Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed()) <= 12 && Tools::GetTotalSupply(true) > 20)
    {
        buildAdditionalSupply();
    }

    if (workersOwned >= 10)
    {
        trainZealots(gatewaysOwned);
    }
    if ((workersOwned == 8 && gatewaysOwned <= 1) || (BWAPI::Broodwar->self()->minerals() >= 500 && gatewaysOwned <= 2))
    {
        buildAdditionalSupply();
        createGateways();
        //scoutStartingPositions();
    }

    else if (workersOwned <= 8 || (pylonsOwned >= 1 && workersOwned < 10) || (m_rushCount == 1 && workersOwned < 15))
    {
        trainAdditionalWorkers();
    }
}

void StarterBot::fleeZealot()
{
    auto& units = BWAPI::Broodwar->self()->getUnits();
    for(auto& unit:units)
    {
        if(unit->getType() == BWAPI::UnitTypes::Protoss_Zealot && (unit->getShields()<10))
        {
            unit->move(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
        }
    }
}

void StarterBot::createGateways()
{
    auto pylonsOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits());

    auto gatewaysOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits());

    if (pylonsOwned >= 1 && BWAPI::Broodwar->self()->minerals() >= 150 && gatewaysOwned <= 2)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Enum::Protoss_Gateway);
        
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
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(Tools::GetDepot(), BWAPI::Broodwar->getMinerals());
            
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
            if(unit->getType() == BWAPI::UnitTypes::Protoss_Gateway && !unit->isTraining() && BWAPI::Broodwar->self()->minerals() >= 100)
            {
                unit->train(BWAPI::UnitTypes::Protoss_Zealot);
            }
        }
    }
}

void StarterBot::startZealotRush()
{
    auto& units = BWAPI::Broodwar->self()->getUnits();
    BWAPI::Unit targetEnemy = nullptr;    
    for (auto& unit : units)
    {
        if (unit->isIdle() && unit->getType() == BWAPI::UnitTypes::Protoss_Zealot && unit->isCompleted() && unit->getShields() > 40)
        {
            unit->move(m_enemyBasePosition,true);
            
        }
    }
    m_rushCount++;
}

void StarterBot::zealotsAttack()
{
    auto& units = BWAPI::Broodwar->self()->getUnits();
    auto& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();

    for (auto& unit : units)
    {
        
        auto closestEnemy = Tools::GetClosestVisibleEnemyTo(unit);
        if(closestEnemy)
        {
            if (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot && unit->canAttackUnit() && closestEnemy->isVisible())
            {
                if (std::find(m_currentlyAttackingUnits.begin(), m_currentlyAttackingUnits.end(), unit) != m_currentlyAttackingUnits.end())
                {
                    if (!unit->isAttacking())
                    {
                        remove(m_currentlyAttackingUnits.begin(), m_currentlyAttackingUnits.end(), unit);
                    }
                }
                else
                {
                    if (unit->getDistance(closestEnemy) < 1000)
                    {
                        unit->stop();
                        //closestEnemy = Tools::GetClosestVisibleEnemyTo(unit);
                        unit->attack(closestEnemy);
                        m_currentlyAttackingUnits.push_back(unit);
                    }

                    /* v does not contain x */
                }
            }
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

    if (BWAPI::Broodwar->self()->minerals() >= 100) 
    {
        const bool startedBuilding = Tools::BuildBuilding(supplyProviderType, true);
        if (startedBuilding)
        {
            BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
        }
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
                std::cout << pos.x << " " << pos.y << "\n";
                m_enemyBasePosition = pos;
                scout->move(pos);
                m_scout = scout;
                Tools::m_scoutID = scout->getID();
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
            m_enemyRace = enemyRace;
            m_enemyFound = true;
            m_enemyBasePosition = m_scout->getPosition();
            m_scout->stop();
                
            if(m_enemyRace!= BWAPI::Races::Zerg)
            {
                m_scout->move(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
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
    else if(text == "speed1")
    {
        BWAPI::Broodwar->setLocalSpeed(1);
    }
    else if (text == "speed5")
    {
        BWAPI::Broodwar->setLocalSpeed(5);
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