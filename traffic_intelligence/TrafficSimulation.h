
#ifndef TRAFFIC_SIMULATION_H
#define TRAFFIC_SIMULATION_H

#include <iostream>
#include <string>

#include "TrafficManager.h"

using namespace std;

class TrafficSimulation {
private:
    TrafficManager manager;

    int simulationTime;
    int nextVehicleID;

    void generateArrival();
    void generateTrafficUpdate();
    void generateEmergencyEvent();

public:
    TrafficSimulation();

    void step();
    void run(int seconds);
    void displaySimulationStatus();
};

#endif
