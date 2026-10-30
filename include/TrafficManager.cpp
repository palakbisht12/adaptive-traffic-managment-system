
#include "TrafficManager.h"

TrafficManager::TrafficManager() {
    roadCount = 0;
    nextVehicleID = 1;
    lastSelectedRoadIndex = -1;
    consecutiveSelections = 0;
}

int TrafficManager::findRoadIndex(const string& roadID) {
    for (int i = 0; i < roadCount; i++) {
        if (roads[i].getRoadID() == roadID) {
            return i;
        }
    }
    return -1;
}

// FIX 4: demand is measured by the road's reported vehicle count or its
// emergency status, NOT by whether individual Vehicle objects happen to be
// sitting in the queue. updateRoadTraffic() and the Road constructor set the
// count without queueing anything, so a queue-only test made those roads
// invisible to the selector.
bool TrafficManager::hasDemand(int index) const {
    if (index < 0 || index >= roadCount) {
        return false;
    }

    return roads[index].getVehicleCount() > 0 ||
           roads[index].getEmergencyStatus();
}

bool TrafficManager::addRoad(const Road& road) {
    if (roadCount >= MAX_ROADS) {
        cout << "Maximum road limit reached." << endl;
        return false;
    }

    if (road.getRoadID().empty()) {
        cout << "Road ID cannot be empty." << endl;
        return false;
    }

    if (findRoadIndex(road.getRoadID()) != -1) {
        cout << "Road ID already exists." << endl;
        return false;
    }

    roads[roadCount] = road;
    roadTable.insertRoad(road);
    roadCount++;
    return true;
}

bool TrafficManager::removeRoad(const string& roadID) {
    int index = findRoadIndex(roadID);

    if (index == -1) {
        cout << "Road not found." << endl;
        return false;
    }

    roadTable.deleteRoad(roadID);

    Vehicle discarded;
    while (vehicleQueues[index].serveVehicle(discarded)) {
    }

    // Shift each road and its queue together.
    for (int i = index; i < roadCount - 1; i++) {
        roads[i] = roads[i + 1];

        Vehicle vehicle;
        while (vehicleQueues[i + 1].serveVehicle(vehicle)) {
            vehicleQueues[i].enqueue(vehicle);
        }
    }

    roadCount--;
    lastSelectedRoadIndex = -1;
    consecutiveSelections = 0;

    calculateAllPriorities();
    return true;
}

bool TrafficManager::updateRoadTraffic(
    const string& roadID,
    int vehicleCount,
    int trafficDensity
) {
    int index = findRoadIndex(roadID);

    if (index == -1 || vehicleCount < 0 ||
        trafficDensity < 0 || trafficDensity > 100) {
        cout << "Invalid road ID or traffic values." << endl;
        return false;
    }

    roads[index].setVehicleCount(vehicleCount);
    roads[index].setTrafficDensity(trafficDensity);
    calculateAllPriorities();
    return true;
}

bool TrafficManager::addVehicle(
    const string& roadID,
    const Vehicle& vehicle
) {
    int index = findRoadIndex(roadID);

    if (index == -1) {
        cout << "Road not found." << endl;
        return false;
    }

    vehicleQueues[index].enqueue(vehicle);
    roads[index].setVehicleCount(
        roads[index].getVehicleCount() + 1
    );

    if (vehicle.getIsEmergency()) {
        roads[index].setEmergencyVehicleCount(
            roads[index].getEmergencyVehicleCount() + 1
        );
    }

    calculateAllPriorities();
    return true;
}

bool TrafficManager::addEmergencyVehicle(
    const string& roadID,
    const Vehicle& vehicle
) {
    if (!vehicle.getIsEmergency()) {
        cout << "The supplied vehicle is not an emergency vehicle."
             << endl;
        return false;
    }

    return addVehicle(roadID, vehicle);
}


