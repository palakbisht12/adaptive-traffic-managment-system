#include "Database.h"
#include "FileStorage.h"

#ifdef USE_SQLITE
  #include "SqliteStorage.h"
#endif

#include <ctime>
#include <cstdio>

Database::Database() {
    connected = false;
}

Database::~Database() {
    disconnect();
}

std::unique_ptr<IStorage> Database::makeStorage() {
#ifdef USE_SQLITE
    return std::unique_ptr<IStorage>(new SqliteStorage());
#else
    return std::unique_ptr<IStorage>(new FileStorage());
#endif
}

std::string Database::nowTimestamp() {
    std::time_t raw = std::time(0);
    std::tm* parts = std::localtime(&raw);

    char buffer[64];   
    if (parts == 0) {
        return "";
    }
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
                  parts->tm_year + 1900, parts->tm_mon + 1, parts->tm_mday,
                  parts->tm_hour, parts->tm_min, parts->tm_sec);
    return std::string(buffer);
}

bool Database::requireOpen(OperationResult& result) const {
    if (!connected || !storage || !storage->isOpen()) {
        result = OperationResult::failure("Database is not connected", 503);
        return false;
    }
    return true;
}

OperationResult Database::connect(const std::string& target) {
    disconnect();

    storage = makeStorage();
    std::string error;

    if (!storage->open(target, error)) {
        lastError = error;
        storage.reset();
        connected = false;
        return OperationResult::failure("Connection failed: " + error, 503);
    }

    connected = true;
    lastError.clear();
    return OperationResult::success("Connected using " +
                                    storage->backendName());
}

OperationResult Database::createTables() {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }
    return OperationResult::success("Tables ready");
}

void Database::disconnect() {
    if (storage) {
        storage->close();
        storage.reset();
    }
    connected = false;
}

bool Database::isConnected() const {
    return connected && storage && storage->isOpen();
}

std::string Database::getBackendName() const {
    return storage ? storage->backendName() : std::string("none");
}

std::string Database::getLastError() const { return lastError; }


OperationResult Database::insertRoad(const RoadState& road) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    if (road.roadID.empty()) {
        return OperationResult::failure("Road ID cannot be empty");
    }
    if (road.vehicleCount < 0 || road.waitingTime < 0) {
        return OperationResult::failure("Negative values are not allowed");
    }
    if (road.trafficDensity < 0 || road.trafficDensity > 100) {
        return OperationResult::failure("Density must be between 0 and 100");
    }

    std::string error;
    if (!storage->insertRoad(road, error)) {
        lastError = error;
        const int status = (error.find("Duplicate") != std::string::npos)
                               ? 409 : 400;
        return OperationResult::failure(error, status);
    }
    return OperationResult::success("Road inserted: " + road.roadID);
}

OperationResult Database::updateRoad(const RoadState& road) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }
    if (road.roadID.empty()) {
        return OperationResult::failure("Road ID cannot be empty");
    }

    std::string error;
    if (!storage->updateRoad(road, error)) {
        lastError = error;
        return OperationResult::failure(error, 404);
    }
    return OperationResult::success("Road updated: " + road.roadID);
}

OperationResult Database::deleteRoad(const std::string& roadID) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    std::string error;
    if (!storage->deleteRoad(roadID, error)) {
        lastError = error;
        return OperationResult::failure(error, 404);
    }
    return OperationResult::success("Road deleted: " + roadID);
}

OperationResult Database::getRoad(const std::string& roadID, RoadState& out) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    std::string error;
    if (!storage->selectRoad(roadID, out, error)) {
        lastError = error;
        return OperationResult::failure(error, 404);
    }
    return OperationResult::success();
}

OperationResult Database::getAllRoads(std::vector<RoadState>& out) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    std::string error;
    if (!storage->selectAllRoads(out, error)) {
        lastError = error;
        return OperationResult::failure(error, 500);
    }
    // An empty database is a valid state, not an error.
    return OperationResult::success();
}


