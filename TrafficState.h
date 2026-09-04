#ifndef TRAFFIC_STATE_H
#define TRAFFIC_STATE_H

#include <string>
#include <map>
#include <vector>

using namespace std;


// --------------------
// Vehicle Types
// --------------------

enum class VehicleType
{
    CAR,
    AMBULANCE
};


// --------------------
// Actions
// --------------------

enum class ActionType
{
    ADD_CAR,
    REMOVE_CAR,
    SEND_AMBULANCE
};


// --------------------
// Road Information
// --------------------

struct RoadState
{
    int count = 0;
    int waitTime = 0;

    bool hasEmergency = false;

    string signalState = "red";

    int remaining = 0;
};


// --------------------
// Complete Traffic State
// --------------------

struct TrafficState
{
    map<string, RoadState> roads;

    string activeRoad = "";

    int cyclesRun = 0;
};


// --------------------
// Controller Settings
// --------------------

struct ControllerConfig
{
    int minGreen = 10;
    int maxGreen = 50;

    double densityWeight = 1.0;
    double waitWeight = 0.6;
};


// --------------------
// Traffic Decision
// --------------------

struct Decision
{
    string roadName = "";

    bool isEmergency = false;

    int greenDuration = 0;
};


// --------------------
// Log Information
// --------------------

struct LogEntry
{
    string timestamp = "";

    string message = "";

    string category = "info";
};


// --------------------
// Simulation Statistics
// --------------------

struct SimulationStats
{
    int totalVehicles = 0;

    int cyclesRun = 0;

    string activeRoadLabel = "-";
};


// --------------------
// User Action
// --------------------

struct UserAction
{
    string roadName = "";

    ActionType action = ActionType::ADD_CAR;

    VehicleType vehicleType = VehicleType::CAR;
};


// --------------------
// Road Names
// --------------------

const vector<string> ROAD_NAMES =
{
    "Road A",
    "Road B",
    "Road C",
    "Road D"
};


// ==================================================
// FUNCTION DECLARATIONS
// ==================================================

// Vehicle management
void addVehicle(TrafficState& state, string roadName, VehicleType type);

void removeVehicle(TrafficState& state, string roadName);


// Emergency management
void sendAmbulance(TrafficState& state, string roadName);


// Traffic decision
Decision makeDecision(
    const TrafficState& state,
    const ControllerConfig& config
);


// Signal management
void updateSignals(
    TrafficState& state,
    const Decision& decision
);


// Waiting time
void updateWaitingTime(TrafficState& state);


// Statistics
SimulationStats getStatistics(const TrafficState& state);


// Logging
void addLog(
    vector<LogEntry>& logs,
    string message,
    string category
);


#endif