bool TrafficManager::removeVehicle(const string& roadID) {
    int index = findRoadIndex(roadID);

    if (index == -1) {
        cout << "Road not found." << endl;
        return false;
    }

    // FIX 4: a road can hold vehicles that were never queued individually
    // (updateRoadTraffic sets only the count). Without this branch the
    // selector would give such a road GREEN, fail to serve anything, and
    // the road would hold GREEN forever because its count never dropped.
    if (vehicleQueues[index].isEmpty()) {
        if (roads[index].getVehicleCount() == 0) {
            cout << "No vehicles waiting on " << roadID << "." << endl;
            return false;
        }

        roads[index].setVehicleCount(
            roads[index].getVehicleCount() - 1
        );

        if (roads[index].getVehicleCount() == 0) {
            roads[index].setEmergencyVehicleCount(0);
        }

        calculateAllPriorities();
        return true;
    }

    Vehicle servedVehicle;

    if (!vehicleQueues[index].serveEmergencyVehicle(servedVehicle)) {
        if (!vehicleQueues[index].serveVehicle(servedVehicle)) {
            return false;
        }
    }

    cout << "Served vehicle: "
         << servedVehicle.getVehicleID()
         << (servedVehicle.getIsEmergency()
                 ? " (EMERGENCY)"
                 : " (NORMAL)")
         << endl;

    int count = roads[index].getVehicleCount();

    if (count > 0) {
        roads[index].setVehicleCount(count - 1);
    }

    if (servedVehicle.getIsEmergency()) {
        int emergencyCount =
            roads[index].getEmergencyVehicleCount();

        if (emergencyCount > 0) {
            roads[index].setEmergencyVehicleCount(
                emergencyCount - 1
            );
        }
    }

    calculateAllPriorities();
    return true;
}

// FIX 5: this definition was deleted while the declaration stayed in
// TrafficManager.h, which left an undefined reference for any caller.
bool TrafficManager::updateWaitingTime(
    const string& roadID,
    int waitingTime
) {
    int index = findRoadIndex(roadID);

    if (index == -1 || waitingTime < 0) {
        cout << "Invalid road ID or waiting time." << endl;
        return false;
    }

    roads[index].setWaitingTime(waitingTime);
    calculateAllPriorities();
    return true;
}

bool TrafficManager::updateTrafficDensity(
    const string& roadID,
    int density
) {
    int index = findRoadIndex(roadID);

    if (index == -1 || density < 0 || density > 100) {
        cout << "Invalid road ID or traffic density." << endl;
        return false;
    }

    roads[index].setTrafficDensity(density);
    calculateAllPriorities();
    return true;
}

void TrafficManager::calculateAllPriorities() {
    for (int i = 0; i < roadCount; i++) {
        priorityManager.updateRoadPriority(roads[i], 0);
    }
}

