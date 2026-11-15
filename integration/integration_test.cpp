
#include <iostream>
#include <string>
#include <vector>

#include "Integration.h"
#include "API.h"
#include "RequestHandler.h"
#include "Json.h"

using namespace std;

static int run = 0;
static int passed = 0;

static void group(const string& name) {
    cout << "\n" << name << endl;
}

static void check(const string& name, bool condition) {
    run++;
    if (condition) {
        passed++;
        cout << "  PASS  " << name << endl;
    } else {
        cout << "  FAIL  " << name << endl;
    }
}

static HttpRequest makeRequest(const string& method,
                               const string& path,
                               const string& body = "",
                               const string& query = "") {
    HttpRequest r;
    r.method = method;
    r.path = path;
    r.body = body;
    r.query = query;
    return r;
}

static bool bodyHas(const HttpResponse& response, const string& needle) {
    return response.body.find(needle) != string::npos;
}

int main() {
    cout << "INTEGRATION TESTS (Member 4)" << endl;

    Integration system;
    system.initialise("test_integration_data");

    API api(system, "frontend");
    RequestHandler handler(api, false);

    group("Startup");

    check("storage reports a backend",
          !system.databaseBackend().empty());

    OperationResult seeded = system.seedDemoRoads();
    check("demo roads seeded", seeded.ok);

    vector<RoadState> roads;
    system.getAllRoads(roads);
    check("four roads present", roads.size() == 4);

    group("Shared contract");

    RoadState one;
    check("road readable through the contract",
          system.getRoad("R101", one).ok);
    check("contract carries the ID", one.roadID == "R101");
    check("contract carries the name", !one.roadName.empty());
    check("contract carries a priority", one.currentPriority > 0);
    check("contract carries a signal", !one.signalStatus.empty());

    group("API routing");

    HttpResponse health = handler.process(makeRequest("GET", "/api/health"));
    check("GET /api/health returns 200", health.status == 200);
    check("health reports the backend", bodyHas(health, "storage"));

    HttpResponse list = handler.process(makeRequest("GET", "/api/roads"));
    check("GET /api/roads returns 200", list.status == 200);
    check("road list contains R101", bodyHas(list, "R101"));

    HttpResponse single =
        handler.process(makeRequest("GET", "/api/road/R102"));
    check("GET /api/road/{id} returns 200", single.status == 200);
    check("single road is the one asked for", bodyHas(single, "R102"));

    HttpResponse created = handler.process(makeRequest(
        "POST", "/api/road",
        "{\"roadID\":\"R105\",\"roadName\":\"Mall Road\","
        "\"vehicleCount\":12,\"trafficDensity\":20,\"waitingTime\":4}"));
    check("POST /api/road creates a road", created.status == 200);

    HttpResponse updated = handler.process(makeRequest(
        "PUT", "/api/road/R105", "{\"vehicleCount\":30}"));
    check("PUT /api/road/{id} updates", updated.status == 200);

    RoadState afterUpdate;
    system.getRoad("R105", afterUpdate);
    check("update reached Traffic Intelligence",
          afterUpdate.vehicleCount == 30);

    HttpResponse deleted =
        handler.process(makeRequest("DELETE", "/api/road/R105"));
    check("DELETE /api/road/{id} removes", deleted.status == 200);
    check("deleted road is gone",
          !system.getRoad("R105", afterUpdate).ok);

    group("Error handling (spec 4.9)");

    HttpResponse badJson =
        handler.process(makeRequest("POST", "/api/road", "{not json"));
    check("malformed JSON returns 400", badJson.status == 400);

    HttpResponse noID = handler.process(
        makeRequest("POST", "/api/road", "{\"roadName\":\"X\"}"));
    check("missing roadID returns 400", noID.status == 400);

    HttpResponse duplicate = handler.process(makeRequest(
        "POST", "/api/road",
        "{\"roadID\":\"R101\",\"roadName\":\"Clone\"}"));
    check("duplicate road returns 409", duplicate.status == 409);

    HttpResponse negative = handler.process(makeRequest(
        "POST", "/api/road",
        "{\"roadID\":\"RX\",\"roadName\":\"Bad\",\"vehicleCount\":-5}"));
    check("negative vehicle count returns 400", negative.status == 400);

    HttpResponse badDensity = handler.process(makeRequest(
        "POST", "/api/road",
        "{\"roadID\":\"RY\",\"roadName\":\"Bad\",\"trafficDensity\":300}"));
    check("density above 100 returns 400", badDensity.status == 400);

    HttpResponse notNumber = handler.process(makeRequest(
        "POST", "/api/road",
        "{\"roadID\":\"RZ\",\"roadName\":\"Z\",\"vehicleCount\":\"abc\"}"));
    check("non-numeric field returns 400", notNumber.status == 400);

    HttpResponse unknown =
        handler.process(makeRequest("GET", "/api/road/R999"));
    check("unknown road returns 404", unknown.status == 404);

    HttpResponse noRoute =
        handler.process(makeRequest("GET", "/api/nowhere"));
    check("unknown endpoint returns 404", noRoute.status == 404);

    HttpResponse wrongMethod =
        handler.process(makeRequest("DELETE", "/api/roads"));
    check("wrong method returns 405", wrongMethod.status == 405);

    HttpResponse traversal =
        handler.process(makeRequest("GET", "/../etc/passwd"));
    check("path traversal is blocked", traversal.status >= 400);

    HttpResponse huge = handler.process(
        makeRequest("POST", "/api/road", string(70000, 'x')));
    check("oversized body returns 400", huge.status == 400);

    check("errors were counted", handler.getErrorCount() > 0);

    group("Simulation control");

    HttpResponse pauseFirst =
        handler.process(makeRequest("POST", "/api/simulation/pause"));
    check("pausing a stopped simulation returns 409",
          pauseFirst.status == 409);

    check("start", handler.process(
        makeRequest("POST", "/api/simulation/start")).status == 200);
    check("state is RUNNING",
          system.simulationStatus() == SimulationState::RUNNING);

    check("pause", handler.process(
        makeRequest("POST", "/api/simulation/pause")).status == 200);
    check("state is PAUSED",
          system.simulationStatus() == SimulationState::PAUSED);

    check("resume", handler.process(
        makeRequest("POST", "/api/simulation/resume")).status == 200);
    check("reset", handler.process(
        makeRequest("POST", "/api/simulation/reset")).status == 200);
    check("state is STOPPED after reset",
          system.simulationStatus() == SimulationState::STOPPED);
    check("cycle counter reset", system.getCycleCount() == 0);

    group("SCENARIO 1 — normal traffic end to end");

    HttpResponse snapshot =
        handler.process(makeRequest("GET", "/api/traffic/state"));
    check("traffic state returns 200", snapshot.status == 200);
    check("snapshot carries roads", bodyHas(snapshot, "\"roads\""));
    check("snapshot carries a decision", bodyHas(snapshot, "\"priority\""));
    check("snapshot carries a reason", bodyHas(snapshot, "\"reason\""));
    check("snapshot carries the simulation state",
          bodyHas(snapshot, "\"simulation\""));

    PriorityResult decision = system.getCurrentPriority();
    check("a road is selected", decision.valid);
    check("selection has a reason", !decision.reason.empty());
    check("selected road is GREEN", decision.signalStatus == "GREEN");

    int greens = 0;
    system.getAllRoads(roads);
    for (size_t i = 0; i < roads.size(); i++) {
        if (roads[i].signalStatus == "GREEN") {
            greens++;
        }
    }
    check("exactly one road is GREEN", greens == 1);

    group("SCENARIO 2 — emergency overrides a heavy jam");

    PriorityResult before = system.getCurrentPriority();
    check("the jam holds GREEN first", before.roadID == "R102");

    HttpResponse emergency = handler.process(makeRequest(
        "POST", "/api/emergency/start",
        "{\"roadID\":\"R104\",\"type\":\"Ambulance\"}"));
    check("emergency accepted", emergency.status == 200);
    check("response carries the new decision",
          bodyHas(emergency, "\"priority\""));

    PriorityResult during = system.getCurrentPriority();
    check("AMBULANCE ON THE QUIETEST ROAD TAKES GREEN",
          during.roadID == "R104");
    check("reason names the emergency",
          during.reason.find("Emergency") != string::npos);
    check("emergency road outscores the jam", during.priority > 1000);

    RoadState jam;
    system.getRoad("R102", jam);
    check("the jam is now RED", jam.signalStatus == "RED");

    group("SCENARIO 3 — traffic increase changes the selection");

    handler.process(makeRequest(
        "POST", "/api/emergency/end", "{\"roadID\":\"R104\"}"));

    PriorityResult beforeSurge = system.getCurrentPriority();

    HttpResponse surge = handler.process(makeRequest(
        "POST", "/api/traffic/update",
        "{\"roadID\":\"R103\",\"addVehicles\":500,"
        "\"trafficDensity\":100,\"waitingTime\":200}"));
    check("traffic update accepted", surge.status == 200);

    PriorityResult afterSurge = system.getCurrentPriority();
    check("the surging road is now selected", afterSurge.roadID == "R103");
    check("the selection actually changed",
          beforeSurge.roadID != afterSurge.roadID);

    RoadState surged;
    system.getRoad("R103", surged);
    check("vehicle count reached the database layer",
          surged.vehicleCount >= 500);

    group("SCENARIO 4 — waiting road is eventually served");

    system.simulationReset();

    vector<string> everGreen;
    for (int cycle = 0; cycle < 40; cycle++) {
        system.runCycle();

        PriorityResult d = system.getSnapshot().current;
        if (!d.valid) {
            continue;
        }
        bool seen = false;
        for (size_t i = 0; i < everGreen.size(); i++) {
            if (everGreen[i] == d.roadID) {
                seen = true;
            }
        }
        if (!seen) {
            everGreen.push_back(d.roadID);
        }
    }
    check("cycles ran", system.getCycleCount() == 40);
    check("more than one road received GREEN over 40 cycles",
          everGreen.size() >= 2);
    cout << "        roads served: ";
    for (size_t i = 0; i < everGreen.size(); i++) {
        cout << everGreen[i] << " ";
    }
    cout << endl;

    group("SCENARIO 5 — new emergency while another road waits");

    RoadState waiting;
    system.getRoad("R102", waiting);
    const int waitingBefore = waiting.waitingTime;

    handler.process(makeRequest(
        "POST", "/api/emergency/start", "{\"roadID\":\"R101\"}"));

    PriorityResult interrupted = system.getCurrentPriority();
    check("the new emergency takes GREEN", interrupted.roadID == "R101");

    system.getRoad("R102", waiting);
    check("the waiting road is still tracked",
          waiting.roadID == "R102" && waiting.waitingTime >= waitingBefore);
    check("the waiting road is RED", waiting.signalStatus == "RED");

    handler.process(makeRequest(
        "POST", "/api/emergency/end", "{\"roadID\":\"R101\"}"));

    RoadState cleared;
    system.getRoad("R101", cleared);
    check("the emergency flag is cleared", !cleared.emergencyStatus);

    PriorityResult afterEmergency = system.getCurrentPriority();
    check("control returns to a non-emergency road",
          afterEmergency.reason.find("Emergency") == string::npos);

    group("Synchronisation (spec 4.8)");

    PriorityResult live = system.getCurrentPriority();

    vector<RoadState> fromController;
    system.getAllRoads(fromController);

    bool consistent = true;
    for (size_t i = 0; i < fromController.size(); i++) {
        if (fromController[i].roadID == live.roadID) {
            if (fromController[i].signalStatus != "GREEN") {
                consistent = false;
            }
        } else if (fromController[i].signalStatus == "GREEN") {
            consistent = false;
        }
    }
    check("the selected road, and only it, is GREEN", consistent);

    if (system.isDatabaseReady()) {
        vector<TrafficRecord> stored;
        system.getTrafficHistory("", 5, stored);
        check("traffic history was persisted", !stored.empty());

        vector<SignalRecord> signalRows;
        system.getSignalHistory("", 5, signalRows);
        check("signal changes were persisted", !signalRows.empty());

        vector<EmergencyInfo> events;
        system.getEmergencyHistory("", events);
        check("emergency events were persisted", !events.empty());
    }

    group("History endpoints");

    HttpResponse th = handler.process(
        makeRequest("GET", "/api/history/traffic", "", "limit=5"));
    check("GET /api/history/traffic returns 200", th.status == 200);
    check("traffic history has records", bodyHas(th, "\"records\""));

    HttpResponse sh = handler.process(
        makeRequest("GET", "/api/history/signal", "", "limit=5"));
    check("GET /api/history/signal returns 200", sh.status == 200);

    HttpResponse eh =
        handler.process(makeRequest("GET", "/api/history/emergency"));
    check("GET /api/history/emergency returns 200", eh.status == 200);

    HttpResponse badKind =
        handler.process(makeRequest("GET", "/api/history/nonsense"));
    check("unknown history type returns 404", badKind.status == 404);

    system.shutdown();

    cout << "\n----------------------------------------" << endl;
    cout << "Requests processed: " << handler.getRequestCount()
         << "  (" << handler.getErrorCount() << " errors, expected)"
         << endl;
    cout << passed << " / " << run << " tests passed." << endl;

    return (passed == run) ? 0 : 1;
}
