#include "SignalRepository.h"

using namespace std;

OperationResult SignalRepository::saveSignalChange(
    const string& roadID, const string& previousSignal,
    const string& newSignal) {
    return db.saveSignalChange(roadID, previousSignal, newSignal);
}

OperationResult SignalRepository::getSignalHistory(
    vector<SignalRecord>& out, int limit) {
    return db.getSignalHistory("", limit, out);
}

OperationResult SignalRepository::getLatestSignal(const string& roadID,
                                                  SignalRecord& out) {
    vector<SignalRecord> rows;

    OperationResult r = db.getSignalHistory(roadID, 1, rows);
    if (!r.ok) {
        return r;
    }
    if (rows.empty()) {
        return OperationResult::failure("No signal records for " + roadID,
                                        404);
    }
    out = rows[0];
    return OperationResult::success();
}

OperationResult SignalRepository::getSignalChangesForRoad(
    const string& roadID, vector<SignalRecord>& out, int limit) {
    return db.getSignalHistory(roadID, limit, out);
}