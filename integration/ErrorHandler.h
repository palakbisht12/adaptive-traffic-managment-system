
#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <string>
#include "traffic.h"
#include "HttpServer.h"

using namespace std;

class ErrorHandler {
public:
    static const int BAD_REQUEST = 400;
    static const int NOT_FOUND = 404;
    static const int NOT_ALLOWED = 405;
    static const int CONFLICT = 409;
    static const int SERVER_ERROR = 500;
    static const int UNAVAILABLE = 503;

    static HttpResponse error(int status, const string& message);
    static HttpResponse fromResult(const OperationResult& result);

    static HttpResponse ok(const string& message);
    static HttpResponse okJson(const string& jsonBody);

    static HttpResponse invalidRequest(const string& detail);
    static HttpResponse unknownRoad(const string& roadID);

    static HttpResponse methodNotAllowed(const string& method,
                                         const string& path);

    static HttpResponse notFound(const string& path);
    static HttpResponse databaseUnavailable(const string& detail);
};

#endif
