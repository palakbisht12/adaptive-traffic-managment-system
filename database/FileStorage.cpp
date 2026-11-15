#include "FileStorage.h"

#include <fstream>
#include <sstream>
#include <cstdio>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(p) _mkdir(p)
#else
#include <sys/types.h>
#define MAKE_DIR(p) mkdir(p, 0755)
#endif

using namespace std;

static const char FIELD_SEP = '\x1F';

static const char* T_ROADS = "roads.tbl";
static const char* T_TRAFFIC = "traffic_history.tbl";
static const char* T_SIGNAL = "signal_history.tbl";
static const char* T_EMERGENCY = "emergency_events.tbl";

static string toStr(int value) {
    ostringstream out;
    out << value;
    return out.str();
}

static int toInt(const string& text) {
    if (text.empty()) {
        return 0;
    }
    istringstream in(text);
    int value = 0;
    in >> value;
    return in.fail() ? 0 : value;
}

FileStorage::FileStorage() {
    opened = false;
    nextTrafficID = 1;
    nextSignalID = 1;
    nextEmergencyID = 1;
}

FileStorage::~FileStorage() {
    close();
}

string FileStorage::pathFor(const string& table) const {
    return directory + "/" + table;
}

vector<string> FileStorage::splitLine(const string& line) {
    vector<string> fields;
    string current;

    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == FIELD_SEP) {
            fields.push_back(current);
            current.clear();
        } else {
            current += line[i];
        }
    }
    fields.push_back(current);
    return fields;
}

string FileStorage::joinFields(const vector<string>& fields) {
    string out;
    for (size_t i = 0; i < fields.size(); i++) {
        if (i > 0) {
            out += FIELD_SEP;
        }
        out += fields[i];
    }
    return out;
}

