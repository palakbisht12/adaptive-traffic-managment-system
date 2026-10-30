
#include <iostream>
#include <cassert>
#include "queue.h"

using namespace std;

int main() {
    VehicleQueue queue;

    queue.enqueue(Vehicle("A1", "Car", 0, false));
    queue.enqueue(Vehicle("A2", "Car", 0, false));
    queue.enqueue(Vehicle("AMB-A", "Ambulance", 0, true));
    queue.enqueue(Vehicle("A3", "Car", 0, false));

    Vehicle served;

    // The ambulance should be served first.
    assert(queue.serveEmergencyVehicle(served));
    assert(served.getVehicleID() == "AMB-A");
    assert(queue.size() == 3);

    // Normal vehicles should retain their original order.
    assert(queue.serveVehicle(served));
    assert(served.getVehicleID() == "A1");

    assert(queue.serveVehicle(served));
    assert(served.getVehicleID() == "A2");

    assert(queue.serveVehicle(served));
    assert(served.getVehicleID() == "A3");

    assert(queue.isEmpty());

    cout << "PASS: Emergency priority and normal FIFO order." << endl;

    return 0;
}
