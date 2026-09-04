#ifndef TRAFFIC_STATE_H
#define TRAFFIC_STATE_H

#include <string>
#include <map>
#include <vector>

enum class VehicleType
{
    CAR,
    AMBULANCE
};

enum class ActionType
{
    ADD_CAR,
    REMOVE_CAR,
    SEND_AMBULANCE
};

struct RoadState
{
    int count = 0;
    int waitTime = 0;
    bool hasEmergency = false;
    std::string signalState = "red";
    int remaining = 0;
};

struct TrafficState
{
    std::map<std::string, RoadState> roads;
    std::string activeRoad = "";
    int cyclesRun = 0;
};

struct ControllerConfig
{
    int minGreen = 10;
    int maxGreen = 50;
    double densityWeight = 1.0;
    double waitWeight = 0.6;
};

struct Decision
{
    std::string roadName = "";
    bool isEmergency = false;
    int greenDuration = 0;
};

struct LogEntry
{
    std::string timestamp = "";
    std::string message = "";
    std::string category = "info";
};

struct SimulationStats
{
    int totalVehicles = 0;
    int cyclesRun = 0;
    std::string activeRoadLabel = "-";
};

struct UserAction
{
    std::string roadName = "";
    ActionType action = ActionType::ADD_CAR;
};

inline const std::vector<std::string> ROAD_NAMES =
{
    "Road A",
    "Road B",
    "Road C",
    "Road D"
};

#endif
