#ifndef INTEGRATION_H
#define INTEGRATION_H


#include <string>
#include <vector>
#include <map>

#include "traffic.h"
#include "../include/TrafficManager.h"
#include "../database/Database.h"
#include "../database/RoadRepository.h"
#include "../database/TrafficRepository.h"
#include "../database/SignalRepository.h"
#include "../database/EmergencyRepository.h"

class Integration {
private:
    TrafficManager manager;      
    Database database;           

    RoadRepository roads;
    TrafficRepository trafficLog;
    SignalRepository signalLog;
    EmergencyRepository emergencies;

    SimulationState simulation;
    int cycleCount;
    PriorityResult lastDecision;
    bool databaseReady;
    int cycleIntervalMs;
    long long lastTickMs;

    static long long nowMillis();

    std::map<std::string, std::string> previousSignals;

    static RoadState toState(Road* road, int queued);
    void syncRoadToDatabase(const std::string& roadID);
    void syncAllRoadsToDatabase();
    void recordSignalChanges();
    PriorityResult buildDecision();

    
    PriorityResult readCurrentDecision();

public:
    Integration();
    ~Integration();

    Integration(const Integration&) = delete;
    Integration& operator=(const Integration&) = delete;

    OperationResult initialise(const std::string& dataTarget);
    OperationResult loadFromDatabase();     // dashboard open → spec 4.6
    OperationResult seedDemoRoads();
    void shutdown();

    bool isDatabaseReady() const;
    std::string databaseBackend() const;

    OperationResult addRoad(const std::string& roadID,
                            const std::string& roadName,
                            int vehicles, int density, int waiting);
    OperationResult updateRoad(const std::string& roadID,
                               int vehicles, int density, int waiting);
    OperationResult removeRoad(const std::string& roadID);
    OperationResult getRoad(const std::string& roadID, RoadState& out);
    OperationResult getAllRoads(std::vector<RoadState>& out);

    OperationResult updateTraffic(const std::string& roadID,
                                  int addVehicles, int removeVehicles,
                                  int density, int waiting);

    OperationResult startEmergency(const std::string& roadID,
                                   const std::string& type);
    OperationResult endEmergency(const std::string& roadID);

    PriorityResult getCurrentPriority();
    OperationResult runCycle();

    OperationResult simulationStart();
    OperationResult simulationPause();
    OperationResult simulationResume();
    OperationResult simulationReset();
    SimulationState simulationStatus() const;
    int getCycleCount() const;

    void tickIfRunning();

    void setCycleInterval(int milliseconds);
    int getCycleInterval() const;

    TrafficStateSnapshot getSnapshot();

    OperationResult getTrafficHistory(const std::string& roadID, int limit,
                                      std::vector<TrafficRecord>& out);
    OperationResult getSignalHistory(const std::string& roadID, int limit,
                                     std::vector<SignalRecord>& out);
    OperationResult getEmergencyHistory(const std::string& roadID,
                                        std::vector<EmergencyInfo>& out);

   
    int findVehiclePosition(const std::string& roadID,
                            const std::string& vehicleID);

    OperationResult getQueue(const std::string& roadID,
                             std::vector<QueuedVehicle>& out);

    
    OperationResult addVehicleToQueue(const std::string& roadID,
                                      const std::string& vehicleID,
                                      const std::string& type);
};

#endif 