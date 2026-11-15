
#include "Integration.h"

#include <sstream>
#include <chrono>

using namespace std;

Integration::Integration()
    : roads(database),
      trafficLog(database),
      signalLog(database),
      emergencies(database) {
    simulation = SimulationState::STOPPED;
    cycleCount = 0;
    databaseReady = false;
    cycleIntervalMs = 1200;
    lastTickMs = 0;
}

Integration::~Integration() {
    shutdown();
}

RoadState Integration::toState(Road* road, int queued) {
    RoadState s;
    if (road == 0) {
        return s;
    }

    s.roadID = road->getRoadID();
    s.roadName = road->getRoadName();
    s.vehicleCount = road->getVehicleCount();
    s.trafficDensity = road->getTrafficDensity();
    s.waitingTime = road->getWaitingTime();
    s.emergencyVehicleCount = road->getEmergencyVehicleCount();
    s.emergencyStatus = road->getEmergencyStatus();
    s.currentPriority = road->getCurrentPriority();
    s.signalStatus = road->getSignalStatus();
    s.activeStatus = road->getActiveStatus();
    s.queuedVehicles = queued;
    return s;
}

OperationResult Integration::initialise(const string& dataTarget) {
    OperationResult connectResult = database.connect(dataTarget);

    if (!connectResult.ok) {
        databaseReady = false;
        return OperationResult::failure(
            "Running WITHOUT persistence — " + connectResult.message, 200);
    }

    database.createTables();
    databaseReady = true;
    return OperationResult::success("Storage ready (" +
                                    database.getBackendName() + ")");
}

OperationResult Integration::loadFromDatabase() {
    if (!databaseReady) {
        return OperationResult::failure("Database is not connected", 503);
    }

    vector<RoadState> stored;
    OperationResult r = roads.getAllRoads(stored);
    if (!r.ok) {
        return r;
    }

    int restored = 0;
    for (size_t i = 0; i < stored.size(); i++) {
        const RoadState& s = stored[i];

        Road road(s.roadID, s.roadName, s.vehicleCount, s.trafficDensity,
                  s.waitingTime, s.emergencyVehicleCount);
        if (manager.addRoad(road)) {
            restored++;
            previousSignals[s.roadID] = s.signalStatus;
        }
    }

    manager.calculateAllPriorities();

    ostringstream msg;
    msg << "Restored " << restored << " road(s) from storage";
    return OperationResult::success(msg.str());
}

OperationResult Integration::seedDemoRoads() {
    if (manager.getRoadCount() > 0) {
        return OperationResult::success("Roads already present; not seeding");
    }

    addRoad("R101", "Rajpur Road", 40, 55, 12);
    addRoad("R102", "Saharanpur Road", 75, 90, 30);
    addRoad("R103", "Haridwar Road", 20, 30, 8);
    addRoad("R104", "Chakrata Road", 8, 10, 5);

    return OperationResult::success("Seeded 4 demo roads");
}

void Integration::shutdown() {
    if (databaseReady) {
        syncAllRoadsToDatabase();
        database.disconnect();
        databaseReady = false;
    }
}

bool Integration::isDatabaseReady() const {
    return databaseReady;
}

string Integration::databaseBackend() const {
    return databaseReady ? database.getBackendName() : string("none");
}

void Integration::syncRoadToDatabase(const string& roadID) {
    if (!databaseReady) {
        return;
    }

    Road* road = manager.getRoad(roadID);
    if (road == 0) {
        return;
    }

    const RoadState state = toState(road, 0);

    if (!roads.updateRoad(state).ok) {
        roads.addRoad(state);
    }
}

void Integration::syncAllRoadsToDatabase() {
    if (!databaseReady) {
        return;
    }

    for (int i = 0; i < manager.getRoadCount(); i++) {
        Road* road = manager.getRoad(i);
        if (road != 0) {
            syncRoadToDatabase(road->getRoadID());
        }
    }
}

void Integration::recordSignalChanges() {
    for (int i = 0; i < manager.getRoadCount(); i++) {
        Road* road = manager.getRoad(i);
        if (road == 0) {
            continue;
        }

        const string id = road->getRoadID();
        const string now = road->getSignalStatus();

        map<string, string>::iterator it = previousSignals.find(id);
        const string before =
            (it == previousSignals.end()) ? string("RED") : it->second;

        if (before != now) {
            if (databaseReady) {
                signalLog.saveSignalChange(id, before, now);
            }
            previousSignals[id] = now;
        }
    }
}

