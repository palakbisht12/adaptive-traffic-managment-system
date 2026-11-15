
#include <iostream>
#include "TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    Road roadA("A", "Road A", 100, 80, 0, 0);
    Road roadB("B", "Road B", 100, 80, 0, 0);

    manager.addRoad(roadA);
    manager.addRoad(roadB);

    // Road A: emergency vehicle at position 3.
    manager.addVehicle("A", Vehicle("A1", "Car", 0, false));
    manager.addVehicle("A", Vehicle("A2", "Car", 0, false));
    manager.addEmergencyVehicle(
        "A", Vehicle("AMB-A", "Ambulance", 0, true)
    );

    // Road B: emergency vehicle at position 5.
    manager.addVehicle("B", Vehicle("B1", "Car", 0, false));
    manager.addVehicle("B", Vehicle("B2", "Car", 0, false));
    manager.addVehicle("B", Vehicle("B3", "Car", 0, false));
    manager.addVehicle("B", Vehicle("B4", "Car", 0, false));
    manager.addEmergencyVehicle(
        "B", Vehicle("AMB-B", "Ambulance", 0, true)
    );

    cout << "\n===== INITIAL QUEUES =====" << endl;
    manager.displayAllRoads();

    cout << "\n===== TRAFFIC CYCLE =====" << endl;
    cout << "\n===== MULTIPLE TRAFFIC CYCLES =====" << endl;

for (int cycle = 1; cycle <= 4; cycle++) {
    cout << "\n--- Cycle " << cycle << " ---" << endl;
    manager.processTrafficCycle();
}

    cout << "\n===== RESULTS =====" << endl;
    manager.displayAllRoads();

    return 0;
}
