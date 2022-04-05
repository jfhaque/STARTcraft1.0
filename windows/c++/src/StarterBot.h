#pragma once

#include "MapTools.h"

#include <BWAPI.h>

class StarterBot
{
    MapTools m_mapTools;
	bool m_scoutActive = false;
	bool m_enemyFound = false;
	bool m_currentlyBuilding = false;
	enum races{Protoss, Terrans, Zerg};
	races m_enemyRace;

public:

    StarterBot();

    // helper functions to get you started with bot programming and learn the API
    void sendIdleWorkersToMinerals();
    void trainAdditionalWorkers();
	void trainZealots(int gatewaysOwned);
    void buildAdditionalSupply();
    void drawDebugInformation();
	void createAPylonAndGateways();
	void scoutStartingPositions();

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
};