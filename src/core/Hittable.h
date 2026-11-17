#pragma once
#include <memory>
#include "../core/Ray.h"
#include "../core/AABB.h"

// Zapowiedź (forward declaration) klasy Material.
// Pozwala użyć wskaźnika na ten typ bez wczytywania całego pliku Material.h
class Material;

struct HitRecord {
    Point3 p;
    Vec3 normal;
    std::shared_ptr<Material> mat_ptr; // Polimorficzny wskaźnik na materiał
    double t;
    bool front_face;

    // Wymusza, aby wektor normalny zawsze wskazywał przeciwnie do kierunku promienia
    inline void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const = 0;
    virtual bool bounding_box(AABB& output_box) const = 0;
};