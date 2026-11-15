
#include <iostream>
#include "../include/TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    // Step 1: Create roads.
    Road road1("R1", "North Road", 20, 60, 10, 0);
    Road road2("R2", "South Road", 40, 80, 15, 1);

    // Step 2: Add roads before using their IDs.
    cout << "Add North Road: "
         << manager.addRoad(road1) << endl;

    cout << "Add South Road: "
         << manager.addRoad(road2) << endl;

    cout << "Total roads: "
         << manager.getRoadCount() << endl;

    // Step 3: Select the highest-priority road.
    int selected = manager.selectNextRoad();

    if (selected != -1) {
        Road* selectedRoad = manager.getRoad("R2");

        if (selectedRoad != nullptr) {
            cout << "\nSelected road: "
                 << selectedRoad->getRoadName() << endl;
        }
    }

    // Step 4: Display road status after selection.
    cout << "\n--- Road Status After Selection ---\n";
    manager.displayAllRoads();

    // Step 5: Test an invalid road lookup.
    cout << "Missing road lookup: "
         << (manager.getRoad("R999") == nullptr)
         << endl;

    // Step 6: Add an emergency vehicle to North Road.
    Vehicle ambulance("V101", "Ambulance", 0, true);

    cout << "\n--- Emergency Vehicle Test ---" << endl;

    bool added = manager.addEmergencyVehicle("R1", ambulance);

    cout << "Emergency vehicle added: "
         << added << endl;

    // Step 7: Check updated road values.
    Road* northRoad = manager.getRoad("R1");

    if (northRoad != nullptr) {
        cout << "Vehicle count: "
             << northRoad->getVehicleCount() << endl;

        cout << "Traffic density: "
             << northRoad->getTrafficDensity() << endl;

        cout << "Waiting time: "
             << northRoad->getWaitingTime() << endl;

        cout << "Emergency count: "
             << northRoad->getEmergencyVehicleCount() << endl;

        cout << "Emergency status: "
             << northRoad->getEmergencyStatus() << endl;

        cout << "Updated priority: "
             << northRoad->getCurrentPriority() << endl;
    }

    return 0;
}
