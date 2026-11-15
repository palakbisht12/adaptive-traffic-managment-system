#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#include <string>
#include <vector>
#include "Database.h"

using namespace std;

class FileStorage : public IStorage {
private:
    string directory;
    bool opened;
    int nextTrafficID;
    int nextSignalID;
    int nextEmergencyID;

    string pathFor(const string& table) const;
    static vector<string> splitLine(const string& line);
    static string joinFields(const vector<string>& fields);

    bool readLines(const string& table,
                   vector<vector<string> >& rows,
                   string& error) const;
    bool writeLines(const string& table,
                    const vector<vector<string> >& rows,
                    string& error) const;
    bool appendLine(const string& table,
                    const vector<string>& fields,
                    string& error) const;

    void recoverCounters();

public:
    FileStorage();
    ~FileStorage();

    bool open(const string& target, string& error) override;
    void close() override;
    bool isOpen() const override;
    string backendName() const override;

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