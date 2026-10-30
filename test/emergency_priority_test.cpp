
#include "TrafficManager.h"

int main() {
    TrafficManager manager;

    Road roadA("A", "Normal Traffic Road", 20, 40, 0, 0);
    Road roadB("B", "Emergency Vehicle Road", 10, 20, 0, 0);

    manager.addRoad(roadA);
    manager.addRoad(roadB);

    // Add normal vehicles to Road A.
    for (int i = 1; i <= 3; i++) {
        Vehicle car("A" + to_string(i), "Car", 0, false);
        manager.addVehicle("A", car);
    }

    // Add an emergency vehicle to Road B.
    Vehicle ambulance("AMB1", "Ambulance", 0, true);
    manager.addVehicle("B", ambulance);

    cout << "\n===== EMERGENCY PRIORITY TEST =====" << endl;

    manager.processTrafficCycle();

    cout << "\n===== FINAL ROAD STATUS =====" << endl;
    manager.displayAllRoads();

    return 0;
}
