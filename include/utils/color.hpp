#ifndef COLOR_MANAGEMENT_AND_CLASS_HEADER
#define COLOR_MANAGEMENT_AND_CLASS_HEADER

#include <string>
#include <stdexcept>
#include <algorithm>
#include <array>

using Hex = std::string;
using HexDigit = char;

class Color {
    public:
        float r;
        float g;
        float b;

        float decR;
        float decG;
        float decB;

        float a;

        float* value[4] = {&r, &g, &b, &a};
        float* decValue[4] = {&decR, &decG, &decB, &a};

        Color (const int& colorValue): r(colorValue), g(colorValue), b(colorValue), a(1.0f)  { validateIntValues(); calculateDecimals(); }
        Color (const int& red, const int& green, const int& blue): r(red), g(green), b(blue), a(1.0f) { validateIntValues(); calculateDecimals(); }
        Color (const int& red, const int& green, const int& blue, const int& alpha): r(red), g(green), b(blue), a(alpha) { validateIntValues(); calculateDecimals(); }

        Color (const float& decimalColorValue): decR(decimalColorValue), decG(decimalColorValue), decB(decimalColorValue), a(1.0f) { validateDecimalValues(); calculatesInts(); }
        Color (const float& decimalRed, const float& decimalGreen, const float& decimalBlue): decR(decimalRed), decG(decimalGreen), decB(decimalBlue), a(1.0f) { validateDecimalValues(); calculatesInts(); }
        Color (const float& decimalRed, const float& decimalGreen, const float& decimalBlue, const float& alpha): decR(decimalRed), decG(decimalGreen), decB(decimalBlue), a(alpha) { validateDecimalValues(); calculatesInts(); }
        
        Color (Hex colorValue): a(1.0f) {
            if (colorValue == "") { throw std::invalid_argument("Hex color code is empty"); }
            if (colorValue[0] == '#') { colorValue.erase(0, 1); }
            if (colorValue.size() != 6) { throw std::invalid_argument("Hex color code is mangled: " + colorValue); }

            for (const HexDigit& digit : colorValue) {
                if (!isHex(digit)) { throw std::invalid_argument("Color is not in proper hex format: " + colorValue); }
            }

            r = std::stoi(colorValue.substr(0, 2), nullptr, 16);
            g = std::stoi(colorValue.substr(2, 2), nullptr, 16);
            b = std::stoi(colorValue.substr(4, 2), nullptr, 16);
            
            validateIntValues();
            calculateDecimals();
        };

        std::array<float, 4> getFlatValue() {
            return {r, g, b, a};
        }
        std::array<float, 4> getFlatDecValue() {
            return {decR, decG, decB, a};
        }

    private:
        bool isHex(HexDigit hexNumber) {
            return (hexNumber >= '0' && hexNumber <= '9') || 
                   (hexNumber >= 'a' && hexNumber <= 'f') || 
                   (hexNumber >= 'A' && hexNumber <= 'F');
        }

        template<typename T>
        void validateMemberNumber(T& number, T maxValue) { number = std::max((T)0, std::min(number, maxValue)); }

        template<typename T>
        void validateMemberNumber(T* number, T maxValue) { *number = std::max((T)0, std::min(*number, maxValue)); }

        void validateDecimalValues() {
            for (int i = 0; i < 3; ++i) { validateMemberNumber(decValue[i], 1.0f); }
            validateMemberNumber(a, 1.0f);
        }

        void validateIntValues() {
            for (int i = 0; i < 3; ++i) { validateMemberNumber(value[i], 255.0f); }
            validateMemberNumber(a, 1.0f);
        }

        void calculateDecimals() { for (int i = 0; i < 3; ++i) { *(decValue[i]) = (*(value[i]) / 255.0f); } }
        void calculatesInts() { for (int i = 0; i < 3; ++i) { *(value[i]) = (*(decValue[i]) * 255.0f); } }
};

#endif // COLOR_MANAGEMENT_AND_CLASS_HEADER