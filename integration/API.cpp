
#include "API.h"
#include "Json.h"
#include "ErrorHandler.h"

#include <fstream>
#include <sstream>

using namespace std;

API::API(Integration& integration, const string& frontendDir)
    : system(integration), frontendDirectory(frontendDir) {}

void API::setFrontendDirectory(const string& dir) {
    frontendDirectory = dir;
}

HttpResponse API::dispatch(const HttpRequest& request, void* context) {
    API* self = static_cast<API*>(context);

    if (self == 0) {
        return ErrorHandler::error(500, "API is not initialised");
    }

    return self->handle(request);
}

string API::roadToJson(const RoadState& r) {
    JsonWriter out;
    out.beginObject()
       .field("roadID", r.roadID)
       .field("roadName", r.roadName)
       .field("vehicleCount", r.vehicleCount)
       .field("trafficDensity", r.trafficDensity)
       .field("waitingTime", r.waitingTime)
       .field("emergencyVehicleCount", r.emergencyVehicleCount)
       .field("emergencyStatus", r.emergencyStatus)
       .field("currentPriority", r.currentPriority)
       .field("signalStatus", r.signalStatus)
       .field("activeStatus", r.activeStatus)
       .endObject();

    return out.str();
}

string API::decisionToJson(const PriorityResult& d) {
    JsonWriter out;
    out.beginObject()
       .field("valid", d.valid)
       .field("roadID", d.roadID)
       .field("roadName", d.roadName)
       .field("priority", d.priority)
       .field("signalStatus", d.signalStatus)
       .field("reason", d.reason)
       .field("errorMessage", d.errorMessage)
       .endObject();

    return out.str();
}

string API::snapshotToJson(TrafficStateSnapshot& snap) {
    JsonWriter out;
    out.beginObject();
    out.field("ok", true);

    out.beginArray("roads");

    for (size_t i = 0; i < snap.roads.size(); i++) {
        const RoadState& r = snap.roads[i];

        out.arrayObjectStart()
           .field("roadID", r.roadID)
           .field("roadName", r.roadName)
           .field("vehicleCount", r.vehicleCount)
           .field("trafficDensity", r.trafficDensity)
           .field("waitingTime", r.waitingTime)
           .field("emergencyVehicleCount", r.emergencyVehicleCount)
           .field("emergencyStatus", r.emergencyStatus)
           .field("currentPriority", r.currentPriority)
           .field("signalStatus", r.signalStatus)
           .field("activeStatus", r.activeStatus)
           .endObject();
    }

    out.endArray();

    out.beginObject("priority")
       .field("valid", snap.current.valid)
       .field("roadID", snap.current.roadID)
       .field("roadName", snap.current.roadName)
       .field("priority", snap.current.priority)
       .field("signalStatus", snap.current.signalStatus)
       .field("reason", snap.current.reason)
       .field("errorMessage", snap.current.errorMessage)
       .endObject();

    out.field("simulation", simulationStateToString(snap.simulation));
    out.field("cycle", snap.cycle);
    out.field("cycleIntervalMs", system.getCycleInterval());
    out.field("storage", system.databaseBackend());
    out.endObject();

    return out.str();
}

HttpResponse API::serveStaticFile(const string& path) {
    string relative = (path == "/") ? "/index.html" : path;

    if (relative.find("..") != string::npos) {
        return ErrorHandler::error(400, "Invalid path");
    }

    const string full = frontendDirectory + relative;
    ifstream in(full.c_str(), ios::binary);

    if (!in.is_open()) {
        return ErrorHandler::notFound(path);
    }

    ostringstream content;
    content << in.rdbuf();

    return HttpResponse::file(
        HttpServer::mimeTypeFor(relative),
        content.str()
    );
}

