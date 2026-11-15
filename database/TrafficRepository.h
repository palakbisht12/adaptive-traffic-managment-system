
#ifndef TRAFFICREPOSITORY_H
#define TRAFFICREPOSITORY_H

#include <string>
#include <vector>
#include "Database.h"

using namespace std;

class TrafficRepository {
private:
    Database& db;

public:
    explicit TrafficRepository(Database& database) : db(database) {}

    OperationResult saveTrafficUpdate(const RoadState& road);

    OperationResult getTrafficHistory(const string& roadID,
                                      vector<TrafficRecord>& out,
                                      int limit = 0);

    OperationResult getLatestTrafficData(const string& roadID,
                                         TrafficRecord& out);

    OperationResult getAllTrafficRecords(vector<TrafficRecord>& out,
                                         int limit = 0);
};

#endif
