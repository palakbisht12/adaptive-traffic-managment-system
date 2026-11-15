#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <string>
#include "HttpServer.h"
#include "API.h"

class RequestHandler {
private:
    API& api;
    bool verbose;
    long requestCount;
    long errorCount;

    std::string lastSignature;
    int repeatCount;

    static std::string signatureOf(const HttpRequest& request);
    bool validate(const HttpRequest& request, HttpResponse& rejection);
    void log(const HttpRequest& request, const HttpResponse& response);

public:
    RequestHandler(API& apiLayer, bool verboseLogging);

    HttpResponse process(const HttpRequest& request);

    long getRequestCount() const;
    long getErrorCount() const;
    void setVerbose(bool value);

    static HttpResponse dispatch(const HttpRequest& request, void* context);
};

#endif
