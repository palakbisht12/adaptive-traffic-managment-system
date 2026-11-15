#include "RoadRepository.h"

using namespace std;

OperationResult RoadRepository::addRoad(const RoadState& road) {
    return db.insertRoad(road);
}

OperationResult RoadRepository::findRoad(const string& roadID,
                                         RoadState& out) {
    return db.getRoad(roadID, out);
}

OperationResult RoadRepository::updateRoad(const RoadState& road) {
    return db.updateRoad(road);
}

OperationResult RoadRepository::deleteRoad(const string& roadID) {
    return db.deleteRoad(roadID);
}

OperationResult RoadRepository::getAllRoads(vector<RoadState>& out) {
    return db.getAllRoads(out);
}

bool RoadRepository::exists(const string& roadID) {
    RoadState probe;
    return db.getRoad(roadID, probe).ok;
}

int RoadRepository::count() {
    vector<RoadState> all;
    if (!db.getAllRoads(all).ok) {
        return 0;
    }
    return (int)all.size();
}