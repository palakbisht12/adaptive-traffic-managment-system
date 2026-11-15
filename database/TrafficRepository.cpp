
#include "TrafficRepository.h"

using namespace std;

OperationResult TrafficRepository::saveTrafficUpdate(const RoadState& road) {
    return db.saveTrafficRecord(road);
}

OperationResult TrafficRepository::getTrafficHistory(
    const string& roadID, vector<TrafficRecord>& out, int limit) {
    return db.getTrafficHistory(roadID, limit, out);
}

OperationResult TrafficRepository::getLatestTrafficData(
    const string& roadID, TrafficRecord& out) {
    vector<TrafficRecord> rows;

    OperationResult r = db.getTrafficHistory(roadID, 1, rows);

    if (!r.ok) {
        return r;
    }

    if (rows.empty()) {
        return OperationResult::failure("No traffic records for " + roadID, 404);
    }

    out = rows[0];

    return OperationResult::success();
}

OperationResult TrafficRepository::getAllTrafficRecords(
    vector<TrafficRecord>& out, int limit) {
    return db.getTrafficHistory("", limit, out);
}
