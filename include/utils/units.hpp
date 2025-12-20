#ifndef PHYSICS_UNITS_HEADER
#define PHYSICS_UNITS_HEADER

#include <math.h>

class units{
    private:
        enum unitID : signed char {
            BASE = 0,       // BASE eg. g
            BASE_3 = 3,     // BASE ^ 3 eg. kg
            BASE_6 = 6      // BASE ^ 6 eg. t
        };

    public:
        struct kilometers {

            double value;

            static constexpr unsigned char unitIndex = unitID::BASE_3;

            constexpr kilometers(): value(0.0) {}
            constexpr kilometers(double value): value(value) {}
            constexpr kilometers(const kilometers& master): value(master.value) {}

            constexpr kilometers& operator*=(double scalar) { this->value *= scalar; return *this; }
            constexpr kilometers& operator*=(kilometers km) { this->value *= km.value; return *this; }

            constexpr kilometers& operator/=(double scalar) { this->value /= scalar; return *this; }
            constexpr kilometers& operator/=(kilometers km) { this->value /= km.value; return *this; }

            constexpr friend kilometers operator*(const kilometers& km, double scalar) { return kilometers(km.value * scalar); }
            constexpr friend kilometers operator*(double scalar, const kilometers& km) { return kilometers(km.value * scalar); }
            constexpr friend kilometers operator*(const kilometers& km_x, const kilometers& km_y) { return kilometers(km_x.value * km_y.value); }

            constexpr friend kilometers operator/(const kilometers& km, double scalar) { return kilometers(km.value / scalar); }
            constexpr friend kilometers operator/(double scalar, const kilometers& km) { return kilometers(km.value / scalar); }
            constexpr friend kilometers operator/(const kilometers& km_x, const kilometers& km_y) { return kilometers(km_x.value / km_y.value); }

            constexpr operator double() const { return value; }

            template <typename T>
            T get() const {
                static_assert(requires { T::unitIndex; }, "Cannot be converted to type that does not have a static member 'unitIndex'");
                static_assert(requires { T::value; }, "Cannot be converted to type that does not have a member 'value'");

                if (T::unitIndex != this->unitIndex) { return T(this->value * std::pow(10.0, this->unitIndex - T::unitIndex)); }
                else { return T(this->value); }
            }
        };
        struct meters {

            double value;

            static constexpr unsigned char unitIndex = unitID::BASE;

            constexpr meters(): value(0.0) {}
            constexpr meters(double value): value(value) {}
            constexpr meters(const meters& master): value(master.value) {}

            constexpr meters& operator*=(double scalar) { this->value *= scalar; return *this; }
            constexpr meters& operator*=(meters m) { this->value *= m.value; return *this; }

            constexpr meters& operator/=(double scalar) { this->value /= scalar; return *this; }
            constexpr meters& operator/=(meters m) { this->value /= m.value; return *this; }

            constexpr friend meters operator*(const meters& m, double scalar) { return meters(m.value * scalar); }
            constexpr friend meters operator*(double scalar, const meters& m) { return meters(m.value * scalar); }
            constexpr friend meters operator*(const meters& m_x, const meters& m_y) { return meters(m_x.value * m_y.value); }
            
            constexpr friend meters operator/(const meters& m, double scalar) { return meters(m.value / scalar); }
            constexpr friend meters operator/(double scalar, const meters& m) { return meters(m.value / scalar); }
            constexpr friend meters operator/(const meters& m_x, const meters& m_y) { return meters(m_x.value / m_y.value); }

            constexpr operator double() const { return value; }

            template <typename T>
            T get() const {
                static_assert(requires { T::unitIndex; }, "Cannot be converted to type that does not have a static member 'unitIndex'");
                static_assert(requires { T::value; }, "Cannot be converted to type that does not have a member 'value'");

                if (T::unitIndex != this->unitIndex) { return T(this->value * std::pow(10.0, this->unitIndex - T::unitIndex)); }
                else { return T(this->value); }
            }
        };
        struct tons {

            double value;

            static constexpr unsigned char unitIndex = unitID::BASE_6;

            constexpr tons(): value(0.0) {}
            constexpr tons(double value): value(value) {}
            constexpr tons(const tons& master): value(master.value) {}

            constexpr tons& operator*=(double scalar) { this->value *= scalar; return *this; }
            constexpr tons& operator*=(tons t) { this->value *= t.value; return *this; }

