#include <config.hpp>
#include <debug.hpp>

#include <FormatConsole.hpp>
#include <paths.hpp>

#include <json.hpp>

#include <variant>
#include <unordered_map>
#include <tuple>
#include <any>

#include <updateRunningConfig.hpp>

using Json = nlohmann::json;

template <typename T>
using JsonSetter = void(*)(T*, const char*, const Json&);

template <typename T>
struct SettingsEntry {
    T* variable;
    JsonSetter<T> setter;

    SettingsEntry() = default;
    SettingsEntry(T* var, JsonSetter<T> set) : variable(var), setter(set) {}
};

template <typename T>
void setValue(T* variable, const char* jsonNameIndex, const Json& data) {
    *variable = data[jsonNameIndex].get<T>();
}

void setColor(Color* variable, const char* jsonNameIndex, const Json& data) {
    const auto& bgArray = data[jsonNameIndex];
    *variable = Color(bgArray[0].get<float>(), bgArray[1].get<float>(), 
                    bgArray[2].get<float>(), (bgArray.size() > 3 ? bgArray[3].get<float>() : 1.0f));
}

void setNanoseconds(std::chrono::nanoseconds* variable, const char* jsonNameIndex, const Json& data) {
    *variable = std::chrono::nanoseconds(data[jsonNameIndex].get<int>());
}

using SettingsVariant = std::variant<
    SettingsEntry<bool>,
    SettingsEntry<int>,
    SettingsEntry<float>,
    SettingsEntry<double>,
    SettingsEntry<Color>,
    SettingsEntry<std::chrono::nanoseconds>,
    SettingsEntry<unsigned char>,
    SettingsEntry<unsigned int>,
    SettingsEntry<simulationType>
>;

std::unordered_map<std::string, SettingsVariant> settings = {
    {"debugMode",                          SettingsEntry(&debugMode, setValue<bool>)},
    {"maxFrameRate",                       SettingsEntry(&maxFrameRate, setValue<int>)},
    {"defaultWindowWidth",                 SettingsEntry(&defaultWindowWidth, setValue<int>)},
    {"defaultWindowHeight",                SettingsEntry(&defaultWindowHeight, setValue<int>)},
    {"minWindowWidth",                     SettingsEntry(&minWindowWidth, setValue<int>)},
    {"minWindowHeight",                    SettingsEntry(&minWindowHeight, setValue<int>)},
    {"defaultBackgroundColor",             SettingsEntry(&backgroundColor, setColor)},
    {"prettyOutput",                       SettingsEntry(&prettyOutput, setValue<bool>)},
    {"VSync",                              SettingsEntry(&VSync, setValue<int>)},
    {"StaticFrameDelayFraction",           SettingsEntry(&staticDelayFraction, setValue<float>)},
    {"spinDelayNS",                        SettingsEntry(&spinDelay, setNanoseconds)},
    {"simulateObjectRotation",             SettingsEntry(&simulateObjectRotation, setValue<bool>)},
    {"lightUpdateFrameSkip",               SettingsEntry(&lightUpdateFrameSkip, setValue<unsigned char>)},
    {"renderDistance",                     SettingsEntry(&renderDistance, setValue<float>)},
    {"cameraSpeed",                        SettingsEntry(&cameraSpeed, setValue<float>)},
    {"cameraSensitivity",                  SettingsEntry(&cameraSensitivity, setValue<float>)},
    {"simulationMode",                     SettingsEntry(&simulationMode, setValue<simulationType>)},
    {"simpleMaxScale",                     SettingsEntry(&maxScale, setValue<float>)},
    {"normalizedModelRadius",              SettingsEntry(&normalizedModelRadius, setValue<float>)},
    {"renderScaleDistortion",              SettingsEntry(&renderScaleDistortion, setValue<double>)},
    {"phyiscsSubsteps",                    SettingsEntry(&phyiscsSubsteps, setValue<unsigned int>)},
    {"phyiscsBufferedFrames",              SettingsEntry(&phyiscsBufferedFrames, setValue<unsigned int>)},
    {"doPostProcess",                      SettingsEntry(&doPostProcess, setValue<bool>)},
    {"doFXAA",                             SettingsEntry(&doFXAA, setValue<bool>)}
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

    // avantgarde asshole
    for (auto& entryValue : settings) { // auto& [x, y] -> unpacks std::pair as references
        
        auto name = entryValue.first;
        auto entry = entryValue.second;

        if (data.contains(name)) {
            // std::visit -> takes a visitor (lambda) and a variant and applies the visitor to the currently active type in variant
            std::visit([& /*captures all variables in scope by reference*/](auto&& arg /*arguments - entry - settings[i].second.setter(...)*/) { // auto&& universal reference -> both r and l values
                arg.setter(arg.variable, name.c_str(), data);
            }, entry);
        }
    }

    updateRunningConfig::updateAllFromSet();

    if (debugMode) { cout << formatSuccess("Done") << endl; }
}