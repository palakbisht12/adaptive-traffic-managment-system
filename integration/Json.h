#ifndef JSON_H
#define JSON_H



#include <string>
#include <vector>
#include <map>


class JsonWriter {
private:
    std::string buffer;
    std::vector<bool> firstInScope;   

    void separate();

public:
    JsonWriter();

    static std::string escape(const std::string& text);

    JsonWriter& beginObject();
    JsonWriter& beginObject(const std::string& key);
    JsonWriter& endObject();

    JsonWriter& beginArray(const std::string& key);
    JsonWriter& endArray();

    JsonWriter& field(const std::string& key, const std::string& value);
    JsonWriter& field(const std::string& key, const char* value);
    JsonWriter& field(const std::string& key, int value);
    JsonWriter& field(const std::string& key, bool value);

    // For array elements that are bare objects.
    JsonWriter& arrayObjectStart();

    std::string str() const;
};


class JsonObject {
private:
    std::map<std::string, std::string> values;
    bool valid;
    std::string parseError;

public:
    JsonObject();

    bool parse(const std::string& text);

    bool isValid() const;
    std::string error() const;

    bool has(const std::string& key) const;
    std::string getString(const std::string& key,
                          const std::string& fallback = "") const;
    int getInt(const std::string& key, int fallback = 0) const;
    bool getBool(const std::string& key, bool fallback = false) const;

    
    bool getIntStrict(const std::string& key, int& out) const;
};

#endif 
