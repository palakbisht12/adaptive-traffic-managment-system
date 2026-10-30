
#include <iostream>
#include "../include/TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    manager.addRoad(Road("A", "Road A", 1, 20, 0, 0));
    manager.addRoad(Road("B", "Road B", 1, 30, 0, 0));
    manager.addRoad(Road("C", "Road C", 1, 40, 0, 0));

    // Test duplicate road ID.
    cout << "\nTesting duplicate road ID:\n";

    bool added = manager.addRoad(
        Road("A", "Duplicate Road A", 5, 50, 0, 0)
    );

    cout << "Duplicate road added: "
         << (added ? "Yes" : "No") << endl;

    cout << "Road count: "
         << manager.getRoadCount() << endl;

    // Add one vehicle to each road.
    manager.addVehicle("A", Vehicle("A1", "Car", 0, false));
    manager.addVehicle("B", Vehicle("B1", "Car", 0, false));
    manager.addVehicle("C", Vehicle("C1", "Car", 0, false));

    cout << "\nBefore removing Road B:\n";
    manager.displayAllRoads();

    // Remove Road B.
    manager.removeRoad("B");

    cout << "\nAfter removing Road B:\n";
    manager.displayAllRoads();

    // Test removal of a nonexistent road.
    cout << "\nTesting removal of a nonexistent road:\n";

    bool removed = manager.removeRoad("Z");

    cout << "Removal successful: "
         << (removed ? "Yes" : "No") << endl;

    cout << "Road count: "
         << manager.getRoadCount() << endl;

    cout << "\nProcessing one traffic cycle:\n";
    manager.processTrafficCycle();

    cout << "\nFinal status:\n";
    manager.displayAllRoads();

    return 0;
}
