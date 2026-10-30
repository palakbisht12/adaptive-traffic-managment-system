
#include "TrafficManager.h"

int main() {
    TrafficManager manager;

    Road roadA("A", "Normal Road", 20, 40, 0, 0);
    Road roadB("B", "Emergency Road", 10, 20, 0, 0);

    manager.addRoad(roadA);
    manager.addRoad(roadB);

    // Road A has normal vehicles.
    Vehicle car1("A1", "Car", 0, false);
    Vehicle car2("A2", "Car", 0, false);
    manager.addVehicle("A", car1);
    manager.addVehicle("A", car2);

    // Road B has a normal vehicle followed by two ambulances.
    Vehicle car3("B1", "Car", 0, false);
    Vehicle ambulance1("AMB1", "Ambulance", 0, true);
    Vehicle ambulance2("AMB2", "Ambulance", 0, true);

    manager.addVehicle("B", car3);
    manager.addVehicle("B", ambulance1);
    manager.addVehicle("B", ambulance2);

    cout << "\n===== MULTIPLE EMERGENCY TEST =====" << endl;

    manager.processTrafficCycle();

    cout << "\n===== FINAL ROAD STATUS =====" << endl;
    manager.displayAllRoads();

    return 0;
}
