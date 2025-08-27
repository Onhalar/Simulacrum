#ifndef TOML_PARSER_HEADER_LIB
#define TOML_PARSER_HEADER_LIB

#include <string>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <vector>
#include <stdexcept>

class TomlEntry;

using TomlCategory = std::unordered_map<std::string, TomlEntry>;
using InternalTomlData = std::unordered_map<std::string, TomlCategory>;

// value containing the Toml entry value
class TomlEntry {
    public:
        TomlEntry(): rawValue("") {}
        TomlEntry(std::string value): rawValue(value) {}

        operator std::string() const { return rawValue; }
        operator const char*() { return rawValue.c_str(); }

        template<typename T>
        TomlEntry& operator=(T value) {
            rawValue = std::to_string(value);
            return *this;
        }

        std::string get() {
            return rawValue;
        }

        template <typename T>
        T get() {
            std::stringstream buffer(rawValue);
            T out;

            if constexpr (std::is_same_v<T, bool>) {
                out = rawValue == "true" || rawValue == "True";
            } 
            else if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) {
                int tempOut;
                try { tempOut = stoi(rawValue); }
                catch (std::invalid_argument) {
                    buffer >> tempOut;
                }
                out = (T)tempOut;
            }
            else if constexpr (std::is_floating_point_v<T>) {
                double tempOut;
                try { tempOut = stod(rawValue); }
                catch (std::invalid_argument) {
                    buffer >> tempOut;
                }
                out = (T)tempOut;
            }
            else {
                buffer >> out;
            }

            return out;
        }
    private:
        std::string rawValue;
};

// Toml data manager
class Toml {  
    public:
        Toml() { data = {}; }
        Toml(std::string tomlData) { internalize(tomlData); }

        TomlCategory& operator[](std::string i) { return data[i]; }
        TomlCategory& operator[](const char* i) { return data[i]; }
        Toml& operator=(std::string value) { internalize(value); return *this; }
        Toml& operator=(const char* value) { internalize(value); return *this; }

        // defines a function that has access to classes private (s)
        friend std::istream& operator>>(std::istream& is, Toml& toml);

        std::string exportToString(bool spacedOutput = false) {
            std::stringstream exportBuffer;
            bool firstLine = true;
            for (const auto& category : data) {
                exportBuffer << (firstLine? "" : (spacedOutput? "\n" : "")) << '[' << category.first << "]\n";

                for (const auto& entry : category.second) {
                    exportBuffer << entry.first << (spacedOutput? " " : "") << (std::string)entry.second << '\n';
                }

                firstLine = false;
            }

            return exportBuffer.str();
        }

        bool valueExists(std::string value) {
            for (const auto& category : data) {
                if (category.second.find(value) != category.second.end()) { return true; }
            }
            return false;
        }

    private:
        InternalTomlData data;

        std::vector<std::string> getLines(std::string text) {
            std::vector<std::string> lines;
            std::string line;

            for (const char& character : text) {
                if (character != '\n') { line += character; }
                else {lines.push_back(line); line = ""; }
            }
            if (line != "") { lines.push_back(line); }
            
            return lines;
        }

        void internalize(std::string tomlData) {
            std::regex pairPattern(R"((\w+)\s*=\s*([0-9]+\.[0-9]+e[0-9]+|[0-9]+\.[0-9]+|\w+))");
            std::regex categoryPattern(R"(\[(\w+)\])");

            if (tomlData == "") { throw std::invalid_argument("empty TOML data"); }

            std::vector<std::string> lines = getLines(tomlData);

            if (!std::regex_search(lines[0], categoryPattern)) { throw std::invalid_argument("invalid TOML data"); }

            std::string lastCategory;

            for (const auto& line : lines) {
                std::smatch categoryMatch;
                if (std::regex_search(line, categoryMatch, categoryPattern)) {
                    std::string categoryName = categoryMatch[1];
                    data.emplace(categoryName, TomlCategory{});
                    lastCategory = categoryName;
                    continue;
                }

                std::smatch pairMatch;
                if (std::regex_search(line, pairMatch, pairPattern)) {
                    std::string key = pairMatch[1]; std::string value = pairMatch[2];
                    data[lastCategory][key] = TomlEntry(value);
                }
            }

        }

};

// Implementation of the friend function
std::istream& operator>>(std::istream& is, Toml& toml) {
    std::stringstream buffer;
    buffer << is.rdbuf();
    toml.internalize(buffer.str());
    return is;
}

#endif // TOML_PARSER_HEADER_LIB

