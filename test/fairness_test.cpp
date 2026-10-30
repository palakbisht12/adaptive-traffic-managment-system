
#include <iostream>
#include <string>
#include "TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    // Create two roads with different traffic densities.
    Road northRoad("R1", "North Road", 0, 20, 0, 0);
    Road southRoad("R2", "South Road", 0, 80, 0, 0);

    manager.addRoad(northRoad);
    manager.addRoad(southRoad);

    int northSelected = 0;
    int southSelected = 0;
    int vehiclesServed = 0;

    const int TOTAL_CYCLES = 100;

    for (int cycle = 1; cycle <= TOTAL_CYCLES; cycle++) {

        // Add one new vehicle to each road.
        string northID = "N" + to_string(cycle);
        string southID = "S" + to_string(cycle);

        Vehicle northCar(northID, "Car", 0, false);
        Vehicle southCar(southID, "Car", 0, false);

        manager.addVehicle("R1", northCar);
        manager.addVehicle("R2", southCar);

        cout << "\n===== Cycle " << cycle << " =====" << endl;

        // Process one traffic cycle.
        manager.processTrafficCycle();

        // Check which road received the green signal.
        Road* north = manager.getRoad("R1");
        Road* south = manager.getRoad("R2");

        if (north != nullptr &&
            north->getSignalStatus() == "GREEN") {
            northSelected++;
        }
        else if (south != nullptr &&
                 south->getSignalStatus() == "GREEN") {
            southSelected++;
        }

        // Count the number of vehicles served in this cycle.
        // The current implementation serves one vehicle
        // whenever a road is selected.
        if ((north != nullptr &&
             north->getSignalStatus() == "GREEN") ||
            (south != nullptr &&
             south->getSignalStatus() == "GREEN")) {
            vehiclesServed++;
        }

        cout << "North queue: "
             << north->getVehicleCount() << endl;

        cout << "South queue: "
             << south->getVehicleCount() << endl;
    }

    // Display final results.
    cout << "\n==================================" << endl;
    cout << "       FAIRNESS TEST RESULTS      " << endl;
    cout << "==================================" << endl;

    cout << "Total cycles: " << TOTAL_CYCLES << endl;
    cout << "North selected: " << northSelected << endl;
    cout << "South selected: " << southSelected << endl;
    cout << "Total vehicles served: "
         << vehiclesServed << endl;

    cout << "==================================" << endl;

    return 0;
}
