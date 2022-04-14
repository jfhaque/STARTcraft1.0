#pragma once

#include "MapTools.h"

#include <BWAPI.h>

class StarterBot
{
    MapTools m_mapTools;
	BWAPI::Unit m_scout = nullptr;
	BWAPI::Unit m_randomEnemy = nullptr;
	BWAPI::Race m_enemyRace;
	bool m_enemyFound = false;
	BWAPI::Position m_enemyBasePosition;
	bool m_currentlyBuilding = false;
	std::vector<BWAPI::Unit> m_currentlyAttackingUnits;
	int m_rushCount = 0;
public:

    StarterBot();

    // helper functions to get you started with bot programming and learn the API
    void sendIdleWorkersToMinerals();
    void trainAdditionalWorkers();
	void trainZealots(int gatewaysOwned);
    void buildAdditionalSupply();
    void drawDebugInformation();
	void createGateways();
	void scoutStartingPositions();
	void isEnemyFound();
	void startZealotRush();
	void fleeZealot();
	void zealotsAttack();

    // functions that are triggered by various BWAPI events from main.cpp
	void onStart();
	void onFrame();
	void onEnd(bool isWinner);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onSendText(std::string text);
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitComplete(BWAPI::Unit unit);
	void onUnitShow(BWAPI::Unit unit);
	void onUnitHide(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);

	bool isScout(BWAPI::Unit unit);
};