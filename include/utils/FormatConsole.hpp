#ifndef FORMAT_CONSOLE_TEXT_HEADER
#define FORMAT_CONSOLE_TEXT_HEADER

#include <string>

#define ANSII_RESET  "\033[0m"
#define ANSII_RED  "\033[31m"
#define ANSII_BLUE  "\033[34m"
#define ANSII_GREEN  "\033[32m"
#define ANSII_MAGENTA  "\033[35m"
#define ANSII_CYAN  "\033[36m"
#define ANSII_YELLOW "\u001b[33m"

std::string colorText(const std::string &text, const std::string &ansii_color) { return ansii_color + text + ANSII_RESET; }
std::string formatError(const std::string &text) { return ANSII_RED + text + ANSII_RESET; }
std::string formatPath(const std::string &path) { return ANSII_MAGENTA + path + ANSII_RESET; }
std::string formatSuccess(const std::string &text) { return ANSII_GREEN + text + ANSII_RESET; }
std::string formatProcess(const std::string &text) { return ANSII_BLUE + text + ANSII_RESET; }
auto formatRole = formatProcess;

#endif // FORMAT_CONSOLE_TEXT_HEADER