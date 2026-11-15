
#include <iostream>
#include "../include/TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    // Create two roads with different traffic conditions.
    Road northRoad("R1", "North Road", 0, 20, 0, 0);
    Road southRoad("R2", "South Road", 10, 80, 0, 0);

    manager.addRoad(northRoad);
    manager.addRoad(southRoad);

    // Add an ambulance to North Road.
    Vehicle ambulance("AMB101", "Ambulance", 0, true);
    manager.addEmergencyVehicle("R1", ambulance);

    // Add an ordinary car to South Road.
    Vehicle car("CAR101", "Car", 0, false);
    manager.addVehicle("R2", car);

    cout << "\n===== BEFORE TRAFFIC CYCLE =====" << endl;
    manager.displayAllRoads();

    cout << "\n===== PROCESSING TRAFFIC =====" << endl;
    manager.processTrafficCycle();

    cout << "\n===== AFTER TRAFFIC CYCLE =====" << endl;
    manager.displayAllRoads();

    return 0;
}
