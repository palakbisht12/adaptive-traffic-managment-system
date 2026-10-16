

#include <iostream>
#include <string>
#include "road.h"
#include "vehicle.h"
#include "queue.h"
#include "heap.h"
#include "hashtable.h"
#include "sorting.h"
#include "trafficintelligent.h"

using namespace std;

int testsRun = 0;
int testsPassed = 0;

void check(const string& name, bool condition) {
    testsRun++;

    if (condition) {
        testsPassed++;
        cout << "  PASS  " << name << endl;
    } else {
        cout << "  FAIL  " << name << endl;
    }
}

void testRoad() {
    cout << "\nRoad" << endl;

    Road road("R1", "Main Road", 50, 80, 10, 2);

    check("constructor stores fields",
          road.getRoadID() == "R1" &&
          road.getVehicleCount() == 50 &&
          road.getTrafficDensity() == 80 &&
          road.getWaitingTime() == 10);

    check("emergency vehicles set emergency status",
          road.getEmergencyStatus() == true);

    road.setVehicleCount(60);
    check("vehicle count update", road.getVehicleCount() == 60);

    road.setSignalStatus("GREEN");
    check("valid signal accepted", road.getSignalStatus() == "GREEN");

    road.setSignalStatus("PURPLE");
    check("invalid signal falls back to RED",
          road.getSignalStatus() == "RED");

    // Boundary / invalid input
    road.setVehicleCount(-5);
    check("negative vehicle count clamped to 0",
          road.getVehicleCount() == 0);

    road.setTrafficDensity(150);
    check("density above 100 rejected",
          road.getTrafficDensity() == 0);

    road.setEmergencyVehicleCount(0);
    check("zero emergency vehicles clears status",
          road.getEmergencyStatus() == false);

    road.setSignalStatus("GREEN");
    road.deactivateRoad();
    check("deactivating forces RED",
          road.getActiveStatus() == false &&
          road.getSignalStatus() == "RED");
}

void testVehicle() {
    cout << "\nVehicle" << endl;

    Vehicle car("V1", "Car", 5, false);
    check("normal vehicle is not emergency",
          car.getIsEmergency() == false &&
          car.getVehicleType() == "Car");

    Vehicle amb("V2", "Ambulance", 6, false);
    check("ambulance is detected as emergency",
          amb.getIsEmergency() == true);

    car.setArrivalTime(-3);
    check("negative arrival time clamped", car.getArrivalTime() == 0);

    car.setVehicleType("Fire Brigade");
    check("setVehicleType keeps emergency types consistent",
          car.getIsEmergency() == true);
}

void testQueue() {
    cout << "\nQueue (FIFO)" << endl;

    VehicleQueue q;
    check("new queue is empty", q.isEmpty() && q.size() == 0);

    q.enqueue(Vehicle("V1", "Car", 1, false));
    q.enqueue(Vehicle("V2", "Bus", 2, false));
    q.enqueue(Vehicle("V3", "Car", 3, false));

    check("size after 3 enqueues", q.size() == 3);

    Vehicle* f = q.front();
    check("front is the first inserted (FIFO)",
          f != nullptr && f->getVehicleID() == "V1");

    q.dequeue();
    f = q.front();
    check("front advances after dequeue",
          f != nullptr && f->getVehicleID() == "V2" && q.size() == 2);

    q.dequeue();
    q.dequeue();
    check("queue empty after draining",
          q.isEmpty() && q.front() == nullptr);

    check("dequeue on empty queue returns false",
          q.dequeue() == false);
}

void testHeap() {
    cout << "\nMax-Heap" << endl;

    RoadHeap heap;
    check("new heap is empty",
          heap.isEmpty() && heap.getHighestPriorityRoad() == nullptr);

    Road a("R1", "A", 0, 0, 0, 0); a.setCurrentPriority(5);
    Road b("R2", "B", 0, 0, 0, 0); b.setCurrentPriority(9);
    Road c("R3", "C", 0, 0, 0, 0); c.setCurrentPriority(7);

    heap.insertRoad(a);
    heap.insertRoad(b);
    heap.insertRoad(c);

    check("size after inserts", heap.size() == 3);

    Road* top = heap.getHighestPriorityRoad();
    check("peek returns maximum priority",
          top != nullptr && top->getRoadID() == "R2");

    // Removing must yield strictly decreasing priorities.
    Road out;
    heap.removeHighestPriorityRoad(out);
    check("first removal is priority 9", out.getCurrentPriority() == 9);

    heap.removeHighestPriorityRoad(out);
    check("second removal is priority 7", out.getCurrentPriority() == 7);

    heap.removeHighestPriorityRoad(out);
    check("third removal is priority 5", out.getCurrentPriority() == 5);

    check("heap empty after all removals", heap.isEmpty());
}