HttpResponse API::handle(const HttpRequest& request) {
    const string& path = request.path;

    if (path.compare(0, 5, "/api/") != 0) {
        if (request.method != "GET") {
            return ErrorHandler::methodNotAllowed(request.method, path);
        }

        return serveStaticFile(path);
    }

    const string group = request.segment(1);

    if (group == "roads" || group == "road") {
        return routeRoads(request);
    }

    if (group == "traffic") {
        return routeTraffic(request);
    }

    if (group == "emergency") {
        return routeEmergency(request);
    }

    if (group == "priority" || group == "cycle") {
        return routePriority(request);
    }

    if (group == "simulation") {
        return routeSimulation(request);
    }

    if (group == "history") {
        return routeHistory(request);
    }

    if (group == "vehicle") {
        return routeVehicle(request);
    }

    if (group == "queue") {
        return routeQueue(request);
    }

    if (group == "health") {
        return routeHealth(request);
    }

    return ErrorHandler::notFound(path);
}

HttpResponse API::routeRoads(const HttpRequest& request) {
    const string group = request.segment(1);

    if (group == "roads") {
        if (request.method != "GET") {
            return ErrorHandler::methodNotAllowed(
                request.method, request.path);
        }

        vector<RoadState> all;
        system.getAllRoads(all);

        JsonWriter out;
        out.beginObject().field("ok", true).beginArray("roads");

        for (size_t i = 0; i < all.size(); i++) {
            const RoadState& r = all[i];

            out.arrayObjectStart()
               .field("roadID", r.roadID)
               .field("roadName", r.roadName)
               .field("vehicleCount", r.vehicleCount)
               .field("trafficDensity", r.trafficDensity)
               .field("waitingTime", r.waitingTime)
               .field("emergencyStatus", r.emergencyStatus)
               .field("currentPriority", r.currentPriority)
               .field("signalStatus", r.signalStatus)
               .field("activeStatus", r.activeStatus)
               .endObject();
        }

        out.endArray().endObject();
        return ErrorHandler::okJson(out.str());
    }

    if (request.method == "POST" && request.segmentCount() == 2) {
        JsonObject body;

        if (!body.parse(request.body)) {
            return ErrorHandler::invalidRequest(body.error());
        }

        const string roadID = body.getString("roadID");
        const string roadName = body.getString("roadName");

        if (roadID.empty()) {
            return ErrorHandler::invalidRequest("roadID is required");
        }

        if (roadName.empty()) {
            return ErrorHandler::invalidRequest("roadName is required");
        }

        int vehicles = 0, density = 0, waiting = 0;

        if (body.has("vehicleCount") &&
            !body.getIntStrict("vehicleCount", vehicles)) {
            return ErrorHandler::invalidRequest(
                "vehicleCount must be a whole number");
        }

        if (body.has("trafficDensity") &&
            !body.getIntStrict("trafficDensity", density)) {
            return ErrorHandler::invalidRequest(
                "trafficDensity must be a whole number");
        }

        if (body.has("waitingTime") &&
            !body.getIntStrict("waitingTime", waiting)) {
            return ErrorHandler::invalidRequest(
                "waitingTime must be a whole number");
        }

        return ErrorHandler::fromResult(
            system.addRoad(roadID, roadName, vehicles, density, waiting));
    }

    if (request.segmentCount() >= 3) {
        const string roadID = request.segment(2);

        if (request.method == "GET") {
            RoadState state;
            OperationResult r = system.getRoad(roadID, state);

            if (!r.ok) {
                return ErrorHandler::fromResult(r);
            }

            return ErrorHandler::okJson(roadToJson(state));
        }

        if (request.method == "DELETE") {
            return ErrorHandler::fromResult(system.removeRoad(roadID));
        }

        if (request.method == "PUT") {
            JsonObject body;

            if (!body.parse(request.body)) {
                return ErrorHandler::invalidRequest(body.error());
            }

            RoadState current;
            OperationResult found = system.getRoad(roadID, current);

            if (!found.ok) {
                return ErrorHandler::fromResult(found);
            }

            int vehicles = current.vehicleCount;
            int density = current.trafficDensity;
            int waiting = current.waitingTime;

            if (body.has("vehicleCount") &&
                !body.getIntStrict("vehicleCount", vehicles)) {
                return ErrorHandler::invalidRequest(
                    "vehicleCount must be a whole number");
            }

            if (body.has("trafficDensity") &&
                !body.getIntStrict("trafficDensity", density)) {
                return ErrorHandler::invalidRequest(
                    "trafficDensity must be a whole number");
            }

            if (body.has("waitingTime") &&
                !body.getIntStrict("waitingTime", waiting)) {
                return ErrorHandler::invalidRequest(
                    "waitingTime must be a whole number");
            }

            return ErrorHandler::fromResult(
                system.updateRoad(roadID, vehicles, density, waiting));
        }

        return ErrorHandler::methodNotAllowed(
            request.method, request.path);
    }

    return ErrorHandler::notFound(request.path);
}

