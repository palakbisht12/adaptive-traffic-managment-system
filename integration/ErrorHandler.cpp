#include "ErrorHandler.h"
#include "Json.h"

const int ErrorHandler::BAD_REQUEST;
const int ErrorHandler::NOT_FOUND;
const int ErrorHandler::NOT_ALLOWED;
const int ErrorHandler::CONFLICT;
const int ErrorHandler::SERVER_ERROR;
const int ErrorHandler::UNAVAILABLE;

HttpResponse ErrorHandler::error(int status, const std::string& message) {
    JsonWriter out;
    out.beginObject()
       .field("ok", false)
       .field("error", message)
       .field("code", status)
       .endObject();
    return HttpResponse::json(status, out.str());
}

HttpResponse ErrorHandler::fromResult(const OperationResult& result) {
    if (result.ok) {
        return ok(result.message);
    }
    return error(result.httpStatus, result.message);
}

HttpResponse ErrorHandler::ok(const std::string& message) {
    JsonWriter out;
    out.beginObject()
       .field("ok", true)
       .field("message", message)
       .endObject();
    return HttpResponse::json(200, out.str());
}

HttpResponse ErrorHandler::okJson(const std::string& jsonBody) {
    return HttpResponse::json(200, jsonBody);
}

HttpResponse ErrorHandler::invalidRequest(const std::string& detail) {
    return error(BAD_REQUEST, detail);
}

HttpResponse ErrorHandler::unknownRoad(const std::string& roadID) {
    return error(NOT_FOUND, "Unknown Road ID: " + roadID);
}

HttpResponse ErrorHandler::methodNotAllowed(const std::string& method,
                                            const std::string& path) {
    return error(NOT_ALLOWED, method + " is not allowed on " + path);
}

HttpResponse ErrorHandler::notFound(const std::string& path) {
    return error(NOT_FOUND, "No such endpoint: " + path);
}

HttpResponse ErrorHandler::databaseUnavailable(const std::string& detail) {
    return error(UNAVAILABLE, "Storage unavailable: " + detail);
}
