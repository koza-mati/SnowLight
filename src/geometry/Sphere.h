#pragma once
#include "../core/Hittable.h"
#include "../core/Vec3.h"
#include "../core/AABB.h"
#include <memory>
#include <cmath>

class Sphere : public Hittable {
public:
    Sphere() {}
    
    // Konstruktor sfery wymaga wskaźnika na konkretny materiał
    Sphere(Point3 cen, double r, std::shared_ptr<Material> m) 
        : center(cen), radius(r), mat_ptr(m) {};

    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override;
    virtual bool bounding_box(AABB& output_box) const override;

private:
    Point3 center;
    double radius;
    std::shared_ptr<Material> mat_ptr;
};

inline bool Sphere::hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const {
    Vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius*radius;
    auto discriminant = half_b*half_b - a*c;

    if (discriminant < 0) return false;
    auto sqrtd = std::sqrt(discriminant);

    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || root > t_max) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || root > t_max)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    Vec3 outward_normal = (rec.p - center) * (1.0 / radius);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;

    return true;
}

inline bool Sphere::bounding_box(AABB& output_box) const {
    output_box = AABB(
        center - Vec3(radius, radius, radius),
        center + Vec3(radius, radius, radius)
    );
    return true;
}