PriorityResult Integration::readCurrentDecision() {
    PriorityResult result;

    for (int i = 0; i < manager.getRoadCount(); i++) {
        Road* road = manager.getRoad(i);
        if (road == 0 || road->getSignalStatus() != "GREEN") {
            continue;
        }

        result.valid = true;
        result.roadID = road->getRoadID();
        result.roadName = road->getRoadName();
        result.priority = road->getCurrentPriority();
        result.signalStatus = "GREEN";
        result.reason = manager.getSelectionReason(i);
        recordSignalChanges();
        return result;
    }

    result.valid = false;
    result.reason = "No road can be served";
    result.errorMessage = (manager.getRoadCount() == 0)
        ? "No roads configured"
        : "No active road has waiting traffic";
    recordSignalChanges();
    return result;
}

PriorityResult Integration::buildDecision() {
    PriorityResult result;

    const int index = manager.selectNextRoad();
    if (index < 0) {
        result.valid = false;
        result.reason = "No road can be served";
        result.errorMessage =
            (manager.getRoadCount() == 0)
                ? "No roads configured"
                : "No active road has waiting traffic";
        return result;
    }

    Road* road = manager.getRoad(index);
    if (road == 0) {
        result.valid = false;
        result.errorMessage = "Selected road index is out of range";
        return result;
    }

    result.valid = true;
    result.roadID = road->getRoadID();
    result.roadName = road->getRoadName();
    result.priority = road->getCurrentPriority();
    result.signalStatus = road->getSignalStatus();
    result.reason = manager.getSelectionReason(index);

    recordSignalChanges();
    return result;
}

OperationResult Integration::addRoad(const string& roadID,
                                     const string& roadName,
                                     int vehicles, int density, int waiting) {
    if (roadID.empty()) {
        return OperationResult::failure("Road ID cannot be empty");
    }
    if (roadName.empty()) {
        return OperationResult::failure("Road name cannot be empty");
    }
    if (vehicles < 0 || waiting < 0) {
        return OperationResult::failure("Negative values are not allowed");
    }
    if (density < 0 || density > 100) {
        return OperationResult::failure("Density must be between 0 and 100");
    }
    if (manager.getRoad(roadID) != 0) {
        return OperationResult::failure("Duplicate Road ID: " + roadID, 409);
    }

    Road road(roadID, roadName, vehicles, density, waiting, 0);
    if (!manager.addRoad(road)) {
        return OperationResult::failure("Traffic Intelligence rejected the "
                                        "road (limit reached?)", 500);
    }

    previousSignals[roadID] = "RED";
    manager.calculateAllPriorities();
    syncRoadToDatabase(roadID);

    if (databaseReady) {
        RoadState state = toState(manager.getRoad(roadID), 0);
        trafficLog.saveTrafficUpdate(state);
    }

    return OperationResult::success("Road added: " + roadID);
}

OperationResult Integration::updateRoad(const string& roadID,
                                        int vehicles, int density,
                                        int waiting) {
    if (manager.getRoad(roadID) == 0) {
        return OperationResult::failure("Unknown Road ID: " + roadID, 404);
    }
    if (vehicles < 0 || waiting < 0) {
        return OperationResult::failure("Negative values are not allowed");
    }
    if (density < 0 || density > 100) {
        return OperationResult::failure("Density must be between 0 and 100");
    }

    manager.updateRoadTraffic(roadID, vehicles, density);
    manager.updateWaitingTime(roadID, waiting);
    manager.calculateAllPriorities();

    syncRoadToDatabase(roadID);
    if (databaseReady) {
        RoadState state = toState(manager.getRoad(roadID), 0);
        trafficLog.saveTrafficUpdate(state);
    }

    return OperationResult::success("Road updated: " + roadID);
}

OperationResult Integration::removeRoad(const string& roadID) {
    if (manager.getRoad(roadID) == 0) {
        return OperationResult::failure("Unknown Road ID: " + roadID, 404);
    }
    if (!manager.removeRoad(roadID)) {
        return OperationResult::failure("Could not remove " + roadID, 500);
    }

    previousSignals.erase(roadID);
    if (databaseReady) {
        roads.deleteRoad(roadID);
    }

    return OperationResult::success("Road removed: " + roadID);
}

OperationResult Integration::getRoad(const string& roadID, RoadState& out) {
    Road* road = manager.getRoad(roadID);
    if (road == 0) {
        return OperationResult::failure("Unknown Road ID: " + roadID, 404);
    }

    out = toState(road, 0);
    return OperationResult::success();
}

OperationResult Integration::getAllRoads(vector<RoadState>& out) {
    out.clear();
    manager.calculateAllPriorities();

    for (int i = 0; i < manager.getRoadCount(); i++) {
        Road* road = manager.getRoad(i);
        if (road != 0) {
            out.push_back(toState(road, 0));
        }
    }

    return OperationResult::success();
}

