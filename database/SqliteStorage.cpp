
#include "SqliteStorage.h"

#ifdef USE_SQLITE

#include <sstream>

using namespace std;

static string textColumn(sqlite3_stmt* stmt, int col) {
    const unsigned char* raw = sqlite3_column_text(stmt, col);
    return raw ? string(reinterpret_cast<const char*>(raw)) : string();
}

SqliteStorage::SqliteStorage() {
    db = 0;
    opened = false;
}

SqliteStorage::~SqliteStorage() {
    close();
}

string SqliteStorage::sqlError() const {
    return db ? string(sqlite3_errmsg(db)) : string("no connection");
}

bool SqliteStorage::exec(const string& sql, string& error) {
    if (!db) {
        error = "Database is not connected";
        return false;
    }

    char* message = 0;

    if (sqlite3_exec(db, sql.c_str(), 0, 0, &message) != SQLITE_OK) {
        error = message ? string(message) : sqlError();

        if (message) {
            sqlite3_free(message);
        }

        return false;
    }

    return true;
}

bool SqliteStorage::prepare(const string& sql, sqlite3_stmt** stmt,
                            string& error) {
    if (!db) {
        error = "Database is not connected";
        return false;
    }

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, stmt, 0) != SQLITE_OK) {
        error = sqlError();
        return false;
    }

    return true;
}

bool SqliteStorage::open(const string& target, string& error) {
    string path = target;

    if (path.size() < 3 || path.substr(path.size() - 3) != ".db") {
        path += ".db";
    }

    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        error = sqlError();
        sqlite3_close(db);
        db = 0;
        opened = false;
        return false;
    }

    if (!exec("PRAGMA foreign_keys = ON;", error)) {
        return false;
    }

    opened = true;
    return createSchema(error);
}

void SqliteStorage::close() {
    if (db) {
        sqlite3_close(db);
        db = 0;
    }

    opened = false;
}

bool SqliteStorage::isOpen() const {
    return opened;
}

string SqliteStorage::backendName() const {
    return "SQLite";
}

bool SqliteStorage::createSchema(string& error) {
    const char* ddl =
        "CREATE TABLE IF NOT EXISTS roads ("
        " road_id TEXT PRIMARY KEY,"
        " road_name TEXT NOT NULL,"
        " vehicle_count INTEGER NOT NULL DEFAULT 0 CHECK (vehicle_count >= 0),"
        " traffic_density INTEGER NOT NULL DEFAULT 0 CHECK (traffic_density BETWEEN 0 AND 100),"
        " waiting_time INTEGER NOT NULL DEFAULT 0 CHECK (waiting_time >= 0),"
        " emergency_status INTEGER NOT NULL DEFAULT 0,"
        " current_priority INTEGER NOT NULL DEFAULT 0,"
        " signal_status TEXT NOT NULL DEFAULT 'RED' CHECK (signal_status IN ('RED','YELLOW','GREEN')),"
        " active_status INTEGER NOT NULL DEFAULT 1);"

        "CREATE TABLE IF NOT EXISTS traffic_history ("
        " record_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " road_id TEXT NOT NULL,"
        " vehicle_count INTEGER NOT NULL,"
        " traffic_density INTEGER NOT NULL,"
        " waiting_time INTEGER NOT NULL,"
        " timestamp TEXT NOT NULL,"
        " FOREIGN KEY (road_id) REFERENCES roads(road_id) ON DELETE CASCADE);"

        "CREATE TABLE IF NOT EXISTS signal_history ("
        " record_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " road_id TEXT NOT NULL,"
        " previous_signal TEXT NOT NULL,"
        " new_signal TEXT NOT NULL,"
        " timestamp TEXT NOT NULL,"
        " FOREIGN KEY (road_id) REFERENCES roads(road_id) ON DELETE CASCADE);"

        "CREATE TABLE IF NOT EXISTS emergency_events ("
        " event_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " road_id TEXT NOT NULL,"
        " emergency_type TEXT NOT NULL DEFAULT 'Ambulance',"
        " start_time TEXT NOT NULL,"
        " end_time TEXT NOT NULL DEFAULT '',"
        " status TEXT NOT NULL DEFAULT 'ACTIVE' CHECK (status IN ('ACTIVE','COMPLETED')),"
        " FOREIGN KEY (road_id) REFERENCES roads(road_id) ON DELETE CASCADE);"

        "CREATE INDEX IF NOT EXISTS idx_traffic_road ON traffic_history (road_id);"
        "CREATE INDEX IF NOT EXISTS idx_signal_road ON signal_history (road_id);"
        "CREATE INDEX IF NOT EXISTS idx_emergency_road ON emergency_events (road_id);"
        "CREATE INDEX IF NOT EXISTS idx_emergency_live ON emergency_events (status);";

    return exec(ddl, error);
}

