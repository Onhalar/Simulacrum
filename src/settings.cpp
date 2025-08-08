#include <config.hpp>
#include <debug.hpp>

#include <FormatConsole.hpp>
#include <paths.hpp>

#include <json.hpp>

#include <variant>
#include <unordered_map>
#include <tuple>

#include <updateRunningConfig.hpp>

using AnyType = variant<float*, int*, bool*, string*, std::chrono::nanoseconds*, Color*>;
using Json = nlohmann::json;
using JsonSetter = void(*)(AnyType, const char*, const Json&);

void setString(AnyType variable, const char* jsonNameIndex, const Json&);
void setInt(AnyType variable, const char* jsonNameIndex, const Json&);
void setTuple(AnyType variable, const char* jsonNameIndex, const Json&);
void setBool(AnyType variable, const char* jsonNameIndex, const Json&);
void setFloat(AnyType variable, const char* jsonNameIndex, const Json&);
void setNanoseconds(AnyType variable, const char* jsonNameIndex, const Json&);

struct SettingsEntry {
    AnyType variable;
    JsonSetter setter;

    SettingsEntry() = default;

    SettingsEntry(AnyType var, JsonSetter set) {
        variable = var; setter = set;
    }
};

std::unordered_map<std::string, SettingsEntry> settings = {
    {"debugMode",                         SettingsEntry(&debugMode, setBool)},
    {"maxFrameRate",                      SettingsEntry(&maxFrameRate, setInt)},
    {"defaultWindowWidth",                SettingsEntry(&defaultWindowWidth, setInt)},
    {"defaultWindowHeight",               SettingsEntry(&defaultWindowHeight, setInt)},
    {"minWindowWidth",                    SettingsEntry(&minWindowWidth, setInt)},
    {"minWindowHeight",                   SettingsEntry(&minWindowHeight, setInt)},
    {"defaultBackgroundColor",            SettingsEntry(&defaultBackgroundColor, setTuple)},
    {"prettyOutput",                      SettingsEntry(&prettyOutput, setBool)},
    {"VSync",                             SettingsEntry(&VSync, setInt)},
    {"StaticFrameDelayFraction",          SettingsEntry(&staticDelayFraction, setFloat)},
    {"spinDelayNS",                       SettingsEntry(&spinDelay, setNanoseconds)},
    {"simulateObjectRotation",            SettingsEntry(&simulateObjectRotation, setBool)}
};

void loadSettings(std::filesystem::path path) {
    if (debugMode) {
        cout << formatProcess("Loading") << " settings '" << formatPath(getFileName(path.string())) << "' ... ";
    }

    if (!filesystem::exists(path) && debugMode) {
        cout << formatError("FAILED") << "\n";
        cerr << "unable to open '" << formatPath(path.string()) << "'\n" << endl;
        return;
    }

    ifstream file(path);
    Json data;
    file >> data;
    file.close();

    for (const auto& entry : data.items()) {
        const char* key = entry.key().c_str();
        if (settings.find(key) != settings.end()) {
            settings[key].setter(settings[key].variable, key, data);
        }
    }

    updateRunningConfig::updateAllFromSet();

    if (debugMode) { cout << formatSuccess("Done") << endl; }
}




void setString(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<std::string*>(variable) = data[jsonNameIndex].get<std::string>();
}
void setInt(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<int*>(variable) = data[jsonNameIndex].get<int>();
}
void setTuple(AnyType variable, const char* jsonNameIndex, const Json& data) {
    const auto& bgArray = data[jsonNameIndex];
    *std::get<Color*>(variable) = (Color){bgArray[0].get<float>(), bgArray[1].get<float>(), bgArray[2].get<float>(), bgArray[3].get<float>()};
}
void setBool(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<bool*>(variable) = data[jsonNameIndex].get<bool>();
}
void setFloat(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<float*>(variable) = data[jsonNameIndex].get<float>();
}
void setNanoseconds(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<std::chrono::nanoseconds*>(variable) = nanoseconds(data[jsonNameIndex].get<int>());
}