OperationResult Integration::updateTraffic(const string& roadID,
                                           int addVehicles,
                                           int removeVehicles,
                                           int density, int waiting) {
    Road* road = manager.getRoad(roadID);
    if (road == 0) {
        return OperationResult::failure("Unknown Road ID: " + roadID, 404);
    }
    if (addVehicles < 0 || removeVehicles < 0) {
        return OperationResult::failure("Vehicle counts cannot be negative");
    }

    int count = road->getVehicleCount() + addVehicles - removeVehicles;
    if (count < 0) {
        count = 0;
    }

    const int newDensity = (density >= 0) ? density : road->getTrafficDensity();
    if (newDensity > 100) {
        return OperationResult::failure("Density must be between 0 and 100");
    }

    manager.updateRoadTraffic(roadID, count, newDensity);
    if (waiting >= 0) {
        manager.updateWaitingTime(roadID, waiting);
    }

    manager.calculateAllPriorities();
    syncRoadToDatabase(roadID);

    if (databaseReady) {
        RoadState state = toState(manager.getRoad(roadID), 0);
        trafficLog.saveTrafficUpdate(state);
    }

    return OperationResult::success("Traffic updated on " + roadID);
}

OperationResult Integration::startEmergency(const string& roadID,
                                            const string& type) {
    Road* road = manager.getRoad(roadID);
    if (road == 0) {
        return OperationResult::failure("Unknown Road ID: " + roadID, 404);
    }
    if (!road->getActiveStatus()) {
        return OperationResult::failure(
            "Cannot route an emergency onto a closed road: " + roadID);
    }

    const string vehicleID = "EMG-" + roadID;
    Vehicle ambulance(vehicleID, type.empty() ? "Emergency" : type, 0, true);

    if (!manager.addEmergencyVehicle(roadID, ambulance)) {
        return OperationResult::failure(
            "Traffic Intelligence rejected the emergency vehicle", 500);
    }

    manager.calculateAllPriorities();
    lastDecision = buildDecision();

    if (databaseReady) {
        int eventID = 0;
        syncRoadToDatabase(roadID);
        emergencies.addEmergencyEvent(roadID,
                                      type.empty() ? "Ambulance" : type,
                                      eventID);
        syncAllRoadsToDatabase();
    }

    return OperationResult::success("Emergency started on " + roadID);
}

OperationResult Integration::endEmergency(const string& roadID) {
    Road* road = manager.getRoad(roadID);
    if (road == 0) {
        return OperationResult::failure("Unknown Road ID: " + roadID, 404);
    }
    if (!road->getEmergencyStatus()) {
        return OperationResult::failure("No active emergency on " + roadID,
                                        404);
    }

    manager.removeVehicle(roadID);
    road->setEmergencyVehicleCount(0);
    manager.calculateAllPriorities();
    lastDecision = buildDecision();

    if (databaseReady) {
        emergencies.markEmergencyCompleted(roadID);
        syncAllRoadsToDatabase();
    }

    return OperationResult::success("Emergency cleared on " + roadID);
}

PriorityResult Integration::getCurrentPriority() {
    lastDecision = buildDecision();
    if (databaseReady) {
        syncAllRoadsToDatabase();
    }
    return lastDecision;
}

OperationResult Integration::runCycle() {
    if (manager.getRoadCount() == 0) {
        return OperationResult::failure("No roads configured", 400);
    }

    if (simulation == SimulationState::PAUSED) {
        return OperationResult::failure("Simulation is paused", 409);
    }

    manager.processTrafficCycle();
    cycleCount++;

    lastDecision = readCurrentDecision();

    if (databaseReady) {
        syncAllRoadsToDatabase();
        for (int i = 0; i < manager.getRoadCount(); i++) {
            Road* road = manager.getRoad(i);
            if (road != 0) {
                trafficLog.saveTrafficUpdate(toState(road, 0));
            }
        }
    }

    ostringstream msg;
    msg << "Cycle " << cycleCount << " processed";
    return OperationResult::success(msg.str());
}

OperationResult Integration::simulationStart() {
    if (manager.getRoadCount() == 0) {
        return OperationResult::failure("Add a road before starting", 400);
    }

    simulation = SimulationState::RUNNING;
    lastTickMs = 0;
    return OperationResult::success("Simulation running");
}

OperationResult Integration::simulationPause() {
    if (simulation != SimulationState::RUNNING) {
        return OperationResult::failure("Simulation is not running", 409);
    }

    simulation = SimulationState::PAUSED;
    return OperationResult::success("Simulation paused");
}

OperationResult Integration::simulationResume() {
    if (simulation != SimulationState::PAUSED) {
        return OperationResult::failure("Simulation is not paused", 409);
    }

    simulation = SimulationState::RUNNING;
    lastTickMs = 0;
    return OperationResult::success("Simulation resumed");
}

