#ifndef CUSTOM_MATH_HEADER
#define CUSTOM_MATH_HEADER

#include <config.hpp>
#include <types.hpp>
#include <simObject.hpp>
#include <globals.hpp>

template <typename T>
bool isPowerOfTwo (const T& number) {
    static_assert(std::is_unsigned<T>::value, "Type must be unsigned"); // compile time check (static assert); value -> 0/1

    return number != 0 && (number & (number - 1)) == 0;
}

template <typename T>
T roundUpToPowerOfTwo(T& number) {
    static_assert(std::is_unsigned<T>::value, "Type must be unsigned");

    if ( number == 0 ) { return 1; }
    if ( number == std::numeric_limits<T>::max() ) { return ~(number >> 1); } // (>>) 1111 => (~) 0111 => 1000

    number--; // necesery because of if number is 1

    T temp = 1;

    for (; temp < number;) {
        temp <<= 1; // shift bit by one space to the left (equivalent *= 2) => 0001 => 0010
    }

    return temp;
}

units::kilometers exponentialScale(const units::kilometers& minValue, const units::kilometers& MaxValue, const units::kilometers& currentValue, const float& maxScale = maxScale) {
    if (minValue >= MaxValue /*Zero Division handler*/ || currentValue <= minValue) {
        return 1.0;
    }

    if (currentValue >= MaxValue) {
        return maxScale;
    }

    // Normalize the current value to a 0-1 range.
    double normalized_val = (currentValue - minValue) / (MaxValue - minValue);

    return std::pow(maxScale, normalized_val);
}

inline glm::mat4 calcuculateModelMatrixFromPosition(const glm::vec3& position, const glm::mat4& modelMatrix) {
    return glm::translate(modelMatrix, position);
}
inline glm::mat4 calcuculateModelMatrixFromPosition(const glm::vec3& position) {
    return glm::translate(glm::mat4(1.0f), position);
}

#endif // CUSTOM_MATH_HEADER