
#include <iostream>
#include <cassert>
#include "TrafficManager.h"

using namespace std;

int main() {
    TrafficManager manager;

    Road road("A", "Road A", 0, 50, 0, 0);
    assert(manager.addRoad(road));

    assert(manager.addVehicle(
        "A", Vehicle("A1", "Car", 0, false)
    ));

    assert(manager.addVehicle(
        "A", Vehicle("A2", "Car", 0, false)
    ));

    assert(manager.addEmergencyVehicle(
        "A", Vehicle("AMB-A", "Ambulance", 0, true)
    ));

    Road* before = manager.getRoad("A");
    assert(before != nullptr);
    assert(before->getVehicleCount() == 3);
    assert(before->getEmergencyVehicleCount() == 1);

    // Should remove the emergency vehicle first.
    assert(manager.removeVehicle("A"));

    Road* after = manager.getRoad("A");
    assert(after != nullptr);
    assert(after->getVehicleCount() == 2);
    assert(after->getEmergencyVehicleCount() == 0);
    assert(!after->getEmergencyStatus());

    cout << "PASS: TrafficManager removes emergency vehicle "
         << "and updates road counts." << endl;

    return 0;
}
