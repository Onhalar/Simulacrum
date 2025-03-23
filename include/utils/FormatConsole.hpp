#pragma once

#include <string>
using namespace std;

#define ANSII_RESET  "\033[0m"
#define ANSII_RED  "\033[31m"
#define ANSII_BLUE  "\033[34m"
#define ANSII_GREEN  "\033[32m"
#define ANSII_MAGENTA  "\033[35m"
#define ANSII_CYAN  "\033[36m"
#define ANSII_YELLOW "\u001b[33m"

string colorText(const string &text, const string &ansii_color) { return ansii_color + text + ANSII_RESET; }
string formatError(const string &text) { return ANSII_RED + text + ANSII_RESET; }
string formatPath(const string &path) { return ANSII_MAGENTA + path + ANSII_RESET; }
string formatSuccess(const string &text) { return ANSII_GREEN + text + ANSII_RESET; }
string formatProcess(const string &text) { return ANSII_BLUE + text + ANSII_RESET; }
auto formatRole = formatProcess;