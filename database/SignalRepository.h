#ifndef SIGNALREPOSITORY_H
#define SIGNALREPOSITORY_H


#include <string>
#include <vector>
#include "Database.h"

using namespace std;

class SignalRepository {
private:
    Database& db;

public:
    explicit SignalRepository(Database& database) : db(database) {}

    OperationResult saveSignalChange(const string& roadID,
                                     const string& previousSignal,
                                     const string& newSignal);

    OperationResult getSignalHistory(vector<SignalRecord>& out,
                                     int limit = 0);

    OperationResult getLatestSignal(const string& roadID,
                                    SignalRecord& out);

    OperationResult getSignalChangesForRoad(const string& roadID,
                                            vector<SignalRecord>& out,
                                            int limit = 0);
};

#endif