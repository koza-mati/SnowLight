#pragma once
#include <cmath>
#include <random>

// Matematyczny fundament całego silnika, przechowujący współrzędne w przestrzeni (x,y,z) lub wartości kolorów (R,G,B)
class Vec3 {
public:
    double e[3];

    Vec3() : e{0,0,0} {}
    Vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }
    Vec3 operator-(const Vec3& v) const { return Vec3(e[0]-v.e[0], e[1]-v.e[1], e[2]-v.e[2]); }
    Vec3 operator+(const Vec3& v) const { return Vec3(e[0]+v.e[0], e[1]+v.e[1], e[2]+v.e[2]); }
    Vec3 operator*(double t) const { return Vec3(e[0]*t, e[1]*t, e[2]*t); }
    
    // Niezbędny operator do mieszania kolorów w ray tracingu (mnożenie wektor * wektor)
    Vec3 operator*(const Vec3& v) const { return Vec3(e[0]*v.e[0], e[1]*v.e[1], e[2]*v.e[2]); }

    double length() const { return std::sqrt(length_squared()); }
    
    // Zwraca kwadrat długości wektora, przez co pozwala uniknąć kosztownej funkcji pierwiastka przy szybkich porównaniach
    double length_squared() const { return e[0]*e[0] + e[1]*e[1] + e[2]*e[2]; }

    // Zwraca true, jeśli wektor jest bardzo bliski zeru we wszystkich wymiarach (musi być W ŚRODKU klasy)
    bool near_zero() const {
        const auto s = 1e-8;
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }
};

// Funkcja iloczynu skalarnego, która pozwala obliczyć kąt padania światła względem powierzchni
inline double dot(const Vec3& u, const Vec3& v) {
    return u.e[0]*v.e[0] + u.e[1]*v.e[1] + u.e[2]*v.e[2];
}

inline Vec3 unit_vector(Vec3 v) {
    return v * (1.0 / v.length());
}

// Funkcja iloczynu wektorowego (Cross Product). Zwraca wektor 3D prostopadły do u i v.
inline Vec3 cross(const Vec3& u, const Vec3& v) {
    return Vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) {
    return min + (max-min)*random_double();
}

inline Vec3 random_in_unit_sphere() {
    while (true) {
        auto p = Vec3(random_double(-1,1), random_double(-1,1), random_double(-1,1));
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

inline Vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}

using Point3 = Vec3;
using Color = Vec3;