OperationResult Database::saveTrafficRecord(const RoadState& road) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    TrafficRecord rec;
    rec.roadID = road.roadID;
    rec.vehicleCount = road.vehicleCount;
    rec.trafficDensity = road.trafficDensity;
    rec.waitingTime = road.waitingTime;
    rec.timestamp = nowTimestamp();

    std::string error;
    if (!storage->insertTrafficRecord(rec, error)) {
        lastError = error;
        return OperationResult::failure(error, 500);
    }
    return OperationResult::success("Traffic record saved");
}

OperationResult Database::getTrafficHistory(const std::string& roadID,
                                            int limit,
                                            std::vector<TrafficRecord>& out) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    std::string error;
    if (!storage->selectTrafficHistory(roadID, limit, out, error)) {
        lastError = error;
        return OperationResult::failure(error, 500);
    }
    return OperationResult::success();
}

OperationResult Database::saveSignalChange(const std::string& roadID,
                                           const std::string& previousSignal,
                                           const std::string& newSignal) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    if (previousSignal == newSignal) {
        return OperationResult::success("No signal change to record");
    }

    SignalRecord rec;
    rec.roadID = roadID;
    rec.previousSignal = previousSignal;
    rec.newSignal = newSignal;
    rec.timestamp = nowTimestamp();

    std::string error;
    if (!storage->insertSignalRecord(rec, error)) {
        lastError = error;
        return OperationResult::failure(error, 500);
    }
    return OperationResult::success("Signal change saved");
}

OperationResult Database::getSignalHistory(const std::string& roadID,
                                           int limit,
                                           std::vector<SignalRecord>& out) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    std::string error;
    if (!storage->selectSignalHistory(roadID, limit, out, error)) {
        lastError = error;
        return OperationResult::failure(error, 500);
    }
    return OperationResult::success();
}


OperationResult Database::saveEmergencyEvent(const std::string& roadID,
                                             const std::string& type,
                                             int& eventID) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }
    if (roadID.empty()) {
        return OperationResult::failure("Road ID cannot be empty");
    }

    RoadState probe;
    std::string probeError;
    if (!storage->selectRoad(roadID, probe, probeError)) {
        return OperationResult::failure(
            "Cannot record an emergency for an unknown road: " + roadID, 404);
    }

    EmergencyInfo ev;
    ev.roadID = roadID;
    ev.emergencyType = type.empty() ? "Ambulance" : type;
    ev.startTime = nowTimestamp();
    ev.endTime = "";
    ev.status = "ACTIVE";

    std::string error;
    if (!storage->insertEmergency(ev, eventID, error)) {
        lastError = error;
        return OperationResult::failure(error, 500);
    }
    return OperationResult::success("Emergency recorded on " + roadID);
}

OperationResult Database::completeEmergency(const std::string& roadID) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    std::string error;
    if (!storage->completeEmergency(roadID, nowTimestamp(), error)) {
        lastError = error;
        return OperationResult::failure(error, 404);
    }
    return OperationResult::success("Emergency completed on " + roadID);
}

OperationResult Database::getActiveEmergencies(
    std::vector<EmergencyInfo>& out) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    std::string error;
    if (!storage->selectEmergencies("", true, out, error)) {
        lastError = error;
        return OperationResult::failure(error, 500);
    }
    return OperationResult::success();
}

OperationResult Database::getEmergencyHistory(const std::string& roadID,
                                              std::vector<EmergencyInfo>& out) {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    std::string error;
    if (!storage->selectEmergencies(roadID, false, out, error)) {
        lastError = error;
        return OperationResult::failure(error, 500);
    }
    return OperationResult::success();
}

OperationResult Database::clearAll() {
    OperationResult guard;
    if (!requireOpen(guard)) {
        return guard;
    }

    std::string error;
    if (!storage->clearAll(error)) {
        lastError = error;
        return OperationResult::failure(error, 500);
    }
    return OperationResult::success("All data cleared");
}