bool SqliteStorage::insertRoad(const RoadState& road, string& error) {
    sqlite3_stmt* stmt = 0;

    const char* sql =
        "INSERT INTO roads (road_id, road_name, vehicle_count, traffic_density,"
        " waiting_time, emergency_status, current_priority, signal_status,"
        " active_status) VALUES (?,?,?,?,?,?,?,?,?);";

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, road.roadID.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, road.roadName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, road.vehicleCount);
    sqlite3_bind_int(stmt, 4, road.trafficDensity);
    sqlite3_bind_int(stmt, 5, road.waitingTime);
    sqlite3_bind_int(stmt, 6, road.emergencyVehicleCount);
    sqlite3_bind_int(stmt, 7, road.currentPriority);
    sqlite3_bind_text(stmt, 8, road.signalStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 9, road.activeStatus ? 1 : 0);

    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        if (rc == SQLITE_CONSTRAINT) {
            error = "Duplicate Road ID: " + road.roadID;
        } else {
            error = sqlError();
        }

        return false;
    }

    return true;
}

bool SqliteStorage::updateRoad(const RoadState& road, string& error) {
    sqlite3_stmt* stmt = 0;

    const char* sql =
        "UPDATE roads SET road_name=?, vehicle_count=?, traffic_density=?,"
        " waiting_time=?, emergency_status=?, current_priority=?,"
        " signal_status=?, active_status=? WHERE road_id=?;";

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, road.roadName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, road.vehicleCount);
    sqlite3_bind_int(stmt, 3, road.trafficDensity);
    sqlite3_bind_int(stmt, 4, road.waitingTime);
    sqlite3_bind_int(stmt, 5, road.emergencyVehicleCount);
    sqlite3_bind_int(stmt, 6, road.currentPriority);
    sqlite3_bind_text(stmt, 7, road.signalStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, road.activeStatus ? 1 : 0);
    sqlite3_bind_text(stmt, 9, road.roadID.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        error = sqlError();
        return false;
    }

    if (sqlite3_changes(db) == 0) {
        error = "Road not found: " + road.roadID;
        return false;
    }

    return true;
}

bool SqliteStorage::deleteRoad(const string& roadID, string& error) {
    sqlite3_stmt* stmt = 0;

    if (!prepare("DELETE FROM roads WHERE road_id=?;", &stmt, error)) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, roadID.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        error = sqlError();
        return false;
    }

    if (sqlite3_changes(db) == 0) {
        error = "Road not found: " + roadID;
        return false;
    }

    return true;
}

