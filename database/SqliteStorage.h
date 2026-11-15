
#ifndef SQLITESTORAGE_H
#define SQLITESTORAGE_H

#ifdef USE_SQLITE

#include <string>
#include <vector>
#include <sqlite3.h>

#include "Database.h"

using namespace std;

class SqliteStorage : public IStorage {
private:
    sqlite3* db;
    bool opened;

    bool exec(const string& sql, string& error);
    bool prepare(const string& sql, sqlite3_stmt** stmt,
                 string& error);
    string sqlError() const;

public:
    SqliteStorage();
    ~SqliteStorage();

    bool open(const string& target, string& error) override;
    void close() override;
    bool isOpen() const override;
    string backendName() const override;

    bool createSchema(string& error);

    bool insertRoad(const RoadState& road, string& error) override;
    bool updateRoad(const RoadState& road, string& error) override;
    bool deleteRoad(const string& roadID, string& error) override;
    bool selectRoad(const string& roadID, RoadState& out,
                    string& error) override;
    bool selectAllRoads(vector<RoadState>& out,
                        string& error) override;

    bool insertTrafficRecord(const TrafficRecord& rec,
                             string& error) override;
    bool selectTrafficHistory(const string& roadID, int limit,
                              vector<TrafficRecord>& out,
                              string& error) override;

    bool insertSignalRecord(const SignalRecord& rec,
                            string& error) override;
    bool selectSignalHistory(const string& roadID, int limit,
                             vector<SignalRecord>& out,
                             string& error) override;

    bool insertEmergency(const EmergencyInfo& ev, int& newID,
                         string& error) override;
    bool completeEmergency(const string& roadID,
                           const string& endTime,
                           string& error) override;
    bool selectEmergencies(const string& roadID, bool onlyActive,
                           vector<EmergencyInfo>& out,
                           string& error) override;

    bool clearAll(string& error) override;
};

#endif
#endif