            constexpr tons& operator/=(double scalar) { this->value /= scalar; return *this; }
            constexpr tons& operator/=(tons t) { this->value /= t.value; return *this; }

            constexpr friend tons operator*(const tons& t, double scalar) { return tons(t.value * scalar); }
            constexpr friend tons operator*(double scalar, const tons& t) { return tons(t.value * scalar); }
            constexpr friend tons operator*(const tons& t_x, const tons& t_y) { return tons(t_x.value * t_y.value); }

            constexpr friend tons operator/(const tons& t, double scalar) { return tons(t.value / scalar); }
            constexpr friend tons operator/(double scalar, const tons& t) { return tons(t.value / scalar); }
            constexpr friend tons operator/(const tons& t_x, const tons& t_y) { return tons(t_x.value / t_y.value); }

            constexpr operator double() const { return value; }

            template <typename T>
            T get() const {
                static_assert(requires { T::unitIndex; }, "Cannot be converted to type that does not have a static member 'unitIndex'");
                static_assert(requires { T::value; }, "Cannot be converted to type that does not have a member 'value'");

                if (T::unitIndex != this->unitIndex) { return T(this->value * std::pow(10.0, this->unitIndex - T::unitIndex)); }
                else { return T(this->value); }
            }
        };
        struct kilograms {

            double value;

            static constexpr unsigned char unitIndex = unitID::BASE_3;

            constexpr kilograms(): value(0.0) {}
            constexpr kilograms(double value): value(value) {}
            constexpr kilograms(const kilograms& master): value(master.value) {}

            constexpr kilograms& operator*=(double scalar) { this->value *= scalar; return *this; }
            constexpr kilograms& operator*=(kilograms kg) { this->value *= kg.value; return *this; }

            constexpr kilograms& operator/=(double scalar) { this->value /= scalar; return *this; }
            constexpr kilograms& operator/=(kilograms kg) { this->value /= kg.value; return *this; }

            constexpr friend kilograms operator*(const kilograms& kg, double scalar) { return kilograms(kg.value * scalar); }
            constexpr friend kilograms operator*(double scalar, const kilograms& kg) { return kilograms(kg.value * scalar); }
            constexpr friend kilograms operator*(const kilograms& kg_x, const kilograms& kg_y) { return kilograms(kg_x.value * kg_y.value); }

            constexpr friend kilograms operator/(const kilograms& kg, double scalar) { return kilograms(kg.value / scalar); }
            constexpr friend kilograms operator/(double scalar, const kilograms& kg) { return kilograms(kg.value / scalar); }
            constexpr friend kilograms operator/(const kilograms& kg_x, const kilograms& kg_y) { return kilograms(kg_x.value / kg_y.value); }

            constexpr operator double() const { return value; }

            template <typename T>
            T get() const {
                static_assert(requires { T::unitIndex; }, "Cannot be converted to type that does not have a static member 'unitIndex'");
                static_assert(requires { T::value; }, "Cannot be converted to type that does not have a member 'value'");

                if (T::unitIndex != this->unitIndex) { return T(this->value * std::pow(10.0, this->unitIndex - T::unitIndex)); }
                else { return T(this->value); }
            }
        };

        // more pefromant that From_T::get<To_T>(); does to = from * convertor
        template <typename To_T, typename From_T>
        static constexpr To_T manual_cast(const From_T& value, const long double& convertor) {
            static_assert(requires { From_T::unitIndex; }, "Origin type cannot be converted to type that does not have a static member 'unitIndex'");
            static_assert(requires { From_T::value; }, "Origin type cannot be converted to type that does not have a member 'value'");
            static_assert(requires { To_T::unitIndex; }, "Destination type cannot be converted to type that does not have a static member 'unitIndex'");
            static_assert(requires { To_T::value; }, "Destination type cannot be converted to type that does not have a member 'value'");

            return To_T(value.value * convertor);
        }

        // does a very basic check if a variable's type is a unit of this library
        template <typename T>
        static constexpr bool isUnit(const T& var) {
            if constexpr (requires { T::unitindex; T::value; }) { return true; }
            else { return false; }

            return false;
        }
};

constexpr units::kilometers operator"" _km(long double value) { return units::kilometers(value); }
constexpr units::meters operator"" _m(long double value) { return units::meters(value); }
constexpr units::tons operator"" _t(long double value) { return units::tons(value); }
constexpr units::kilograms operator"" _kg(long double value) { return units::kilograms(value); }

#endif // PHYSICS_UNITS_HEADER