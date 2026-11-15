
#include <iostream>
#include "../include/TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    // Road A has heavier traffic.
    Road roadA("A", "High Density Road", 50, 90, 0, 0);

    // Road B has waited much longer.
    Road roadB("B", "Long Waiting Road", 10, 20, 100, 0);

    manager.addRoad(roadA);
    manager.addRoad(roadB);

    Vehicle carA("A1", "Car", 0, false);
    Vehicle carB("B1", "Car", 0, false);

    manager.addVehicle("A", carA);
    manager.addVehicle("B", carB);

    cout << "\n===== FAIRNESS PRIORITY TEST =====" << endl;

    manager.processTrafficCycle();

    cout << "\n===== FINAL ROAD STATUS =====" << endl;
    manager.displayAllRoads();

    return 0;
}