bool FileStorage::readLines(const string& table,
                            vector<vector<string> >& rows,
                            string& error) const {
    rows.clear();

    ifstream in(pathFor(table).c_str());
    if (!in.is_open()) {
        return true;
    }

    string line;
    while (getline(in, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        if (!line.empty()) {
            rows.push_back(splitLine(line));
        }
    }

    if (in.bad()) {
        error = "Read failed on " + table;
        return false;
    }
    return true;
}

bool FileStorage::writeLines(const string& table,
                             const vector<vector<string> >& rows,
                             string& error) const {
    const string finalPath = pathFor(table);
    const string tempPath = finalPath + ".tmp";

    {
        ofstream out(tempPath.c_str(), ios::trunc);
        if (!out.is_open()) {
            error = "Cannot write " + table;
            return false;
        }
        for (size_t i = 0; i < rows.size(); i++) {
            out << joinFields(rows[i]) << "\n";
        }
        out.flush();
        if (!out.good()) {
            error = "Write failed on " + table;
            return false;
        }
    }

    remove(finalPath.c_str());
    if (rename(tempPath.c_str(), finalPath.c_str()) != 0) {
        error = "Could not replace " + table;
        return false;
    }
    return true;
}

bool FileStorage::appendLine(const string& table,
                             const vector<string>& fields,
                             string& error) const {
    ofstream out(pathFor(table).c_str(), ios::app);
    if (!out.is_open()) {
        error = "Cannot append to " + table;
        return false;
    }
    out << joinFields(fields) << "\n";
    out.flush();

    if (!out.good()) {
        error = "Append failed on " + table;
        return false;
    }
    return true;
}

void FileStorage::recoverCounters() {
    string error;
    vector<vector<string> > rows;

    if (readLines(T_TRAFFIC, rows, error)) {
        for (size_t i = 0; i < rows.size(); i++) {
            int id = toInt(rows[i][0]);
            if (id >= nextTrafficID) {
                nextTrafficID = id + 1;
            }
        }
    }

    if (readLines(T_SIGNAL, rows, error)) {
        for (size_t i = 0; i < rows.size(); i++) {
            int id = toInt(rows[i][0]);
            if (id >= nextSignalID) {
                nextSignalID = id + 1;
            }
        }
    }

    if (readLines(T_EMERGENCY, rows, error)) {
        for (size_t i = 0; i < rows.size(); i++) {
            int id = toInt(rows[i][0]);
            if (id >= nextEmergencyID) {
                nextEmergencyID = id + 1;
            }
        }
    }
}

bool FileStorage::open(const string& target, string& error) {
    directory = target;

    struct stat info;
    if (stat(directory.c_str(), &info) != 0) {
        if (MAKE_DIR(directory.c_str()) != 0) {
            error = "Cannot create data directory: " + directory;
            opened = false;
            return false;
        }
    }

    const string probe = directory + "/.writetest";
    ofstream test(probe.c_str());
    if (!test.is_open()) {
        error = "Data directory is not writable: " + directory;
        opened = false;
        return false;
    }
    test.close();
    remove(probe.c_str());

    opened = true;
    recoverCounters();
    return true;
}

void FileStorage::close() { opened = false; }
bool FileStorage::isOpen() const { return opened; }
string FileStorage::backendName() const { return "FileStorage"; }

static vector<string> roadToFields(const RoadState& r) {
    vector<string> f;
    f.push_back(r.roadID);
    f.push_back(r.roadName);
    f.push_back(toStr(r.vehicleCount));
    f.push_back(toStr(r.trafficDensity));
    f.push_back(toStr(r.waitingTime));
    f.push_back(toStr(r.emergencyVehicleCount));
    f.push_back(r.emergencyStatus ? "1" : "0");
    f.push_back(toStr(r.currentPriority));
    f.push_back(r.signalStatus);
    f.push_back(r.activeStatus ? "1" : "0");
    return f;
}

static RoadState fieldsToRoad(const vector<string>& f) {
    RoadState r;
    if (f.size() < 10) {
        return r;
    }
    r.roadID = f[0];
    r.roadName = f[1];
    r.vehicleCount = toInt(f[2]);
    r.trafficDensity = toInt(f[3]);
    r.waitingTime = toInt(f[4]);
    r.emergencyVehicleCount = toInt(f[5]);
    r.emergencyStatus = (f[6] == "1");
    r.currentPriority = toInt(f[7]);
    r.signalStatus = f[8];
    r.activeStatus = (f[9] == "1");
    return r;
}

bool FileStorage::insertRoad(const RoadState& road, string& error) {
    vector<vector<string> > rows;
    if (!readLines(T_ROADS, rows, error)) {
        return false;
    }

    for (size_t i = 0; i < rows.size(); i++) {
        if (!rows[i].empty() && rows[i][0] == road.roadID) {
            error = "Duplicate Road ID: " + road.roadID;
            return false;
        }
    }

    rows.push_back(roadToFields(road));
    return writeLines(T_ROADS, rows, error);
}

bool FileStorage::updateRoad(const RoadState& road, string& error) {
    vector<vector<string> > rows;
    if (!readLines(T_ROADS, rows, error)) {
        return false;
    }

    bool found = false;
    for (size_t i = 0; i < rows.size(); i++) {
        if (!rows[i].empty() && rows[i][0] == road.roadID) {
            rows[i] = roadToFields(road);
            found = true;
            break;
        }
    }

    if (!found) {
        error = "Road not found: " + road.roadID;
        return false;
    }
    return writeLines(T_ROADS, rows, error);
}

bool FileStorage::deleteRoad(const string& roadID, string& error) {
    vector<vector<string> > rows;
    if (!readLines(T_ROADS, rows, error)) {
        return false;
    }

    vector<vector<string> > kept;
    bool found = false;
    for (size_t i = 0; i < rows.size(); i++) {
        if (!rows[i].empty() && rows[i][0] == roadID) {
            found = true;
        } else {
            kept.push_back(rows[i]);
        }
    }

    if (!found) {
        error = "Road not found: " + roadID;
        return false;
    }
    if (!writeLines(T_ROADS, kept, error)) {
        return false;
    }

    const char* children[3] = { T_TRAFFIC, T_SIGNAL, T_EMERGENCY };
    for (int c = 0; c < 3; c++) {
        vector<vector<string> > crows;
        if (!readLines(children[c], crows, error)) {
            return false;
        }
        vector<vector<string> > ckept;
        for (size_t i = 0; i < crows.size(); i++) {
            if (crows[i].size() < 2 || crows[i][1] != roadID) {
                ckept.push_back(crows[i]);
            }
        }
        if (!writeLines(children[c], ckept, error)) {
            return false;
        }
    }
    return true;
}

bool FileStorage::selectRoad(const string& roadID, RoadState& out,
                             string& error) {
    vector<vector<string> > rows;
    if (!readLines(T_ROADS, rows, error)) {
        return false;
    }

    for (size_t i = 0; i < rows.size(); i++) {
        if (!rows[i].empty() && rows[i][0] == roadID) {
            out = fieldsToRoad(rows[i]);
            return true;
        }
    }
    error = "Road not found: " + roadID;
    return false;
}

bool FileStorage::selectAllRoads(vector<RoadState>& out,
                                 string& error) {
    out.clear();

    vector<vector<string> > rows;
    if (!readLines(T_ROADS, rows, error)) {
        return false;
    }
    for (size_t i = 0; i < rows.size(); i++) {
        out.push_back(fieldsToRoad(rows[i]));
    }
    return true;
}

bool FileStorage::insertTrafficRecord(const TrafficRecord& rec,
                                      string& error) {
    vector<string> f;
    f.push_back(toStr(nextTrafficID++));
    f.push_back(rec.roadID);
    f.push_back(toStr(rec.vehicleCount));
    f.push_back(toStr(rec.trafficDensity));
    f.push_back(toStr(rec.waitingTime));
    f.push_back(rec.timestamp);
    return appendLine(T_TRAFFIC, f, error);
}

bool FileStorage::selectTrafficHistory(const string& roadID, int limit,
                                       vector<TrafficRecord>& out,
                                       string& error) {
    out.clear();

    vector<vector<string> > rows;
    if (!readLines(T_TRAFFIC, rows, error)) {
        return false;
    }

    for (size_t n = rows.size(); n > 0; n--) {
        const vector<string>& f = rows[n - 1];
        if (f.size() < 6) {
            continue;
        }
        if (!roadID.empty() && f[1] != roadID) {
            continue;
        }

        TrafficRecord rec;
        rec.recordID = toInt(f[0]);
        rec.roadID = f[1];
        rec.vehicleCount = toInt(f[2]);
        rec.trafficDensity = toInt(f[3]);
        rec.waitingTime = toInt(f[4]);
        rec.timestamp = f[5];
        out.push_back(rec);

        if (limit > 0 && (int)out.size() >= limit) {
            break;
        }
    }
    return true;
}

bool FileStorage::insertSignalRecord(const SignalRecord& rec,
                                     string& error) {
    vector<string> f;
    f.push_back(toStr(nextSignalID++));
    f.push_back(rec.roadID);
    f.push_back(rec.previousSignal);
    f.push_back(rec.newSignal);
    f.push_back(rec.timestamp);
    return appendLine(T_SIGNAL, f, error);
}

bool FileStorage::selectSignalHistory(const string& roadID, int limit,
                                      vector<SignalRecord>& out,
                                      string& error) {
    out.clear();

    vector<vector<string> > rows;
    if (!readLines(T_SIGNAL, rows, error)) {
        return false;
    }

    for (size_t n = rows.size(); n > 0; n--) {
        const vector<string>& f = rows[n - 1];
        if (f.size() < 5) {
            continue;
        }
        if (!roadID.empty() && f[1] != roadID) {
            continue;
        }

        SignalRecord rec;
        rec.recordID = toInt(f[0]);
        rec.roadID = f[1];
        rec.previousSignal = f[2];
        rec.newSignal = f[3];
        rec.timestamp = f[4];
        out.push_back(rec);

        if (limit > 0 && (int)out.size() >= limit) {
            break;
        }
    }
    return true;
}

bool FileStorage::insertEmergency(const EmergencyInfo& ev, int& newID,
                                  string& error) {
    newID = nextEmergencyID++;

    vector<string> f;
    f.push_back(toStr(newID));
    f.push_back(ev.roadID);
    f.push_back(ev.emergencyType);
    f.push_back(ev.startTime);
    f.push_back(ev.endTime);
    f.push_back(ev.status);
    return appendLine(T_EMERGENCY, f, error);
}

bool FileStorage::completeEmergency(const string& roadID,
                                    const string& endTime,
                                    string& error) {
    vector<vector<string> > rows;
    if (!readLines(T_EMERGENCY, rows, error)) {
        return false;
    }

    bool changed = false;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].size() >= 6 && rows[i][1] == roadID &&
            rows[i][5] == "ACTIVE") {
            rows[i][4] = endTime;
            rows[i][5] = "COMPLETED";
            changed = true;
        }
    }

    if (!changed) {
        error = "No active emergency on " + roadID;
        return false;
    }
    return writeLines(T_EMERGENCY, rows, error);
}

