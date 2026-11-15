#include "Json.h"

#include <sstream>
#include <cctype>
#include <cstdlib>


JsonWriter::JsonWriter() {}

std::string JsonWriter::escape(const std::string& text) {
    std::string out;
    out.reserve(text.size() + 8);

    for (std::size_t i = 0; i < text.size(); i++) {
        const unsigned char c = (unsigned char)text[i];
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    
                    char esc[8];
                    std::snprintf(esc, sizeof(esc), "\\u%04x", c);
                    out += esc;
                } else {
                    out += (char)c;
                }
        }
    }
    return out;
}

void JsonWriter::separate() {
    if (firstInScope.empty()) {
        return;
    }
    if (firstInScope.back()) {
        firstInScope.back() = false;
    } else {
        buffer += ",";
    }
}

JsonWriter& JsonWriter::beginObject() {
    separate();
    buffer += "{";
    firstInScope.push_back(true);
    return *this;
}

JsonWriter& JsonWriter::beginObject(const std::string& key) {
    separate();
    buffer += "\"" + escape(key) + "\":{";
    firstInScope.push_back(true);
    return *this;
}

JsonWriter& JsonWriter::endObject() {
    buffer += "}";
    if (!firstInScope.empty()) {
        firstInScope.pop_back();
    }
    return *this;
}

JsonWriter& JsonWriter::beginArray(const std::string& key) {
    separate();
    buffer += "\"" + escape(key) + "\":[";
    firstInScope.push_back(true);
    return *this;
}

JsonWriter& JsonWriter::endArray() {
    buffer += "]";
    if (!firstInScope.empty()) {
        firstInScope.pop_back();
    }
    return *this;
}

JsonWriter& JsonWriter::arrayObjectStart() {
    separate();
    buffer += "{";
    firstInScope.push_back(true);
    return *this;
}

JsonWriter& JsonWriter::field(const std::string& key,
                              const std::string& value) {
    separate();
    buffer += "\"" + escape(key) + "\":\"" + escape(value) + "\"";
    return *this;
}

JsonWriter& JsonWriter::field(const std::string& key, const char* value) {
    return field(key, std::string(value ? value : ""));
}

JsonWriter& JsonWriter::field(const std::string& key, int value) {
    separate();
    std::ostringstream num;
    num << value;
    buffer += "\"" + escape(key) + "\":" + num.str();
    return *this;
}

JsonWriter& JsonWriter::field(const std::string& key, bool value) {
    separate();
    buffer += "\"" + escape(key) + "\":";
    buffer += (value ? "true" : "false");
    return *this;
}

std::string JsonWriter::str() const { return buffer; }


JsonObject::JsonObject() { valid = false; }

static void skipSpace(const std::string& s, std::size_t& i) {
    while (i < s.size() && std::isspace((unsigned char)s[i])) {
        i++;
    }
}

static bool readString(const std::string& s, std::size_t& i,
                       std::string& out) {
    if (i >= s.size() || s[i] != '"') {
        return false;
    }
    i++;  
    out.clear();

    while (i < s.size()) {
        const char c = s[i];
        if (c == '\\') {
            if (i + 1 >= s.size()) {
                return false;
            }
            const char esc = s[i + 1];
            switch (esc) {
                case 'n':  out += '\n'; break;
                case 't':  out += '\t'; break;
                case 'r':  out += '\r'; break;
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case '/':  out += '/';  break;
                default:   out += esc;  break;
            }
            i += 2;
        } else if (c == '"') {
            i++;   
            return true;
        } else {
            out += c;
            i++;
        }
    }
    return false;   
}

bool JsonObject::parse(const std::string& text) {
    values.clear();
    valid = false;
    parseError.clear();

    std::size_t i = 0;
    skipSpace(text, i);

    if (i >= text.size()) {
        valid = true;
        return true;
    }

    if (text[i] != '{') {
        parseError = "Request body must be a JSON object";
        return false;
    }
    i++;
    skipSpace(text, i);

    if (i < text.size() && text[i] == '}') {
        valid = true;
        return true;
    }

    while (i < text.size()) {
        skipSpace(text, i);

        std::string key;
        if (!readString(text, i, key)) {
            parseError = "Expected a quoted key";
            return false;
        }

        skipSpace(text, i);
        if (i >= text.size() || text[i] != ':') {
            parseError = "Expected ':' after key \"" + key + "\"";
            return false;
        }
        i++;
        skipSpace(text, i);

        std::string value;
        if (i < text.size() && text[i] == '"') {
            if (!readString(text, i, value)) {
                parseError = "Unterminated string for key \"" + key + "\"";
                return false;
            }
        } else {
            const std::size_t start = i;
            while (i < text.size() && text[i] != ',' && text[i] != '}' &&
                   !std::isspace((unsigned char)text[i])) {
                i++;
            }
            if (start == i) {
                parseError = "Missing value for key \"" + key + "\"";
                return false;
            }
            value = text.substr(start, i - start);
        }

        values[key] = value;

        skipSpace(text, i);
        if (i < text.size() && text[i] == ',') {
            i++;
            continue;
        }
        if (i < text.size() && text[i] == '}') {
            valid = true;
            return true;
        }
        if (i >= text.size()) {
            parseError = "Unexpected end of JSON object";
            return false;
        }
        parseError = "Expected ',' or '}'";
        return false;
    }

    parseError = "Unexpected end of JSON object";
    return false;
}

bool JsonObject::isValid() const { return valid; }
std::string JsonObject::error() const { return parseError; }

bool JsonObject::has(const std::string& key) const {
    return values.find(key) != values.end();
}

std::string JsonObject::getString(const std::string& key,
                                  const std::string& fallback) const {
    std::map<std::string, std::string>::const_iterator it = values.find(key);
    return (it == values.end()) ? fallback : it->second;
}

bool JsonObject::getIntStrict(const std::string& key, int& out) const {
    std::map<std::string, std::string>::const_iterator it = values.find(key);
    if (it == values.end() || it->second.empty()) {
        return false;
    }

    const std::string& text = it->second;
    std::size_t pos = 0;
    if (text[0] == '-' || text[0] == '+') {
        pos = 1;
    }
    if (pos >= text.size()) {
        return false;
    }
    for (std::size_t i = pos; i < text.size(); i++) {
        if (!std::isdigit((unsigned char)text[i])) {
            return false;
        }
    }

    out = std::atoi(text.c_str());
    return true;
}

int JsonObject::getInt(const std::string& key, int fallback) const {
    int value = 0;
    return getIntStrict(key, value) ? value : fallback;
}

bool JsonObject::getBool(const std::string& key, bool fallback) const {
    std::map<std::string, std::string>::const_iterator it = values.find(key);
    if (it == values.end()) {
        return fallback;
    }
    return it->second == "true" || it->second == "1";
}
