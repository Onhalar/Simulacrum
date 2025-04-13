#ifndef FORMAT_CONSOLE_TEXT_HEADER
#define FORMAT_CONSOLE_TEXT_HEADER

#include <string>

// only to enable coloring of text, can sometimes help with clarity, if not needed remove next line and uncomment the following line
#include <config.hpp>
// bool prettyOutput = true;

#define ANSII_RESET  "\033[0m"
#define ANSII_RED  "\033[31m"
#define ANSII_BLUE  "\033[34m"
#define ANSII_GREEN  "\033[32m"
#define ANSII_MAGENTA  "\033[35m"
#define ANSII_CYAN  "\033[36m"
#define ANSII_YELLOW "\u001b[33m"

std::string colorText(const std::string &text, const std::string &ansii_color) { return prettyOutput ? ansii_color + text + ANSII_RESET : text; }
std::string formatError(const std::string &error) { return prettyOutput ? ANSII_RED + error + ANSII_RESET : error; }
std::string formatPath(const std::string &path) { return prettyOutput ? ANSII_MAGENTA + path + ANSII_RESET : path; }
std::string formatSuccess(const std::string &sucessMessage) { return prettyOutput ? ANSII_GREEN + sucessMessage + ANSII_RESET : sucessMessage; }
std::string formatProcess(const std::string &output) { return prettyOutput ? ANSII_BLUE + output + ANSII_RESET : output; }
auto formatRole = formatProcess;

#endif // FORMAT_CONSOLE_TEXT_HEADER