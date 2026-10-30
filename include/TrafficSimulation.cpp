
#include "TrafficSimulation.h"

#include <iostream>
#include <string>

using namespace std;

// Constructor
TrafficSimulation::TrafficSimulation() {
    simulationTime = 0;
    nextVehicleID = 1;

    manager.addRoad(
        Road("R1", "North Road", 0, 0, 0, 0)
    );

    manager.addRoad(
        Road("R2", "South Road", 0, 0, 0, 0)
    );

    manager.addRoad(
        Road("R3", "East Road", 0, 0, 0, 0)
    );

    manager.addRoad(
        Road("R4", "West Road", 0, 0, 0, 0)
    );
}

// Generate a normal vehicle.
void TrafficSimulation::generateArrival() {
    int roadCount = manager.getRoadCount();

    if (roadCount == 0) {
        return;
    }

    int index = simulationTime % roadCount;
    Road* road = manager.getRoad(index);

    if (road == nullptr || !road->getActiveStatus()) {
        return;
    }

    string id = "V" + to_string(nextVehicleID);
    Vehicle vehicle(id, "Normal", simulationTime, false);

    if (manager.addVehicle(road->getRoadID(), vehicle)) {
        cout << "Vehicle " << id
             << " arrived at "
             << road->getRoadName() << endl;

        nextVehicleID++;
    }
}

// Update traffic density using the manager's vehicle counts.
void TrafficSimulation::generateTrafficUpdate() {
    for (int i = 0; i < manager.getRoadCount(); i++) {
        Road* road = manager.getRoad(i);

        if (road == nullptr) {
            continue;
        }

        int vehicles = road->getVehicleCount();
        int density = vehicles * 10;

        if (density > 100) {
            density = 100;
        }

        manager.updateTrafficDensity(
            road->getRoadID(),
            density
        );
    }

    cout << "Traffic information updated." << endl;
}

// Generate an emergency vehicle.
void TrafficSimulation::generateEmergencyEvent() {
    int roadCount = manager.getRoadCount();

    if (roadCount == 0) {
        return;
    }

    int index = simulationTime % roadCount;
    Road* road = manager.getRoad(index);

    if (road == nullptr || !road->getActiveStatus()) {
        return;
    }

    string id = "V" + to_string(nextVehicleID);
    Vehicle vehicle(id, "Emergency", simulationTime, true);

    if (manager.addEmergencyVehicle(road->getRoadID(), vehicle)) {
        cout << "Emergency vehicle " << id
             << " arrived at "
             << road->getRoadName() << endl;

        nextVehicleID++;
    }
}

// Execute one simulation step.
void TrafficSimulation::step() {
    cout << "\n===== TIME: "
         << simulationTime << " =====" << endl;

    generateArrival();

    if (simulationTime > 0 && simulationTime % 5 == 0) {
        generateEmergencyEvent();
    }

    generateTrafficUpdate();

    // The manager selects a road and serves one queued vehicle.
    manager.processTrafficCycle();

    simulationTime++;
}

// Run the simulation.
void TrafficSimulation::run(int seconds) {
    if (seconds <= 0) {
        cout << "Simulation duration must be greater than zero."
             << endl;
        return;
    }

    for (int i = 0; i < seconds; i++) {
        step();
    }

    cout << "\nSimulation completed." << endl;

    displaySimulationStatus();
}

// Display final road and queue information.
void TrafficSimulation::displaySimulationStatus() {
    cout << "\n===== FINAL ROAD STATUS =====" << endl;
    manager.displayAllRoads();
}
