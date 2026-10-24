#pragma once
#include <cmath>

// Matematyczny fundament całego silnika, przechowując współrzędne w przestrzeni (x,y,z) lub wartości kolorów (R,G,B)
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
    
    double length() const { return std::sqrt(length_squared()); }
     // Zwraca kwadrat długości wektora, przez co pozwala uniknąć kosztownej funkcji pierwisatka przy szybkich porównaniach odległości
    double length_squared() const { return e[0]*e[0] + e[1]*e[1] + e[2]*e[2]; }
};

// Funckja iloczynu skalarnego, która pozwala obliczyć kąt padania światła względem powierzchni, co determinuje jasność oświetlanego punktu 
inline double dot(const Vec3& u, const Vec3& v) {
    return u.e[0]*v.e[0] + u.e[1]*v.e[1] + u.e[2]*v.e[2];
}

inline Vec3 unit_vector(Vec3 v) {
    return v * (1.0 / v.length());
}

using Point3 = Vec3;
using Color = Vec3;