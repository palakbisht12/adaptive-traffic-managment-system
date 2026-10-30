
#include <iostream>
#include "../include/TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    Road roadA("A", "High Density Road", 50, 90, 0, 0);
    Road roadB("B", "Long Waiting Road", 10, 20, 100, 0);

    manager.addRoad(roadA);
    manager.addRoad(roadB);

    // Add 10 ordinary vehicles to each road.
    for (int i = 1; i <= 10; i++) {
        Vehicle carA("A" + to_string(i), "Car", 0, false);
        Vehicle carB("B" + to_string(i), "Car", 0, false);

        manager.addVehicle("A", carA);
        manager.addVehicle("B", carB);
    }

    cout << "\n===== MULTI-CYCLE FAIRNESS TEST =====" << endl;

    for (int cycle = 1; cycle <= 10; cycle++) {
        cout << "\n--- Cycle " << cycle << " ---" << endl;
        manager.processTrafficCycle();
    }

    cout << "\n===== FINAL ROAD STATUS =====" << endl;
    manager.displayAllRoads();

    return 0;
}