bool SqliteStorage::selectRoad(const string& roadID, RoadState& out,
                               string& error) {
    sqlite3_stmt* stmt = 0;

    const char* sql =
        "SELECT road_id, road_name, vehicle_count, traffic_density,"
        " waiting_time, emergency_status, current_priority, signal_status,"
        " active_status FROM roads WHERE road_id=?;";

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, roadID.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out.roadID = textColumn(stmt, 0);
        out.roadName = textColumn(stmt, 1);
        out.vehicleCount = sqlite3_column_int(stmt, 2);
        out.trafficDensity = sqlite3_column_int(stmt, 3);
        out.waitingTime = sqlite3_column_int(stmt, 4);
        out.emergencyVehicleCount = sqlite3_column_int(stmt, 5);
        out.emergencyStatus = out.emergencyVehicleCount > 0;
        out.currentPriority = sqlite3_column_int(stmt, 6);
        out.signalStatus = textColumn(stmt, 7);
        out.activeStatus = sqlite3_column_int(stmt, 8) != 0;
        found = true;
    }

    sqlite3_finalize(stmt);

    if (!found) {
        error = "Road not found: " + roadID;
    }

    return found;
}

bool SqliteStorage::selectAllRoads(vector<RoadState>& out, string& error) {
    out.clear();

    sqlite3_stmt* stmt = 0;

    const char* sql =
        "SELECT road_id, road_name, vehicle_count, traffic_density,"
        " waiting_time, emergency_status, current_priority, signal_status,"
        " active_status FROM roads ORDER BY road_id;";

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        RoadState r;

        r.roadID = textColumn(stmt, 0);
        r.roadName = textColumn(stmt, 1);
        r.vehicleCount = sqlite3_column_int(stmt, 2);
        r.trafficDensity = sqlite3_column_int(stmt, 3);
        r.waitingTime = sqlite3_column_int(stmt, 4);
        r.emergencyVehicleCount = sqlite3_column_int(stmt, 5);
        r.emergencyStatus = r.emergencyVehicleCount > 0;
        r.currentPriority = sqlite3_column_int(stmt, 6);
        r.signalStatus = textColumn(stmt, 7);
        r.activeStatus = sqlite3_column_int(stmt, 8) != 0;

        out.push_back(r);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SqliteStorage::insertTrafficRecord(const TrafficRecord& rec,
                                        string& error) {
    sqlite3_stmt* stmt = 0;

    const char* sql =
        "INSERT INTO traffic_history (road_id, vehicle_count, traffic_density,"
        " waiting_time, timestamp) VALUES (?,?,?,?,?);";

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, rec.roadID.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, rec.vehicleCount);
    sqlite3_bind_int(stmt, 3, rec.trafficDensity);
    sqlite3_bind_int(stmt, 4, rec.waitingTime);
    sqlite3_bind_text(stmt, 5, rec.timestamp.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        error = sqlError();
        return false;
    }

    return true;
}

