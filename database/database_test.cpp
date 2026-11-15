#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include "Database.h"
#include "RoadRepository.h"
#include "TrafficRepository.h"
#include "SignalRepository.h"
#include "EmergencyRepository.h"

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

static RoadState makeRoad(const string& id, const string& name,
                          int vehicles, int density, int waiting) {
    RoadState r;
    r.roadID = id;
    r.roadName = name;
    r.vehicleCount = vehicles;
    r.trafficDensity = density;
    r.waitingTime = waiting;
    r.currentPriority = 0;
    r.signalStatus = "RED";
    r.activeStatus = true;
    return r;
}

int main() {
    cout << "DATABASE MODULE TESTS (Member 2)" << endl;

    Database db;

    group("Connection");

   
    RoadState scratch;
    check("insert before connect is refused",
          !db.insertRoad(makeRoad("X", "X", 1, 1, 1)).ok);
    check("read before connect is refused", !db.getRoad("X", scratch).ok);
    check("not connected reports false", !db.isConnected());

    OperationResult conn = db.connect("test_db_data");
    check("connect succeeds", conn.ok);
    check("connected reports true", db.isConnected());
    check("backend is named", !db.getBackendName().empty());
    cout << "        backend: " << db.getBackendName() << endl;

    db.clearAll(); 

    group("Road CRUD");

    RoadRepository roads(db);

    check("add road", roads.addRoad(makeRoad("R1", "Rajpur Road",
                                             40, 55, 12)).ok);
    check("add second road", roads.addRoad(makeRoad("R2", "Mall Road",
                                                    10, 20, 5)).ok);
    check("road count is 2", roads.count() == 2);

    RoadState found;
    check("find road", roads.findRoad("R1", found).ok);
    check("found road has the right name", found.roadName == "Rajpur Road");
    check("found road has the right vehicle count", found.vehicleCount == 40);

    found.vehicleCount = 99;
    found.signalStatus = "GREEN";
    check("update road", roads.updateRoad(found).ok);

    RoadState reread;
    roads.findRoad("R1", reread);
    check("update persisted", reread.vehicleCount == 99 &&
                              reread.signalStatus == "GREEN");

    vector<RoadState> all;
    check("get all roads", roads.getAllRoads(all).ok);
    check("all roads returns 2", all.size() == 2);

    check("exists() is true for a stored road", roads.exists("R1"));
    check("exists() is false for an absent road", !roads.exists("R9"));

    // ---------------------------------------------------------------
    group("Road error handling (spec 2.8)");

    OperationResult dup = roads.addRoad(makeRoad("R1", "Clone", 1, 1, 1));
    check("duplicate Road ID is rejected", !dup.ok);
    check("duplicate returns HTTP 409", dup.httpStatus == 409);
    check("duplicate did not change the count", roads.count() == 2);

    check("empty Road ID is rejected",
          !roads.addRoad(makeRoad("", "Nameless", 1, 1, 1)).ok);
    check("negative vehicle count is rejected",
          !roads.addRoad(makeRoad("R3", "Bad", -5, 10, 10)).ok);
    check("negative waiting time is rejected",
          !roads.addRoad(makeRoad("R4", "Bad", 10, 10, -3)).ok);
    check("density above 100 is rejected",
          !roads.addRoad(makeRoad("R5", "Bad", 10, 300, 10)).ok);

    RoadState missing;
    OperationResult notFound = roads.findRoad("R404", missing);
    check("missing road reports failure", !notFound.ok);
    check("missing road returns HTTP 404", notFound.httpStatus == 404);

    check("updating a non-existent road is rejected",
          !roads.updateRoad(makeRoad("R404", "Ghost", 1, 1, 1)).ok);
    check("deleting a non-existent road is rejected",
          !roads.deleteRoad("R404").ok);

   
    group("Traffic history");

    TrafficRepository traffic(db);

    RoadState r1;
    roads.findRoad("R1", r1);
    check("save traffic update", traffic.saveTrafficUpdate(r1).ok);

    r1.vehicleCount = 55;
    roads.updateRoad(r1);
    traffic.saveTrafficUpdate(r1);

    vector<TrafficRecord> history;
    check("read traffic history", traffic.getTrafficHistory("R1",
                                                            history).ok);
    check("two records stored", history.size() == 2);
    check("newest record comes first", history[0].vehicleCount == 55);

    TrafficRecord latest;
    check("latest traffic data", traffic.getLatestTrafficData("R1",
                                                              latest).ok);
    check("latest is the most recent write", latest.vehicleCount == 55);
    check("latest for a road with no records fails",
          !traffic.getLatestTrafficData("R2", latest).ok);

    vector<TrafficRecord> limited;
    traffic.getTrafficHistory("R1", limited, 1);
    check("limit is honoured", limited.size() == 1);

 
    group("Signal history");

    SignalRepository signals(db);

    check("save signal change", signals.saveSignalChange("R1", "RED",
                                                         "GREEN").ok);
    check("save second change", signals.saveSignalChange("R1", "GREEN",
                                                         "RED").ok);

    // A signal that did not change is not history.
    OperationResult noChange = signals.saveSignalChange("R1", "RED", "RED");
    check("identical signal is not recorded", noChange.ok);

    vector<SignalRecord> signalRows;
    check("read signal history",
          signals.getSignalChangesForRoad("R1", signalRows).ok);
    check("two signal changes stored", signalRows.size() == 2);

    SignalRecord latestSignal;
    check("latest signal", signals.getLatestSignal("R1", latestSignal).ok);
    check("latest signal is the newest", latestSignal.newSignal == "RED");
    check("latest signal for an untouched road fails",
          !signals.getLatestSignal("R2", latestSignal).ok);

   
    group("Emergency events");

    EmergencyRepository emergencies(db);

    int eventID = 0;
    check("add emergency event",
          emergencies.addEmergencyEvent("R1", "Ambulance", eventID).ok);
    check("event got an ID", eventID > 0);
    check("emergency reported active",
          emergencies.hasActiveEmergency("R1"));

  
    int second = 0;
    check("second simultaneous emergency on the same road is refused",
          !emergencies.addEmergencyEvent("R1", "Fire Brigade", second).ok);

    check("emergency on an unknown road is refused",
          !emergencies.addEmergencyEvent("R404", "Ambulance", second).ok);

    vector<EmergencyInfo> active;
    check("read active emergencies",
          emergencies.getActiveEmergencies(active).ok);
    check("one emergency active", active.size() == 1);
    check("active emergency has no end time", active[0].endTime.empty());

    check("mark completed",
          emergencies.markEmergencyCompleted("R1").ok);
    check("no longer active", !emergencies.hasActiveEmergency("R1"));
    check("completing twice is refused",
          !emergencies.markEmergencyCompleted("R1").ok);

    vector<EmergencyInfo> past;
    emergencies.getEmergencyHistory("R1", past);
    check("history keeps the completed event", past.size() == 1);
    check("completed event has an end time", !past[0].endTime.empty());
    check("completed event status is COMPLETED",
          past[0].status == "COMPLETED");

    group("Cascade delete");

    // History must not outlive the road it describes.
    check("delete road with history", roads.deleteRoad("R1").ok);

    vector<TrafficRecord> orphanTraffic;
    traffic.getTrafficHistory("R1", orphanTraffic);
    check("traffic history removed with the road", orphanTraffic.empty());

    vector<SignalRecord> orphanSignals;
    signals.getSignalChangesForRoad("R1", orphanSignals);
    check("signal history removed with the road", orphanSignals.empty());

    vector<EmergencyInfo> orphanEvents;
    emergencies.getEmergencyHistory("R1", orphanEvents);
    check("emergency history removed with the road", orphanEvents.empty());

    check("the other road survived", roads.exists("R2"));

    group("Empty database");

    db.clearAll();

    vector<RoadState> none;
    OperationResult emptyRead = roads.getAllRoads(none);
    check("reading an empty database succeeds", emptyRead.ok);
    check("empty database returns no rows", none.empty());
    check("count on an empty database is 0", roads.count() == 0);

    vector<TrafficRecord> noHistory;
    check("empty history is not an error",
          traffic.getTrafficHistory("", noHistory).ok);
    check("empty history has no rows", noHistory.empty());

    group("Disconnect");

    db.disconnect();
    check("disconnected reports false", !db.isConnected());

    OperationResult afterClose = roads.addRoad(makeRoad("Z", "Z", 1, 1, 1));
    check("write after disconnect is refused", !afterClose.ok);
    check("write after disconnect returns HTTP 503",
          afterClose.httpStatus == 503);

    cout << "\n----------------------------------------" << endl;
    cout << passed << " / " << run << " tests passed." << endl;
    return (passed == run) ? 0 : 1;
}