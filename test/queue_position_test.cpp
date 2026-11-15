
#include <iostream>
#include "../include/queue.h"

using namespace std;

int main() {
    VehicleQueue queue;

    Vehicle car1("C1", "Car", 0, false);
    Vehicle car2("C2", "Car", 0, false);
    Vehicle ambulance("A1", "Ambulance", 0, true);
    Vehicle car3("C3", "Car", 0, false);

    queue.enqueue(car1);
    queue.enqueue(car2);
    queue.enqueue(ambulance);
    queue.enqueue(car3);

    cout << "\nFirst emergency vehicle position: "
         << queue.getFirstEmergencyPosition() << endl;

    return 0;
}
