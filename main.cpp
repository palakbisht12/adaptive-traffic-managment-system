#include <iostream>
#include <string>
#include <unordered_map>
#include "road.h"
#include "vehicle.h"
#include "queue.h"
#include "heap.h"
#include "hashtable.h"
#include "sorting.h"
#include "trafficintelligent.h"

using namespace std;

int main() {
    cout << "ADAPTIVE TRAFFIC MANAGEMENT SYSTEM" << endl;

    // Realistic data: Road(id, name, vehicles, density, waiting, emergencyVehicles)
    Road roads[3] = {
        Road("R101", "Rajpur Road",      40, 55, 12, 0),
        Road("R102", "Saharanpur Road",  75, 90, 30, 0),
        Road("R103", "Haridwar Road",    20, 30,  8, 1)
    };

    // ---------- HASH TABLE: O(1) average lookup by RoadID ----------
    cout << "\n===== HASH TABLE =====" << endl;

    unordered_map<string, Road*> table;
    for (int i = 0; i < 3; i++) {
        table[roads[i].getRoadID()] = &roads[i];
    }

    for (const auto& entry : table) {
        cout << entry.first << " - " << entry.second->getRoadName() << endl;
    }

    auto foundEntry = table.find("R102");
    Road* found = foundEntry == table.end() ? nullptr : foundEntry->second;
    if (found != nullptr) {
        cout << "Found " << found->getRoadID()
             << " - " << found->getRoadName() << endl;
    }
    else {
        cout << "Road not found." << endl;
    }

    cout << "Lookup of R999: "
            << (table.find("R999") != table.end() ? "found" : "not found") << endl;

    // ---------- TRAFFIC INTELLIGENCE: priority + signal decision ----------
    TrafficIntelligence system;
    for (int i = 0; i < 3; i++) {
        system.addRoad(roads[i]);
    }

    system.displayTrafficIntelligence();

    // ---------- SORTING: ranked report ----------
    cout << "\n===== SORTING ROADS BY PRIORITY =====" << endl;

    Road ranked[3];
    for (int i = 0; i < 3; i++) {
        ranked[i] = *system.getRoad(i);
    }

    RoadSorter sorter;
    sorter.sortRoads(ranked, 3);
    sorter.displaySortedRoads(ranked, 3);

    // ---------- HEAP: highest-priority road ----------
    cout << "\n===== PRIORITY HEAP =====" << endl;

    RoadHeap heap;
    for (int i = 0; i < 3; i++) {
        heap.insertRoad(ranked[i]);
    }

    heap.displayHeap();

    Road highest;
    if (heap.removeHighestPriorityRoad(highest)) {
        cout << "Served road: " << highest.getRoadID()
             << " (priority " << highest.getCurrentPriority() << ")" << endl;
        cout << "Roads remaining in heap: " << heap.size() << endl;
    }

    // ---------- QUEUE: FIFO vehicles waiting at a road ----------
    cout << "\n===== VEHICLE QUEUE =====" << endl;

    VehicleQueue queue;

    queue.enqueue(Vehicle("V101", "Car",       1, false));
    queue.enqueue(Vehicle("V102", "Ambulance", 2, false));
    queue.enqueue(Vehicle("V103", "Bus",       3, false));

    cout << "\nVehicles in queue (" << queue.size() << "):" << endl;
    queue.display();

    Vehicle* nextVehicle = queue.front();
    if (nextVehicle != nullptr) {
        cout << "\nNext to be served:" << endl;
        nextVehicle->displayVehicleInfo();
    }

    cout << "\nRemoving first vehicle:" << endl;
    queue.dequeue();

    cout << "\nRemaining vehicles (" << queue.size() << "):" << endl;
    queue.display();

    return 0;
}