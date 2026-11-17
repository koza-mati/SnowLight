#pragma once
#include <cmath>
#include <algorithm>
#include "Vec3.h"
#include "Ray.h"

class AABB {
public:
    Point3 minimum;
    Point3 maximum;

    AABB() {}
    AABB(const Point3& a, const Point3& b) : minimum(a), maximum(b) {}

    Point3 min() const { return minimum; }
    Point3 max() const { return maximum; }

    // Szybki test przecięcia promienia z prostopadłościanem (metoda slab)
    bool hit(const Ray& r, double t_min, double t_max) const {
        for (int a = 0; a < 3; a++) {
            auto invD = 1.0 / r.direction().e[a];
            auto t0 = (minimum.e[a] - r.origin().e[a]) * invD;
            auto t1 = (maximum.e[a] - r.origin().e[a]) * invD;

            if (invD < 0.0)
                std::swap(t0, t1);

            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;

            if (t_max <= t_min)
                return false;
        }
        return true;
    }
};

// Łączy dwa pudełka AABB w jedno wspólne pudełko nadrzędne
inline AABB surrounding_box(AABB box0, AABB box1) {
    Point3 small(
        std::fmin(box0.min().x(), box1.min().x()),
        std::fmin(box0.min().y(), box1.min().y()),
        std::fmin(box0.min().z(), box1.min().z())
    );
    Point3 big(
        std::fmax(box0.max().x(), box1.max().x()),
        std::fmax(box0.max().y(), box1.max().y()),
        std::fmax(box0.max().z(), box1.max().z())
    );
    return AABB(small, big);
}