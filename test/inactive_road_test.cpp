
#include <iostream>
#include "TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    Road roadA("A", "Inactive Road", 20, 80, 10, 0);
    Road roadB("B", "Active Road", 30, 50, 5, 0);

    manager.addRoad(roadA);
    manager.addRoad(roadB);

    Vehicle ambulance("AMB-A", "Ambulance", 0, true);
    Vehicle car("B1", "Car", 0, false);

    manager.addVehicle("A", ambulance);
    manager.addVehicle("B", car);

    // Make Road A inactive after adding its ambulance.
    Road* road = manager.getRoad("A");
    if (road != nullptr) {
        road->setActiveStatus(false);
    }

    cout << "\n===== INACTIVE ROAD TEST =====" << endl;

    manager.processTrafficCycle();

    cout << "\n===== FINAL ROAD STATUS =====" << endl;
    manager.displayAllRoads();

    return 0;
}
