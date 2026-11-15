#ifndef EMERGENCYREPOSITORY_H
#define EMERGENCYREPOSITORY_H

#include <string>
#include <vector>
#include "Database.h"

using namespace std;

class EmergencyRepository {
private:
    Database& db;

public:
    explicit EmergencyRepository(Database& database) : db(database) {}

    OperationResult addEmergencyEvent(const string& roadID,
                                      const string& type,
                                      int& eventID);
    OperationResult markEmergencyCompleted(const string& roadID);
    OperationResult getActiveEmergencies(vector<EmergencyInfo>& out);
    OperationResult getEmergencyHistory(const string& roadID,
                                        vector<EmergencyInfo>& out);
    bool hasActiveEmergency(const string& roadID);
};

#endif