HttpResponse API::routeTraffic(const HttpRequest& request) {
    const string action = request.segment(2);

    if (action == "state") {
        if (request.method != "GET") {
            return ErrorHandler::methodNotAllowed(
                request.method, request.path);
        }

        system.tickIfRunning();

        TrafficStateSnapshot snap = system.getSnapshot();
        return ErrorHandler::okJson(snapshotToJson(snap));
    }

    if (action == "update") {
        if (request.method != "POST") {
            return ErrorHandler::methodNotAllowed(
                request.method, request.path);
        }

        JsonObject body;

        if (!body.parse(request.body)) {
            return ErrorHandler::invalidRequest(body.error());
        }

        const string roadID = body.getString("roadID");

        if (roadID.empty()) {
            return ErrorHandler::invalidRequest("roadID is required");
        }

        int add = 0, remove = 0, density = -1, waiting = -1;

        if (body.has("addVehicles") &&
            !body.getIntStrict("addVehicles", add)) {
            return ErrorHandler::invalidRequest(
                "addVehicles must be a whole number");
        }

        if (body.has("removeVehicles") &&
            !body.getIntStrict("removeVehicles", remove)) {
            return ErrorHandler::invalidRequest(
                "removeVehicles must be a whole number");
        }

        if (body.has("trafficDensity") &&
            !body.getIntStrict("trafficDensity", density)) {
            return ErrorHandler::invalidRequest(
                "trafficDensity must be a whole number");
        }

        if (body.has("waitingTime") &&
            !body.getIntStrict("waitingTime", waiting)) {
            return ErrorHandler::invalidRequest(
                "waitingTime must be a whole number");
        }

        return ErrorHandler::fromResult(
            system.updateTraffic(roadID, add, remove, density, waiting));
    }

    return ErrorHandler::notFound(request.path);
}

HttpResponse API::routeEmergency(const HttpRequest& request) {
    if (request.method != "POST") {
        return ErrorHandler::methodNotAllowed(
            request.method, request.path);
    }

    JsonObject body;

    if (!body.parse(request.body)) {
        return ErrorHandler::invalidRequest(body.error());
    }

    const string roadID = body.getString("roadID");

    if (roadID.empty()) {
        return ErrorHandler::invalidRequest("roadID is required");
    }

    const string action = request.segment(2);

    if (action == "start") {
        const string type = body.getString("type", "Ambulance");

        OperationResult r = system.startEmergency(roadID, type);

        if (!r.ok) {
            return ErrorHandler::fromResult(r);
        }

        PriorityResult decision = system.getCurrentPriority();

        JsonWriter out;
        out.beginObject()
           .field("ok", true)
           .field("message", r.message)
           .beginObject("priority")
               .field("valid", decision.valid)
               .field("roadID", decision.roadID)
               .field("roadName", decision.roadName)
               .field("priority", decision.priority)
               .field("signalStatus", decision.signalStatus)
               .field("reason", decision.reason)
           .endObject()
           .endObject();

        return ErrorHandler::okJson(out.str());
    }

    if (action == "end") {
        return ErrorHandler::fromResult(system.endEmergency(roadID));
    }

    return ErrorHandler::notFound(request.path);
}

