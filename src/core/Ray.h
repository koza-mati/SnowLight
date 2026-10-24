#pragma once
#include "Vec3.h"


/*

Reprezentacja promieni światła jako funckja liniowa w przestrzeni 3D: P(t) = A + B * t ,
gdzie A to punkt początkowy, a B to wektor kierunkowy.

*/

class Ray {
public:
    Ray() {}
    Ray(const Point3& origin, const Vec3& direction) : orig(origin), dir(direction) {}

    Point3 origin() const  { return orig; } // Oblicza dokładny punkt w przestrzeni 3D dla @param t, 
    // natomiast t jest dystansem pokonanym przez promień wzdłuż wektora kierunkowego.
    Vec3 direction() const { return dir; }
    Point3 at(double t) const { return orig + dir * t; }

private:
    Point3 orig;
    Vec3 dir;
};