bool SqliteStorage::selectTrafficHistory(const string& roadID, int limit,
                                         vector<TrafficRecord>& out,
                                         string& error) {
    out.clear();

    string sql =
        "SELECT record_id, road_id, vehicle_count, traffic_density,"
        " waiting_time, timestamp FROM traffic_history";

    if (!roadID.empty()) {
        sql += " WHERE road_id=?";
    }

    sql += " ORDER BY record_id DESC";

    if (limit > 0) {
        sql += " LIMIT ?";
    }

    sql += ";";

    sqlite3_stmt* stmt = 0;

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    int bindIndex = 1;

    if (!roadID.empty()) {
        sqlite3_bind_text(stmt, bindIndex++, roadID.c_str(), -1,
                          SQLITE_TRANSIENT);
    }

    if (limit > 0) {
        sqlite3_bind_int(stmt, bindIndex++, limit);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TrafficRecord rec;

        rec.recordID = sqlite3_column_int(stmt, 0);
        rec.roadID = textColumn(stmt, 1);
        rec.vehicleCount = sqlite3_column_int(stmt, 2);
        rec.trafficDensity = sqlite3_column_int(stmt, 3);
        rec.waitingTime = sqlite3_column_int(stmt, 4);
        rec.timestamp = textColumn(stmt, 5);

        out.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SqliteStorage::insertSignalRecord(const SignalRecord& rec,
                                       string& error) {
    sqlite3_stmt* stmt = 0;

    const char* sql =
        "INSERT INTO signal_history (road_id, previous_signal, new_signal,"
        " timestamp) VALUES (?,?,?,?);";

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, rec.roadID.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, rec.previousSignal.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, rec.newSignal.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, rec.timestamp.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        error = sqlError();
        return false;
    }

    return true;
}

bool SqliteStorage::selectSignalHistory(const string& roadID, int limit,
                                        vector<SignalRecord>& out,
                                        string& error) {
    out.clear();

    string sql =
        "SELECT record_id, road_id, previous_signal, new_signal, timestamp"
        " FROM signal_history";

    if (!roadID.empty()) {
        sql += " WHERE road_id=?";
    }

    sql += " ORDER BY record_id DESC";

    if (limit > 0) {
        sql += " LIMIT ?";
    }

    sql += ";";

    sqlite3_stmt* stmt = 0;

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    int bindIndex = 1;

    if (!roadID.empty()) {
        sqlite3_bind_text(stmt, bindIndex++, roadID.c_str(), -1,
                          SQLITE_TRANSIENT);
    }

    if (limit > 0) {
        sqlite3_bind_int(stmt, bindIndex++, limit);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SignalRecord rec;

        rec.recordID = sqlite3_column_int(stmt, 0);
        rec.roadID = textColumn(stmt, 1);
        rec.previousSignal = textColumn(stmt, 2);
        rec.newSignal = textColumn(stmt, 3);
        rec.timestamp = textColumn(stmt, 4);

        out.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SqliteStorage::insertEmergency(const EmergencyInfo& ev, int& newID,
                                    string& error) {
    sqlite3_stmt* stmt = 0;

    const char* sql =
        "INSERT INTO emergency_events (road_id, emergency_type, start_time,"
        " end_time, status) VALUES (?,?,?,?,?);";

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, ev.roadID.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, ev.emergencyType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, ev.startTime.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, ev.endTime.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, ev.status.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        error = sqlError();
        return false;
    }

    newID = (int)sqlite3_last_insert_rowid(db);
    return true;
}

bool SqliteStorage::completeEmergency(const string& roadID,
                                      const string& endTime,
                                      string& error) {
    sqlite3_stmt* stmt = 0;

    const char* sql =
        "UPDATE emergency_events SET end_time=?, status='COMPLETED'"
        " WHERE road_id=? AND status='ACTIVE';";

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, endTime.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, roadID.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        error = sqlError();
        return false;
    }

    if (sqlite3_changes(db) == 0) {
        error = "No active emergency on " + roadID;
        return false;
    }

    return true;
}

bool SqliteStorage::selectEmergencies(const string& roadID, bool onlyActive,
                                      vector<EmergencyInfo>& out,
                                      string& error) {
    out.clear();

    string sql =
        "SELECT event_id, road_id, emergency_type, start_time, end_time, status"
        " FROM emergency_events WHERE 1=1";

    if (!roadID.empty()) {
        sql += " AND road_id=?";
    }

    if (onlyActive) {
        sql += " AND status='ACTIVE'";
    }

    sql += " ORDER BY event_id;";

    sqlite3_stmt* stmt = 0;

    if (!prepare(sql, &stmt, error)) {
        return false;
    }

    if (!roadID.empty()) {
        sqlite3_bind_text(stmt, 1, roadID.c_str(), -1, SQLITE_TRANSIENT);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        EmergencyInfo ev;

        ev.eventID = sqlite3_column_int(stmt, 0);
        ev.roadID = textColumn(stmt, 1);
        ev.emergencyType = textColumn(stmt, 2);
        ev.startTime = textColumn(stmt, 3);
        ev.endTime = textColumn(stmt, 4);
        ev.status = textColumn(stmt, 5);

        out.push_back(ev);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SqliteStorage::clearAll(string& error) {
    return exec("DELETE FROM emergency_events;"
                "DELETE FROM signal_history;"
                "DELETE FROM traffic_history;"
                "DELETE FROM roads;", error);
}

#endif
