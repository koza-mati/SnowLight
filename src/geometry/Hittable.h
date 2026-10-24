#pragma once
#include "../core/Ray.h"

// Polimorficzny interfejs, dzięki któremu silinik może analizować kolizje promienia z każdym kształtem bez znajomości jego specyfikacji
struct HitRecord {
    Point3 p;
    Vec3 normal;
    double t;
    bool front_face;

    // Zapewnia, że wektor normalny (prostopadły do danej prostej, płaszczyzny w danym punkcie) uderzonej powierzchni zawsze skierowany jest przeciwko promieniowi
    // Stanowi to warunek do wyliczenia refrakcji na granicy ośrodków lodu i powietrza 
    inline void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const = 0;
};