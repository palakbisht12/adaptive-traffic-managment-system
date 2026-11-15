#include "EmergencyRepository.h"

using namespace std;

OperationResult EmergencyRepository::addEmergencyEvent(
    const string& roadID, const string& type, int& eventID) {
    if (hasActiveEmergency(roadID)) {
        return OperationResult::failure(
            "An emergency is already active on " + roadID, 409);
    }
    return db.saveEmergencyEvent(roadID, type, eventID);
}

OperationResult EmergencyRepository::markEmergencyCompleted(
    const string& roadID) {
    return db.completeEmergency(roadID);
}

OperationResult EmergencyRepository::getActiveEmergencies(
    vector<EmergencyInfo>& out) {
    return db.getActiveEmergencies(out);
}

OperationResult EmergencyRepository::getEmergencyHistory(
    const string& roadID, vector<EmergencyInfo>& out) {
    return db.getEmergencyHistory(roadID, out);
}

bool EmergencyRepository::hasActiveEmergency(const string& roadID) {
    vector<EmergencyInfo> active;
    if (!db.getActiveEmergencies(active).ok) {
        return false;
    }
    for (size_t i = 0; i < active.size(); i++) {
        if (active[i].roadID == roadID) {
            return true;
        }
    }
    return false;
}