void testHashTable() {
    cout << "\nHash Table" << endl;

    RoadHashTable table;

    table.insertRoad(Road("R101", "Rajpur Road", 10, 20, 5, 0));
    table.insertRoad(Road("R102", "Saharanpur Road", 15, 25, 6, 0));

    check("size after 2 inserts", table.size() == 2);

    Road* r = table.searchRoad("R101");
    check("search finds existing key",
          r != nullptr && r->getRoadName() == "Rajpur Road");

    check("search misses absent key",
          table.searchRoad("R999") == nullptr);

    // Same key again must update, not duplicate.
    table.insertRoad(Road("R101", "Rajpur Road Extension", 11, 21, 5, 0));
    r = table.searchRoad("R101");
    check("re-insert updates instead of duplicating",
          table.size() == 2 && r->getRoadName() == "Rajpur Road Extension");

    check("delete existing key", table.deleteRoad("R101") == true);
    check("deleted key is gone",
          table.contains("R101") == false && table.size() == 1);
    check("delete absent key returns false",
          table.deleteRoad("R101") == false);

    // Collision handling: these two IDs have the same character sum,
    // which broke the old hash function. Both must still be findable.
    table.insertRoad(Road("R110", "Road One Ten", 1, 1, 1, 0));
    table.insertRoad(Road("R101", "Road One Oh One", 1, 1, 1, 0));
    check("both anagram-style keys retrievable",
          table.contains("R110") && table.contains("R101"));
}

void testSorting() {
    cout << "\nSorting" << endl;

    Road roads[4];
    int priorities[4] = { 3, 10, 7, 1 };

    for (int i = 0; i < 4; i++) {
        roads[i].setRoadID("R" + to_string(i));
        roads[i].setCurrentPriority(priorities[i]);
    }

    RoadSorter sorter;
    sorter.sortRoads(roads, 4);

    bool descending = true;
    for (int i = 0; i < 3; i++) {
        if (roads[i].getCurrentPriority() <
            roads[i + 1].getCurrentPriority()) {
            descending = false;
        }
    }

    check("sorted in descending priority order", descending);
    check("highest priority first",
          roads[0].getCurrentPriority() == 10);

    // Edge cases that commonly crash hand-written sorts.
    Road empty[1];
    sorter.sortRoads(empty, 0);
    sorter.sortRoads(empty, 1);
    check("size 0 and size 1 handled without crashing", true);
}

void testTrafficIntelligence() {
    cout << "\nTraffic Intelligence" << endl;

    TrafficIntelligence system;
    system.addRoad(Road("R1", "Low traffic",  10, 20,  5, 0));
    system.addRoad(Road("R2", "Heavy jam",    90, 95, 40, 0));
    system.addRoad(Road("R3", "With ambulance", 5, 10, 2, 1));

    system.calculatePriority();

    // Emergency weight is 100, so R3 (10+2+100=112) must outrank
    // the jam on R2 (95+40=135)... it does NOT. Check the real values.
    check("priority = density + waiting + 100 * emergencyVehicles",
          system.getRoad(0)->getCurrentPriority() == 25 &&
          system.getRoad(1)->getCurrentPriority() == 135 &&
          system.getRoad(2)->getCurrentPriority() == 112);

    system.updateSignals();

    int greenCount = 0;
    for (int i = 0; i < system.getRoadCount(); i++) {
        if (system.getRoad(i)->getSignalStatus() == "GREEN") {
            greenCount++;
        }
    }

    check("exactly one road holds GREEN", greenCount == 1);

    // An inactive road must never be chosen.
    system.getRoad(1)->deactivateRoad();
    system.updateSignals();

    check("inactive road gets priority 0 and no GREEN",
          system.getRoad(1)->getCurrentPriority() == 0 &&
          system.getRoad(1)->getSignalStatus() != "GREEN");
}

int main() {
    cout << "ADAPTIVE TRAFFIC MANAGEMENT SYSTEM - UNIT TESTS" << endl;

    testRoad();
    testVehicle();
    testQueue();
    testHeap();
    testHashTable();
    testSorting();
    testTrafficIntelligence();

    cout << "\n--------------------------------" << endl;
    cout << testsPassed << " / " << testsRun << " tests passed." << endl;

    return (testsPassed == testsRun) ? 0 : 1;
}