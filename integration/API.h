
#ifndef API_H
#define API_H

#include <string>
#include "HttpServer.h"
#include "Integration.h"

using namespace std;

class API {
private:
    Integration& system;
    string frontendDirectory;

    HttpResponse serveStaticFile(const string& path);

    static string roadToJson(const RoadState& road);
    static string decisionToJson(const PriorityResult& result);
    string snapshotToJson(TrafficStateSnapshot& snap);

    HttpResponse routeRoads(const HttpRequest& request);
    HttpResponse routeTraffic(const HttpRequest& request);
    HttpResponse routeEmergency(const HttpRequest& request);
    HttpResponse routePriority(const HttpRequest& request);
    HttpResponse routeSimulation(const HttpRequest& request);
    HttpResponse routeHistory(const HttpRequest& request);
    HttpResponse routeHealth(const HttpRequest& request);
    HttpResponse routeVehicle(const HttpRequest& request);
    HttpResponse routeQueue(const HttpRequest& request);

public:
    API(Integration& integration, const string& frontendDir);

    void setFrontendDirectory(const string& dir);

    HttpResponse handle(const HttpRequest& request);

    static HttpResponse dispatch(const HttpRequest& request, void* context);
};

#endif
