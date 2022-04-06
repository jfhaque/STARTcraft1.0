#pragma once

#include <BWAPI.h>

namespace Tools
{
    BWAPI::Unit GetClosestUnitTo(BWAPI::Position p, const BWAPI::Unitset& units);
    BWAPI::Unit GetClosestUnitTo(BWAPI::Unit unit, const BWAPI::Unitset& units);

    int CountUnitsOfType(BWAPI::UnitType type, const BWAPI::Unitset& units); 

    BWAPI::Unit GetUnitOfType(BWAPI::UnitType type);
    BWAPI::Unit GetUnitOfTypeClosestTo(BWAPI::UnitType type, BWAPI::Position p);
    BWAPI::Unit GetDepot();

    bool BuildBuilding(BWAPI::UnitType type, bool forceConstruct = false);

    void DrawUnitBoundingBoxes();
    void DrawUnitCommands();

    void SmartRightClick(BWAPI::Unit unit, BWAPI::Unit target);

    int GetTotalSupply(bool inProgress = false);

    void DrawUnitHealthBars();
    void DrawHealthBar(BWAPI::Unit unit, double ratio, BWAPI::Color color, int yOffset);
}