bool FileStorage::selectEmergencies(const string& roadID, bool onlyActive,
                                    vector<EmergencyInfo>& out,
                                    string& error) {
    out.clear();

    vector<vector<string> > rows;
    if (!readLines(T_EMERGENCY, rows, error)) {
        return false;
    }

    for (size_t i = 0; i < rows.size(); i++) {
        const vector<string>& f = rows[i];
        if (f.size() < 6) {
            continue;
        }
        if (!roadID.empty() && f[1] != roadID) {
            continue;
        }
        if (onlyActive && f[5] != "ACTIVE") {
            continue;
        }

        EmergencyInfo ev;
        ev.eventID = toInt(f[0]);
        ev.roadID = f[1];
        ev.emergencyType = f[2];
        ev.startTime = f[3];
        ev.endTime = f[4];
        ev.status = f[5];
        out.push_back(ev);
    }
    return true;
}

bool FileStorage::clearAll(string& error) {
    const char* tables[4] = { T_ROADS, T_TRAFFIC, T_SIGNAL, T_EMERGENCY };
    vector<vector<string> > empty;

    for (int i = 0; i < 4; i++) {
        if (!writeLines(tables[i], empty, error)) {
            return false;
        }
    }
    nextTrafficID = 1;
    nextSignalID = 1;
    nextEmergencyID = 1;
    return true;
}