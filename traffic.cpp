#include "traffic.h"

using namespace std;


// Add a vehicle
void addVehicle(TrafficState& state, string roadName, VehicleType type)
{
    state.roads[roadName].count++;

    if (type == VehicleType::AMBULANCE)
    {
        state.roads[roadName].hasEmergency = true;
    }
}


// Remove a vehicle
void removeVehicle(TrafficState& state, string roadName)
{
    if (state.roads[roadName].count > 0)
    {
        state.roads[roadName].count--;
    }
}


// Send ambulance
void sendAmbulance(TrafficState& state, string roadName)
{
    state.roads[roadName].hasEmergency = true;
}


// Decide which road should get green
Decision makeDecision(
    const TrafficState& state,
    const ControllerConfig& config)
{
    Decision decision;

    int highestScore = -1;

    for (string road : ROAD_NAMES)
    {
        RoadState currentRoad = state.roads.at(road);

        // Emergency gets highest priority
        if (currentRoad.hasEmergency)
        {
            decision.roadName = road;
            decision.isEmergency = true;
            decision.greenDuration = config.maxGreen;

            return decision;
        }

        // Simple traffic score
        int score = currentRoad.count + currentRoad.waitTime;

        if (score > highestScore)
        {
            highestScore = score;
            decision.roadName = road;
        }
    }

    if (decision.roadName != "")
    {
        RoadState selectedRoad =
            state.roads.at(decision.roadName);

        decision.greenDuration =
            config.minGreen + selectedRoad.count;

        if (decision.greenDuration > config.maxGreen)
        {
            decision.greenDuration = config.maxGreen;
        }
    }

    return decision;
}


// Update traffic signals
void updateSignals(
    TrafficState& state,
    const Decision& decision)
{
    for (string road : ROAD_NAMES)
    {
        if (road == decision.roadName)
        {
            state.roads[road].signalState = "green";
            state.roads[road].remaining =
                decision.greenDuration;

            state.activeRoad = road;
        }
        else
        {
            state.roads[road].signalState = "red";
            state.roads[road].remaining = 0;
        }
    }
}


// Update waiting time
void updateWaitingTime(TrafficState& state)
{
    for (string road : ROAD_NAMES)
    {
        if (road != state.activeRoad)
        {
            if (state.roads[road].count > 0)
            {
                state.roads[road].waitTime += 10;
            }
        }
        else
        {
            // Vehicles on green road get a chance to move
            if (state.roads[road].count > 0)
            {
                state.roads[road].count--;

                if (state.roads[road].waitTime >= 10)
                {
                    state.roads[road].waitTime -= 10;
                }
            }
        }
    }
}


// Get statistics
SimulationStats getStatistics(const TrafficState& state)
{
    SimulationStats stats;

    stats.cyclesRun = state.cyclesRun;
    stats.activeRoadLabel = state.activeRoad;

    for (string road : ROAD_NAMES)
    {
        stats.totalVehicles += state.roads.at(road).count;
    }

    return stats;
}


// Add log
void addLog(
    vector<LogEntry>& logs,
    string message,
    string category)
{
    LogEntry entry;

    entry.message = message;
    entry.category = category;

    logs.push_back(entry);
}
