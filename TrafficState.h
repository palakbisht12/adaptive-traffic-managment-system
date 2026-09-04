#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <string>
#include <vector>

using namespace std;




enum class VehicleType
{
    CAR,
    AMBULANCE
};


class Vehicle
{
private:
    VehicleType type;

public:
    Vehicle(VehicleType type);

    VehicleType getType() const;
    bool isEmergency() const;
};



class Road
{
private:
    string name;
    vector<Vehicle> vehicles;

    int waitTime;
    string signalState;
    int remaining;

public:
    Road(string name);

    void addVehicle(VehicleType type);
    void removeVehicle();

    void increaseWaitingTime();
    void processGreenSignal();

    string getName() const;
    int getVehicleCount() const;
    int getWaitTime() const;

    bool hasEmergency() const;

    string getSignalState() const;
    int getRemaining() const;

    void setSignalState(string state);
    void setRemaining(int time);
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
    string roadName = "";
    bool isEmergency = false;
    int greenDuration = 0;
};



struct LogEntry
{
    string message = "";
    string category = "info";
};



struct SimulationStats
{
    int totalVehicles = 0;
    int cyclesRun = 0;
    string activeRoadLabel = "-";
};



class TrafficController
{
private:
    ControllerConfig config;

public:
    TrafficController();

    Decision makeDecision(const vector<Road>& roads) const;

    void updateSignals(
        vector<Road>& roads,
        const Decision& decision) const;
};



class TrafficSystem
{
private:
    vector<Road> roads;

    TrafficController controller;

    vector<LogEntry> logs;

    string activeRoad;

    int cyclesRun;

public:
    TrafficSystem();

    void addVehicle(string roadName, VehicleType type);
    void removeVehicle(string roadName);
    void sendAmbulance(string roadName);

    
    void refreshSignals();

    // Run one simulation cycle
    void runCycle();

    SimulationStats getStatistics() const;

    void showRoads() const;
    void showLogs() const;
};

#endif
