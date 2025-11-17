#ifndef FORMAT_CONSOLE_TEXT_HEADER
#define FORMAT_CONSOLE_TEXT_HEADER

#include <string>

// only to enable coloring of text, can sometimes help with clarity, if not needed remove next line and uncomment the following line
#include <debug.hpp>
// bool prettyOutput = true;

#define ANSII_RESET  "\033[0m"
#define ANSII_RED  "\033[31m"
#define ANSII_BLUE  "\033[34m"
#define ANSII_GREEN  "\033[32m"
#define ANSII_MAGENTA  "\033[35m"
#define ANSII_CYAN  "\033[36m"
#define ANSII_YELLOW "\u001b[33m"

inline std::string colorText(const std::string &text, const std::string &ansii_color) { return prettyOutput ? ansii_color + text + ANSII_RESET : text; }
inline std::string formatError(const std::string &error) { return prettyOutput ? ANSII_RED + error + ANSII_RESET : error; }
inline std::string formatPath(const std::string &path) { return prettyOutput ? ANSII_MAGENTA + path + ANSII_RESET : path; }
inline std::string formatSuccess(const std::string &sucessMessage) { return prettyOutput ? ANSII_GREEN + sucessMessage + ANSII_RESET : sucessMessage; }
inline std::string formatProcess(const std::string &output) { return prettyOutput ? ANSII_BLUE + output + ANSII_RESET : output; }
inline std::string formatWarning(const std::string &warningMessage) { return prettyOutput ? ANSII_YELLOW + warningMessage + ANSII_RESET : warningMessage; }
inline auto formatRole = formatProcess;

#endif // FORMAT_CONSOLE_TEXT_HEADER