HttpResponse API::routePriority(const HttpRequest& request) {
    const string group = request.segment(1);

    if (group == "cycle") {
        if (request.method != "POST") {
            return ErrorHandler::methodNotAllowed(
                request.method, request.path);
        }

        OperationResult r = system.runCycle();

        if (!r.ok) {
            return ErrorHandler::fromResult(r);
        }

        TrafficStateSnapshot snap = system.getSnapshot();
        return ErrorHandler::okJson(snapshotToJson(snap));
    }

    if (request.method != "GET") {
        return ErrorHandler::methodNotAllowed(
            request.method, request.path);
    }

    return ErrorHandler::okJson(
        decisionToJson(system.getCurrentPriority()));
}

HttpResponse API::routeSimulation(const HttpRequest& request) {
    if (request.method != "POST") {
        return ErrorHandler::methodNotAllowed(
            request.method, request.path);
    }

    const string action = request.segment(2);
    OperationResult r;

    if (action == "speed") {
        JsonObject body;

        if (!body.parse(request.body)) {
            return ErrorHandler::invalidRequest(body.error());
        }

        int interval = 0;

        if (!body.getIntStrict("intervalMs", interval)) {
            return ErrorHandler::invalidRequest(
                "intervalMs must be a whole number");
        }

        system.setCycleInterval(interval);
        r = OperationResult::success("Cycle interval set");
    }
    else if (action == "start") {
        r = system.simulationStart();
    }
    else if (action == "pause") {
        r = system.simulationPause();
    }
    else if (action == "resume") {
        r = system.simulationResume();
    }
    else if (action == "reset") {
        r = system.simulationReset();
    }
    else {
        return ErrorHandler::notFound(request.path);
    }

    if (!r.ok) {
        return ErrorHandler::fromResult(r);
    }

    JsonWriter out;
    out.beginObject()
       .field("ok", true)
       .field("message", r.message)
       .field("simulation", simulationStateToString(system.simulationStatus()))
       .field("cycle", system.getCycleCount())
       .endObject();

    return ErrorHandler::okJson(out.str());
}

HttpResponse API::routeHistory(const HttpRequest& request) {
    if (request.method != "GET") {
        return ErrorHandler::methodNotAllowed(
            request.method, request.path);
    }

    const string kind = request.segment(2);
    const string roadID = request.queryParam("roadID");

    int limit = 20;
    const string limitText = request.queryParam("limit");

    if (!limitText.empty()) {
        limit = atoi(limitText.c_str());

        if (limit < 0) {
            limit = 0;
        }
    }

    JsonWriter out;
    out.beginObject().field("ok", true);

    if (kind == "traffic") {
        vector<TrafficRecord> rows;
        OperationResult r = system.getTrafficHistory(roadID, limit, rows);

        if (!r.ok) {
            return ErrorHandler::fromResult(r);
        }

        out.beginArray("records");

        for (size_t i = 0; i < rows.size(); i++) {
            out.arrayObjectStart()
               .field("recordID", rows[i].recordID)
               .field("roadID", rows[i].roadID)
               .field("vehicleCount", rows[i].vehicleCount)
               .field("trafficDensity", rows[i].trafficDensity)
               .field("waitingTime", rows[i].waitingTime)
               .field("timestamp", rows[i].timestamp)
               .endObject();
        }

        out.endArray();
    }
    else if (kind == "signal") {
        vector<SignalRecord> rows;
        OperationResult r = system.getSignalHistory(roadID, limit, rows);

        if (!r.ok) {
            return ErrorHandler::fromResult(r);
        }

        out.beginArray("records");

        for (size_t i = 0; i < rows.size(); i++) {
            out.arrayObjectStart()
               .field("recordID", rows[i].recordID)
               .field("roadID", rows[i].roadID)
               .field("previousSignal", rows[i].previousSignal)
               .field("newSignal", rows[i].newSignal)
               .field("timestamp", rows[i].timestamp)
               .endObject();
        }

        out.endArray();
    }
    else if (kind == "emergency") {
        vector<EmergencyInfo> rows;
        OperationResult r = system.getEmergencyHistory(roadID, rows);

        if (!r.ok) {
            return ErrorHandler::fromResult(r);
        }

        out.beginArray("records");

        for (size_t i = 0; i < rows.size(); i++) {
            out.arrayObjectStart()
               .field("eventID", rows[i].eventID)
               .field("roadID", rows[i].roadID)
               .field("emergencyType", rows[i].emergencyType)
               .field("startTime", rows[i].startTime)
               .field("endTime", rows[i].endTime)
               .field("status", rows[i].status)
               .endObject();
        }

        out.endArray();
    }
    else {
        return ErrorHandler::notFound(request.path);
    }

    out.endObject();
    return ErrorHandler::okJson(out.str());
}

