#ifndef ROADREPOSITORY_H
#define ROADREPOSITORY_H

#include <string>
#include <vector>
#include "Database.h"

using namespace std;

class RoadRepository {
private:
    Database& db;

public:
    explicit RoadRepository(Database& database) : db(database) {}

    OperationResult addRoad(const RoadState& road);
    OperationResult findRoad(const string& roadID, RoadState& out);
    OperationResult updateRoad(const RoadState& road);
    OperationResult deleteRoad(const string& roadID);
    OperationResult getAllRoads(vector<RoadState>& out);

    bool exists(const string& roadID);
    int count();
};

#endif