OperationResult Integration::simulationReset() {
    simulation = SimulationState::STOPPED;
    cycleCount = 0;
    lastDecision = PriorityResult();

    for (int i = 0; i < manager.getRoadCount(); i++) {
        Road* road = manager.getRoad(i);
        if (road != 0) {
            road->setWaitingTime(0);
            road->setSignalStatus("RED");
        }
    }

    manager.calculateAllPriorities();
    syncAllRoadsToDatabase();

    return OperationResult::success("Simulation reset");
}

SimulationState Integration::simulationStatus() const {
    return simulation;
}

long long Integration::nowMillis() {
    using namespace chrono;

    return duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch()).count();
}

void Integration::tickIfRunning() {
    if (simulation != SimulationState::RUNNING) {
        return;
    }

    const long long now = nowMillis();

    if (lastTickMs == 0) {
        lastTickMs = now;
        runCycle();
        return;
    }

    int budget = 5;
    while (now - lastTickMs >= cycleIntervalMs && budget > 0) {
        lastTickMs += cycleIntervalMs;
        budget--;
        runCycle();
    }

    if (now - lastTickMs >= cycleIntervalMs) {
        lastTickMs = now;
    }
}

void Integration::setCycleInterval(int milliseconds) {
    if (milliseconds < 100) {
        milliseconds = 100;
    }
    if (milliseconds > 10000) {
        milliseconds = 10000;
    }

    cycleIntervalMs = milliseconds;
}

int Integration::getCycleInterval() const {
    return cycleIntervalMs;
}

int Integration::getCycleCount() const {
    return cycleCount;
}

TrafficStateSnapshot Integration::getSnapshot() {
    TrafficStateSnapshot snap;
    getAllRoads(snap.roads);
    snap.current = lastDecision;
    snap.simulation = simulation;
    snap.cycle = cycleCount;
    return snap;
}

OperationResult Integration::getTrafficHistory(const string& roadID,
                                               int limit,
                                               vector<TrafficRecord>& out) {
    if (!databaseReady) {
        return OperationResult::failure("Database is not connected", 503);
    }

    return trafficLog.getTrafficHistory(roadID, out, limit);
}

OperationResult Integration::getSignalHistory(const string& roadID,
                                              int limit,
                                              vector<SignalRecord>& out) {
    if (!databaseReady) {
        return OperationResult::failure("Database is not connected", 503);
    }

    return signalLog.getSignalChangesForRoad(roadID, out, limit);
}

OperationResult Integration::getEmergencyHistory(
    const string& roadID, vector<EmergencyInfo>& out) {
    if (!databaseReady) {
        return OperationResult::failure("Database is not connected", 503);
    }

    return emergencies.getEmergencyHistory(roadID, out);
}

int Integration::findVehiclePosition(const string& roadID,
                                     const string& vehicleID) {
    return manager.findVehiclePosition(roadID, vehicleID);
}

OperationResult Integration::getQueue(const string& roadID,
                                      vector<QueuedVehicle>& out) {
    out.clear();

    if (manager.getRoad(roadID) == 0) {
        return OperationResult::failure("Unknown Road ID: " + roadID, 404);
    }

    Vehicle buffer[200];
    const int count = manager.listQueue(roadID, buffer, 200);

    for (int i = 0; i < count; i++) {
        QueuedVehicle q;
        q.position = i + 1;
        q.vehicleID = buffer[i].getVehicleID();
        q.vehicleType = buffer[i].getVehicleType();
        q.emergency = buffer[i].getIsEmergency();
        q.arrivalTime = buffer[i].getArrivalTime();
        out.push_back(q);
    }

    return OperationResult::success();
}

OperationResult Integration::addVehicleToQueue(const string& roadID,
                                               const string& vehicleID,
                                               const string& type) {
    if (manager.getRoad(roadID) == 0) {
        return OperationResult::failure("Unknown Road ID: " + roadID, 404);
    }
    if (vehicleID.empty()) {
        return OperationResult::failure("Vehicle ID cannot be empty");
    }

    if (manager.findVehiclePosition(roadID, vehicleID) > 0) {
        return OperationResult::failure(
            vehicleID + " is already queued on " + roadID, 409);
    }

    const string kind = type.empty() ? "Normal" : type;
    const bool emergency = (kind == "Emergency" || kind == "Ambulance" ||
                            kind == "Fire Brigade" || kind == "Police");

    Vehicle vehicle(vehicleID, kind, cycleCount, emergency);

    const bool ok = emergency
        ? manager.addEmergencyVehicle(roadID, vehicle)
        : manager.addVehicle(roadID, vehicle);

    if (!ok) {
        return OperationResult::failure("Traffic Intelligence rejected " +
                                        vehicleID, 500);
    }

    manager.calculateAllPriorities();
    syncRoadToDatabase(roadID);

    return OperationResult::success(vehicleID + " queued on " + roadID);
}