HttpResponse API::routeHealth(const HttpRequest& request) {
    if (request.method != "GET") {
        return ErrorHandler::methodNotAllowed(
            request.method, request.path);
    }

    vector<RoadState> all;
    system.getAllRoads(all);

    JsonWriter out;
    out.beginObject()
       .field("ok", true)
       .field("storage", system.databaseBackend())
       .field("databaseReady", system.isDatabaseReady())
       .field("roadCount", (int)all.size())
       .field("simulation", simulationStateToString(system.simulationStatus()))
       .field("cycle", system.getCycleCount())
       .endObject();

    return ErrorHandler::okJson(out.str());
}

HttpResponse API::routeVehicle(const HttpRequest& request) {
    if (request.method != "GET") {
        return ErrorHandler::methodNotAllowed(
            request.method, request.path);
    }

    const string roadID = request.queryParam("roadID");
    const string vehicleID = request.queryParam("vehicleID");

    if (roadID.empty() || vehicleID.empty()) {
        return ErrorHandler::invalidRequest(
            "roadID and vehicleID are both required");
    }

    const int position = system.findVehiclePosition(roadID, vehicleID);

    JsonWriter out;
    out.beginObject()
       .field("ok", true)
       .field("roadID", roadID)
       .field("vehicleID", vehicleID)
       .field("found", position > 0)
       .field("position", position)
       .endObject();

    return ErrorHandler::okJson(out.str());
}

HttpResponse API::routeQueue(const HttpRequest& request) {
    if (request.method == "GET") {
        const string roadID = request.queryParam("roadID");

        if (roadID.empty()) {
            return ErrorHandler::invalidRequest("roadID is required");
        }

        vector<QueuedVehicle> queue;
        OperationResult r = system.getQueue(roadID, queue);

        if (!r.ok) {
            return ErrorHandler::fromResult(r);
        }

        JsonWriter out;
        out.beginObject()
           .field("ok", true)
           .field("roadID", roadID)
           .field("queued", (int)queue.size())
           .beginArray("vehicles");

        for (size_t i = 0; i < queue.size(); i++) {
            out.arrayObjectStart()
               .field("position", queue[i].position)
               .field("vehicleID", queue[i].vehicleID)
               .field("vehicleType", queue[i].vehicleType)
               .field("emergency", queue[i].emergency)
               .field("arrivalTime", queue[i].arrivalTime)
               .endObject();
        }

        out.endArray().endObject();
        return ErrorHandler::okJson(out.str());
    }

    if (request.method == "POST") {
        JsonObject body;

        if (!body.parse(request.body)) {
            return ErrorHandler::invalidRequest(body.error());
        }

        const string roadID = body.getString("roadID");
        const string vehicleID = body.getString("vehicleID");

        if (roadID.empty()) {
            return ErrorHandler::invalidRequest("roadID is required");
        }

        if (vehicleID.empty()) {
            return ErrorHandler::invalidRequest("vehicleID is required");
        }

        return ErrorHandler::fromResult(
            system.addVehicleToQueue(
                roadID, vehicleID, body.getString("type", "Normal")));
    }

    return ErrorHandler::methodNotAllowed(
        request.method, request.path);
}
