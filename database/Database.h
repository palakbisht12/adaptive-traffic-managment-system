#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <memory>

#include "../integration/traffic.h"

using namespace std;

class IStorage {
public:
    virtual ~IStorage() {}

    virtual bool open(const string& target, string& error) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual string backendName() const = 0;

    virtual bool insertRoad(const RoadState& road, string& error) = 0;
    virtual bool updateRoad(const RoadState& road, string& error) = 0;
    virtual bool deleteRoad(const string& roadID, string& error) = 0;
    virtual bool selectRoad(const string& roadID, RoadState& out,
                            string& error) = 0;
    virtual bool selectAllRoads(vector<RoadState>& out,
                                string& error) = 0;

    virtual bool insertTrafficRecord(const TrafficRecord& rec,
                                     string& error) = 0;
    virtual bool selectTrafficHistory(const string& roadID, int limit,
                                      vector<TrafficRecord>& out,
                                      string& error) = 0;

    virtual bool insertSignalRecord(const SignalRecord& rec,
                                    string& error) = 0;
    virtual bool selectSignalHistory(const string& roadID, int limit,
                                     vector<SignalRecord>& out,
                                     string& error) = 0;

    // emergencies
    virtual bool insertEmergency(const EmergencyInfo& ev, int& newID,
                                 string& error) = 0;
    virtual bool completeEmergency(const string& roadID,
                                   const string& endTime,
                                   string& error) = 0;
    virtual bool selectEmergencies(const string& roadID,
                                   bool onlyActive,
                                   vector<EmergencyInfo>& out,
                                   string& error) = 0;

    virtual bool clearAll(string& error) = 0;
};

class Database {
private:
    unique_ptr<IStorage> storage;
    string lastError;
    bool connected;

    static string nowTimestamp();
    bool requireOpen(OperationResult& result) const;

public:
    Database();
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    OperationResult connect(const string& target = "traffic_data");
    OperationResult createTables();
    void disconnect();

    bool isConnected() const;
    string getBackendName() const;
    string getLastError() const;

    OperationResult insertRoad(const RoadState& road);
    OperationResult updateRoad(const RoadState& road);
    OperationResult deleteRoad(const string& roadID);
    OperationResult getRoad(const string& roadID, RoadState& out);
    OperationResult getAllRoads(vector<RoadState>& out);

    OperationResult saveTrafficRecord(const RoadState& road);
    OperationResult getTrafficHistory(const string& roadID, int limit,
                                      vector<TrafficRecord>& out);

    OperationResult saveSignalChange(const string& roadID,
                                     const string& previousSignal,
                                     const string& newSignal);
    OperationResult getSignalHistory(const string& roadID, int limit,
                                     vector<SignalRecord>& out);

    OperationResult saveEmergencyEvent(const string& roadID,
                                       const string& type,
                                       int& eventID);
    OperationResult completeEmergency(const string& roadID);
    OperationResult getActiveEmergencies(vector<EmergencyInfo>& out);
    OperationResult getEmergencyHistory(const string& roadID,
                                        vector<EmergencyInfo>& out);

    OperationResult clearAll();

    static unique_ptr<IStorage> makeStorage();
};

#endif