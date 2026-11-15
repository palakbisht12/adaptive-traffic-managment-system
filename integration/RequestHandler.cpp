#include "RequestHandler.h"
#include "ErrorHandler.h"

#include <iostream>

RequestHandler::RequestHandler(API& apiLayer, bool verboseLogging)
    : api(apiLayer) {
    verbose = verboseLogging;
    requestCount = 0;
    errorCount = 0;
    repeatCount = 0;
}

std::string RequestHandler::signatureOf(const HttpRequest& request) {
    return request.method + " " + request.path + " " + request.body;
}

bool RequestHandler::validate(const HttpRequest& request,
                              HttpResponse& rejection) {
    if (request.method.empty() || request.path.empty()) {
        rejection = ErrorHandler::invalidRequest("Malformed request line");
        return false;
    }
    if (request.path[0] != '/') {
        rejection = ErrorHandler::invalidRequest("Path must start with '/'");
        return false;
    }

    
    if (request.body.size() > 64 * 1024) {
        rejection = ErrorHandler::invalidRequest("Request body is too large");
        return false;
    }

    const bool needsBody =
        (request.method == "POST" || request.method == "PUT");
    if (needsBody && request.path.compare(0, 5, "/api/") == 0) {
        
        if (!request.body.empty() &&
            request.body.find('{') == std::string::npos) {
            rejection =
                ErrorHandler::invalidRequest("Body must be a JSON object");
            return false;
        }
    }
    return true;
}

void RequestHandler::log(const HttpRequest& request,
                         const HttpResponse& response) {
    if (!verbose) {
        return;
    }
    std::cout << "  " << response.status << "  "
              << request.method << " " << request.path;
    if (!request.query.empty()) {
        std::cout << "?" << request.query;
    }
    std::cout << std::endl;
}

HttpResponse RequestHandler::process(const HttpRequest& request) {
    requestCount++;

    HttpResponse rejection;
    if (!validate(request, rejection)) {
        errorCount++;
        log(request, rejection);
        return rejection;
    }

   
    const std::string signature = signatureOf(request);
    if (signature == lastSignature && request.method != "GET") {
        repeatCount++;
    } else {
        repeatCount = 0;
    }
    lastSignature = signature;

    HttpResponse response;

    
    try {
        response = api.handle(request);
    } catch (const std::exception& e) {
        response = ErrorHandler::error(500, std::string("Unhandled error: ") +
                                                e.what());
    } catch (...) {
        response = ErrorHandler::error(500, "Unhandled error in request "
                                            "processing");
    }

    if (response.status >= 400) {
        errorCount++;
    }
    log(request, response);
    return response;
}

long RequestHandler::getRequestCount() const { return requestCount; }
long RequestHandler::getErrorCount() const { return errorCount; }
void RequestHandler::setVerbose(bool value) { verbose = value; }

HttpResponse RequestHandler::dispatch(const HttpRequest& request,
                                      void* context) {
    RequestHandler* self = static_cast<RequestHandler*>(context);
    if (self == 0) {
        return ErrorHandler::error(500, "Request handler is not initialised");
    }
    return self->process(request);
}