int TrafficManager::selectNextRoad() {
    // FIX 3: score EVERY road, not just the candidates, so the priority a
    // caller reads is always consistent with the decision. Inactive roads
    // score 0 here, which is what the dashboard expects to see.
    calculateAllPriorities();

    for (int i = 0; i < roadCount; i++) {
        roads[i].setSignalStatus("RED");
    }

    // STEP 1: Emergency vehicles always take precedence.
    int emergencyRoadIndex = -1;
    int closestEmergencyPosition = -1;

    for (int i = 0; i < roadCount; i++) {
        if (!roads[i].getActiveStatus()) {
            continue;
        }

        int position =
            vehicleQueues[i].getFirstEmergencyPosition();

        if (position == -1) {
            // FIX 4: the emergency may have been reported on the Road
            // object with no emergency Vehicle queued (this is what the
            // API does). Treat it as waiting at the back of the queue.
            if (!roads[i].getEmergencyStatus()) {
                continue;
            }

            position = roads[i].getVehicleCount() + 1;
        }

        if (emergencyRoadIndex == -1 ||
            position < closestEmergencyPosition) {
            emergencyRoadIndex = i;
            closestEmergencyPosition = position;
        }
    }

    if (emergencyRoadIndex != -1) {

        roads[emergencyRoadIndex].setSignalStatus("GREEN");

        priorityManager.updateRoadPriority(
            roads[emergencyRoadIndex], 0
        );

        if (emergencyRoadIndex == lastSelectedRoadIndex) {
            consecutiveSelections++;
        } else {
            lastSelectedRoadIndex = emergencyRoadIndex;
            consecutiveSelections = 1;
        }

        return emergencyRoadIndex;
    }

    // STEP 2: Determine whether another road can be served.
    bool anotherRoadWaiting = false;

    for (int i = 0; i < roadCount; i++) {
        if (i != lastSelectedRoadIndex &&
            roads[i].getActiveStatus() &&
            hasDemand(i)) {
            anotherRoadWaiting = true;
            break;
        }
    }

    bool enforceFairness =
        lastSelectedRoadIndex >= 0 &&
        lastSelectedRoadIndex < roadCount &&
        consecutiveSelections >= 3 &&
        anotherRoadWaiting;


    // STEP 3: When fairness is due, scan from the road
    // after the last selected road, wrapping around.
    if (enforceFairness) {
        for (int offset = 1; offset <= roadCount; offset++) {
            int i = (lastSelectedRoadIndex + offset) % roadCount;

            if (!roads[i].getActiveStatus() || !hasDemand(i)) {
                continue;
            }

            roads[i].setSignalStatus("GREEN");
            priorityManager.updateRoadPriority(roads[i], 0);

            lastSelectedRoadIndex = i;
            consecutiveSelections = 1;

            return i;
        }
    }

    // STEP 4: Fairness is not due, so choose the
    // highest-priority road with queued vehicles.
    int selectedIndex = -1;
    int highestPriority = -1;

    for (int i = 0; i < roadCount; i++) {
        if (!roads[i].getActiveStatus() || !hasDemand(i)) {
            continue;
        }

        priorityManager.updateRoadPriority(roads[i], 0);
        int priority = roads[i].getCurrentPriority();

        if (selectedIndex == -1 || priority > highestPriority) {
            highestPriority = priority;
            selectedIndex = i;
        }
    }

    if (selectedIndex != -1) {
        roads[selectedIndex].setSignalStatus("GREEN");

        if (selectedIndex == lastSelectedRoadIndex) {
            consecutiveSelections++;
        } else {
            lastSelectedRoadIndex = selectedIndex;
            consecutiveSelections = 1;
        }
    }

    return selectedIndex;
}

bool TrafficManager::changeSignal(
    const string& roadID,
    const string& signal
) {
    int index = findRoadIndex(roadID);

    if (index == -1) {
        return false;
    }

    if (signal != "GREEN" && signal != "RED" &&
        signal != "YELLOW") {
        return false;
    }

    if (signal == "GREEN" &&
        !roads[index].getActiveStatus()) {
        return false;
    }

    if (signal == "GREEN") {
        for (int i = 0; i < roadCount; i++) {
            roads[i].setSignalStatus("RED");
        }
    }

    roads[index].setSignalStatus(signal);
    return true;
}

void TrafficManager::processTrafficCycle() {
    for (int i = 0; i < roadCount; i++) {
        if (roads[i].getActiveStatus() && hasDemand(i)) {
            roads[i].setWaitingTime(
                roads[i].getWaitingTime() + 1
            );
        }
    }

    calculateAllPriorities();

    int selectedIndex = selectNextRoad();


    if (selectedIndex == -1) {
        cout << "No active road with queued vehicles." << endl;
        return;
    }

    cout << "\nSelected road: "
         << roads[selectedIndex].getRoadName() << endl;

    cout << "Priority: "
         << roads[selectedIndex].getCurrentPriority() << endl;

    cout << "Signal: "
         << roads[selectedIndex].getSignalStatus() << endl;

    string selectedRoadID = roads[selectedIndex].getRoadID();

    if (removeVehicle(selectedRoadID)) {
        cout << "One vehicle served." << endl;

        int index = findRoadIndex(selectedRoadID);

        if (index != -1 && vehicleQueues[index].isEmpty()) {
            roads[index].setWaitingTime(0);
        }
    }

    calculateAllPriorities();
}

void TrafficManager::displayAllRoads() const {
    for (int i = 0; i < roadCount; i++) {
        roads[i].displayRoad();

        cout << "Queued vehicles: "
             << vehicleQueues[i].size() << endl;

        cout << "------------------------" << endl;
    }
}

int TrafficManager::getRoadCount() const {
    return roadCount;
}

Road* TrafficManager::getRoad(const string& roadID) {
    int index = findRoadIndex(roadID);
    return (index == -1) ? nullptr : &roads[index];
}

Road* TrafficManager::getRoad(int index) {
    if (index < 0 || index >= roadCount) {
        return nullptr;
    }

    return &roads[index];
}
