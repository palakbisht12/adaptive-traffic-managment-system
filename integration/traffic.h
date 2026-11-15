#ifndef TRAFFIC_H
#define TRAFFIC_H



#include <string>
#include <vector>


struct RoadState {
    std::string roadID;
    std::string roadName;
    int vehicleCount = 0;
    int trafficDensity = 0;     
    int waitingTime = 0;          
    int emergencyVehicleCount = 0;
    bool emergencyStatus = false;
    int currentPriority = 0;
    std::string signalStatus = "RED";   
    bool activeStatus = true;
    int queuedVehicles = 0;        
};


struct PriorityResult {
    bool valid = false;
    std::string roadID;
    std::string roadName;
    int priority = 0;
    std::string signalStatus = "RED";
    std::string reason;
    std::string errorMessage;
};


struct QueuedVehicle {
    int position = 0;           
    std::string vehicleID;
    std::string vehicleType;
    bool emergency = false;
    int arrivalTime = 0;
};


struct SignalState {
    std::string roadID;
    std::string signalStatus = "RED";
};


struct EmergencyInfo {
    int eventID = 0;
    std::string roadID;
    std::string emergencyType = "Ambulance";
    std::string startTime;
    std::string endTime;
    std::string status = "ACTIVE";     
};


struct TrafficRecord {
    int recordID = 0;
    std::string roadID;
    int vehicleCount = 0;
    int trafficDensity = 0;
    int waitingTime = 0;
    std::string timestamp;
};

struct SignalRecord {
    int recordID = 0;
    std::string roadID;
    std::string previousSignal;
    std::string newSignal;
    std::string timestamp;
};


enum class SimulationState { STOPPED, RUNNING, PAUSED };

std::string simulationStateToString(SimulationState state);

struct OperationResult {
    bool ok = false;
    std::string message;
    int httpStatus = 200;      

    static OperationResult success(const std::string& text = "OK") {
        OperationResult r;
        r.ok = true;
        r.message = text;
        r.httpStatus = 200;
        return r;
    }

    static OperationResult failure(const std::string& text, int status = 400) {
        OperationResult r;
        r.ok = false;
        r.message = text;
        r.httpStatus = status;
        return r;
    }
};

struct TrafficStateSnapshot {
    std::vector<RoadState> roads;
    PriorityResult current;
    SimulationState simulation = SimulationState::STOPPED;
    int cycle = 0;
};

#endif 
