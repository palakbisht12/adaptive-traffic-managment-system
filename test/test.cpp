
#include <iostream>
#include <string>

#include "road.h"
#include "vehicle.h"
#include "queue.h"
#include "heap.h"
#include "hashtable.h"
#include "sorting.h"
#include "trafficintelligence.h"

using namespace std;

int testsRun = 0;
int testsPassed = 0;

void check(const string& name, bool condition) {
    testsRun++;

    if (condition) {
        testsPassed++;
        cout << "  PASS: " << name << endl;
    } else {
        cout << "  FAIL: " << name << endl;
    }
}

// TEST ROAD
void testRoad() {
    cout << "\n===== ROAD TESTS =====" << endl;

    Road road("R1", "Main Road", 50, 80, 10, 2);

    check("Constructor stores road information",
          road.getRoadID() == "R1" &&
          road.getVehicleCount() == 50 &&
          road.getTrafficDensity() == 80 &&
          road.getWaitingTime() == 10);

    check("Emergency vehicles activate emergency status",
          road.getEmergencyStatus());

    road.setVehicleCount(60);

    check("Vehicle count updates correctly",
          road.getVehicleCount() == 60);

    road.setSignalStatus("GREEN");

    check("Valid signal accepted",
          road.getSignalStatus() == "GREEN");

    road.setSignalStatus("PURPLE");

    check("Invalid signal changes to RED",
          road.getSignalStatus() == "RED");

    road.setVehicleCount(-5);

    check("Negative vehicle count becomes zero",
          road.getVehicleCount() == 0);

    road.setTrafficDensity(150);

    check("Traffic density is capped at 100",
          road.getTrafficDensity() == 100);

    road.setEmergencyVehicleCount(0);

    check("Zero emergency vehicles clears emergency status",
          !road.getEmergencyStatus());

    road.setSignalStatus("GREEN");
    road.deactivateRoad();

    check("Deactivated road has RED signal",
          !road.getActiveStatus() &&
          road.getSignalStatus() == "RED");
}

// TEST VEHICLE
void testVehicle() {
    cout << "\n===== VEHICLE TESTS =====" << endl;

    Vehicle car("V1", "Car", 5, false);

    check("Normal car is not an emergency vehicle",
          !car.getIsEmergency() &&
          car.getVehicleType() == "Car");

    Vehicle ambulance("V2", "Ambulance", 6, false);

    check("Ambulance is detected as an emergency",
          ambulance.getIsEmergency());

    car.setArrivalTime(-3);

    check("Negative arrival time becomes zero",
          car.getArrivalTime() == 0);

    car.setVehicleType("Fire Brigade");

    check("Fire Brigade is detected as an emergency",
          car.getIsEmergency());

    Vehicle emergency("V3", "Car", 7, true);

    check("Emergency flag is respected",
          emergency.getIsEmergency());
}

// TEST QUEUE
void testQueue() {
    cout << "\n===== QUEUE TESTS =====" << endl;

    VehicleQueue q;

    check("New queue is empty",
          q.isEmpty() && q.size() == 0);

    q.enqueue(Vehicle("V1", "Car", 1, false));
    q.enqueue(Vehicle("V2", "Bus", 2, false));
    q.enqueue(Vehicle("V3", "Car", 3, false));

    check("Queue size after three insertions",
          q.size() == 3);

    Vehicle* first = q.front();

    check("Queue follows FIFO order",
          first != nullptr &&
          first->getVehicleID() == "V1");

    q.dequeue();

    first = q.front();

    check("Front advances after removal",
          first != nullptr &&
          first->getVehicleID() == "V2" &&
          q.size() == 2);

    q.dequeue();
    q.dequeue();

    check("Queue becomes empty after removals",
          q.isEmpty() &&
          q.front() == nullptr);

    check("Removing from an empty queue returns false",
          q.dequeue() == false);
}

// TEST HEAP
void testHeap() {
    cout << "\n===== HEAP TESTS =====" << endl;

    RoadHeap heap;

    check("New heap is empty",
          heap.isEmpty() &&
          heap.getHighestPriorityRoad() == nullptr);

    Road a("R1", "Road A", 0, 0, 0, 0);
    Road b("R2", "Road B", 0, 0, 0, 0);
    Road c("R3", "Road C", 0, 0, 0, 0);

    a.setCurrentPriority(5);
    b.setCurrentPriority(9);
    c.setCurrentPriority(7);

    heap.insertRoad(a);
    heap.insertRoad(b);
    heap.insertRoad(c);

    check("Heap size after insertion",
          heap.size() == 3);

    Road* top = heap.getHighestPriorityRoad();

    check("Highest priority is at the top",
          top != nullptr &&
          top->getRoadID() == "R2");

    Road removed;

    heap.removeHighestPriorityRoad(removed);

    check("First removal has priority 9",
          removed.getCurrentPriority() == 9);

    heap.removeHighestPriorityRoad(removed);

    check("Second removal has priority 7",
          removed.getCurrentPriority() == 7);

    heap.removeHighestPriorityRoad(removed);

    check("Third removal has priority 5",
          removed.getCurrentPriority() == 5);

    check("Heap is empty after all removals",
          heap.isEmpty());
}

