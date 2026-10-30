
#include <iostream>
#include "../include/TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    Road roadA("A", "Road A", 20, 80, 10, 0);
    Road roadB("B", "Road B", 50, 40, 5, 0);

    manager.addRoad(roadA);
    manager.addRoad(roadB);

    Vehicle carA("A1", "Car", 0, false);
    Vehicle carB("B1", "Car", 0, false);

    manager.addVehicle("A", carA);
    manager.addVehicle("B", carB);

    cout << "\n===== NORMAL PRIORITY TEST =====" << endl;

    manager.processTrafficCycle();

    cout << "\n===== FINAL ROAD STATUS =====" << endl;
    manager.displayAllRoads();

    return 0;
}
