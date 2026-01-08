#include <config.hpp>
#include <debug.hpp>

#include <FormatConsole.hpp>
#include <paths.hpp>

#include <simpleToml.hpp>

#include <string>
#include <variant>
#include <unordered_map>


template <typename T>
using TomlSetter = void(*)(T*, const char*, const char*, Toml&);

template <typename T>
struct SettingsEntry {
    T* variable;
    TomlSetter<T> setter;

    SettingsEntry() = default;
    SettingsEntry(T* var, TomlSetter<T> set) : variable(var), setter(set) {}
};

template <typename T>
void setValue(T* variable, const char* TomlCategory, const char* TomlEntry, Toml& data) {
    *variable = data[TomlCategory][TomlEntry].get<T>();
}

void setColor(Color* variable, const char* TomlCategory, const char* TomlEntry, Toml& data) {
    *variable = Color((std::string)data[TomlCategory][TomlEntry]);
}

void setNanoseconds(std::chrono::nanoseconds* variable, const char* TomlCategory, const char* TomlEntry, Toml& data) {
    *variable = std::chrono::nanoseconds(data[TomlCategory][TomlEntry].get<int>());
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
    SettingsEntry<simulationType>,
    SettingsEntry<std::string>
>;

std::unordered_map<std::string, std::pair<std::string, SettingsVariant>> settings = {
    {"debugMode",                         {"DEBUG", SettingsEntry(&debugMode, setValue<bool>)}},
    {"prettyOutput",                      {"DEBUG", SettingsEntry(&prettyOutput, setValue<bool>)}},

    {"defaultWindowWidth",                {"WINDOW", SettingsEntry(&defaultWindowWidth, setValue<int>)}},
    {"defaultWindowHeight",               {"WINDOW", SettingsEntry(&defaultWindowHeight, setValue<int>)}},
    {"minWindowWidth",                    {"WINDOW", SettingsEntry(&minWindowWidth, setValue<int>)}},
    {"minWindowHeight",                   {"WINDOW", SettingsEntry(&minWindowHeight, setValue<int>)}},
    {"defaultBackgroundColor",            {"WINDOW", SettingsEntry(&backgroundColor, setColor)}},

    {"maxFrameRate",                      {"RENDER", SettingsEntry(&maxFrameRate, setValue<int>)}},
    {"VSync",                             {"RENDER", SettingsEntry(&VSync, setValue<int>)}},
    {"StaticFrameDelayFraction",          {"RENDER", SettingsEntry(&staticDelayFraction, setValue<float>)}},
    {"spinDelayNS",                       {"RENDER", SettingsEntry(&spinDelay, setNanoseconds)}},
    {"doPostProcess",                     {"RENDER", SettingsEntry(&doPostProcess, setValue<bool>)}},
    {"doFXAA",                            {"RENDER", SettingsEntry(&doFXAA, setValue<bool>)}},
    {"lightUpdateFrameSkip",              {"RENDER", SettingsEntry(&lightUpdateFrameSkip, setValue<unsigned char>)}},
    {"inverseColors",                     {"RENDER", SettingsEntry(&inverseColors, setValue<bool>)}},
    {"fullscreen",                        {"RENDER", SettingsEntry(&fullscreen, setValue<bool>)}},
    {"centerObjectScaleMultiplier",       {"RENEDR", SettingsEntry(&centerObjectScaleMultiplier, setValue<unsigned int>)}},

    {"renderDistance",                    {"CAMERA", SettingsEntry(&renderDistance, setValue<float>)}},
    {"cameraSpeed",                       {"CAMERA", SettingsEntry(&cameraSpeed, setValue<float>)}},
    {"cameraSensitivity",                 {"CAMERA", SettingsEntry(&cameraSensitivity, setValue<float>)}},
    {"cameraFOV",                         {"CAMERA", SettingsEntry(&fovDeg, setValue<float>)}},
    {"sceneZoomModifier",                 {"CAMERA", SettingsEntry(&sceneZoomModifier, setValue<float>)}},

    {"simulationMode",                    {"PHYSICS", SettingsEntry(&simulationMode, setValue<simulationType>)}},
    {"simpleMaxScale",                    {"PHYSICS", SettingsEntry(&maxScale, setValue<float>)}},
    {"unifiedDistance",                   {"PHYSICS", SettingsEntry(&unifiedDistance, setValue<double>)}},
    {"normalizedModelRadius",             {"PHYSICS", SettingsEntry(&normalizedModelRadius, setValue<float>)}},
    {"renderScaleDistortion",             {"PHYSICS", SettingsEntry(&renderScaleDistortion, setValue<double>)}},
    {"physicsSubsteps",                   {"PHYSICS", SettingsEntry(&phyiscsSubsteps, setValue<unsigned int>)}},
    {"simulateObjectRotation",            {"PHYSICS", SettingsEntry(&simulateObjectRotation, setValue<bool>)}},
    {"physicsSteps",                      {"PHYSICS", SettingsEntry(&physicsSteps, setValue<float>)}},
    {"gravityInInitialVel",               {"PHYSICS", SettingsEntry(&gravityInInitialVel, setValue<bool>)}},

    {"fontSize",                          {"GUI", SettingsEntry(&fontSize, setValue<float>)}},
    {"windowRounding",                    {"GUI", SettingsEntry(&windowRounding, setValue<float>)}},
    {"frameRounding",                     {"GUI", SettingsEntry(&frameRounding, setValue<float>)}},
    {"fontFile",                          {"GUI", SettingsEntry(&fontFile, setValue<std::string>)}}
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
    Toml data;
    file >> data;
    file.close();

    // avantgarde asshole
    for (auto& entryValue : settings) { // auto& [x, y] -> unpacks std::pair as references
        
        auto name = entryValue.first;
        auto entry = entryValue.second.second;
        auto category = entryValue.second.first;

        if (data.valueExists(name)) {
            // std::visit -> takes a visitor (lambda) and a variant and applies the visitor to the currently active type in variant
            std::visit([& /*captures all variables in scope by reference*/](auto&& arg /*arguments - entry - settings[i].second.setter(...)*/) { // auto&& universal reference -> both r and l values
                arg.setter(arg.variable, category.c_str(), name.c_str(), data);
            }, entry);
        }
    }

    if (debugMode) { cout << formatSuccess("Done") << endl; }
}