// TEST HASH TABLE
void testHashTable() {
    cout << "\n===== HASH TABLE TESTS =====" << endl;

    RoadHashTable table;

    table.insertRoad(
        Road("R101", "Rajpur Road", 10, 20, 5, 0));

    table.insertRoad(
        Road("R102", "Saharanpur Road", 15, 25, 6, 0));

    check("Hash table size after two insertions",
          table.size() == 2);

    Road* road = table.searchRoad("R101");

    check("Search finds an existing road",
          road != nullptr &&
          road->getRoadName() == "Rajpur Road");

    check("Search returns null for a missing road",
          table.searchRoad("R999") == nullptr);

    // Inserting the same ID should update the existing road.
    table.insertRoad(
        Road("R101", "Rajpur Road Extension", 11, 21, 5, 0));

    road = table.searchRoad("R101");

    check("Duplicate ID updates instead of adding another entry",
          table.size() == 2 &&
          road != nullptr &&
          road->getRoadName() == "Rajpur Road Extension");

    check("Existing road can be deleted",
          table.deleteRoad("R101"));

    check("Deleted road cannot be found",
          !table.contains("R101") &&
          table.size() == 1);

    check("Deleting a missing road returns false",
          !table.deleteRoad("R101"));

    table.insertRoad(
        Road("R110", "Road One Ten", 1, 1, 1, 0));

    table.insertRoad(
        Road("R101", "Road One Oh One", 1, 1, 1, 0));

    check("Colliding keys remain searchable",
          table.contains("R110") &&
          table.contains("R101"));
}

// TEST SORTING
void testSorting() {
    cout << "\n===== SORTING TESTS =====" << endl;

    Road roads[4];

    int priorities[4] = {3, 10, 7, 1};

    for (int i = 0; i < 4; i++) {
        roads[i].setRoadID("R" + to_string(i));
        roads[i].setCurrentPriority(priorities[i]);
    }

    RoadSorting sorter;

    sorter.sortRoads(roads, 4);

    bool descending = true;

    for (int i = 0; i < 3; i++) {
        if (roads[i].getCurrentPriority() <
            roads[i + 1].getCurrentPriority()) {
            descending = false;
        }
    }

    check("Roads sorted by descending priority",
          descending);

    check("Highest priority appears first",
          roads[0].getCurrentPriority() == 10);

    Road empty[1];

    sorter.sortRoads(empty, 0);
    sorter.sortRoads(empty, 1);

    check("Sorting zero or one element does not crash",
          true);
}

// TEST TRAFFIC INTELLIGENCE
void testTrafficIntelligence() {
    cout << "\n===== TRAFFIC INTELLIGENCE TESTS =====" << endl;

    TrafficIntelligence system;

    system.addRoad(
        Road("R1", "Low Traffic", 10, 20, 5, 0));

    system.addRoad(
        Road("R2", "Heavy Traffic", 90, 95, 40, 0));

    system.addRoad(
        Road("R3", "Ambulance Road", 5, 10, 2, 1));

    system.calculatePriority();

    // FIX 2: these numbers used to be 45 / 315 / 122, which came from the
    // old inline formula that existed before PriorityManager. They are
    // recomputed here from PriorityManager's actual weights:
    //   emergency 1000, vehicles x2, density x1, waiting x2
    //   R1 = 10*2 + 20*1 +  5*2          =   50
    //   R2 = 90*2 + 95*1 + 40*2          =  355
    //   R3 = 1000 + 5*2 + 10*1 + 2*2     = 1024
    check("Priority calculation includes all four factors",
          system.getRoad(0)->getCurrentPriority() == 50 &&
          system.getRoad(1)->getCurrentPriority() == 355 &&
          system.getRoad(2)->getCurrentPriority() == 1024);

    // The single most important assertion in the project. An ambulance on
    // the quietest road must outrank the heaviest jam. The old weights
    // (emergency 100) failed this, which is why they were wrong.
    check("Ambulance road outranks the heaviest jam",
          system.getRoad(2)->getCurrentPriority() >
          system.getRoad(1)->getCurrentPriority());

    system.updateSignals();

    int greenCount = 0;

    for (int i = 0; i < system.getRoadCount(); i++) {
        if (system.getRoad(i)->getSignalStatus() == "GREEN") {
            greenCount++;
        }
    }

    check("Exactly one road has a GREEN signal",
          greenCount == 1);

    // Deactivate a road and ensure it is not selected.
    system.getRoad(1)->deactivateRoad();

    system.updateSignals();

    check("Inactive road has zero priority and RED signal",
          system.getRoad(1)->getCurrentPriority() == 0 &&
          system.getRoad(1)->getSignalStatus() == "RED");

    check("selectGreenRoad returns a valid index",
          system.selectGreenRoad() >= 0);
}

// RUN ALL TESTS
int main() {
    cout << "ADAPTIVE TRAFFIC MANAGEMENT SYSTEM - UNIT TESTS"
         << endl;

    testRoad();
    testVehicle();
    testQueue();
    testHeap();
    testHashTable();
    testSorting();
    testTrafficIntelligence();

    cout << "\n================================" << endl;

    cout << testsPassed << " / "
         << testsRun << " tests passed." << endl;

    if (testsPassed == testsRun) {
        cout << "All tests passed!" << endl;
        return 0;
    }

    cout << "Some tests failed. Review the FAIL messages." << endl;
    return 1;
}
