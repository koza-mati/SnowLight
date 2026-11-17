#pragma once
#include <cmath>
#include <memory>
#include "../core/Hittable.h"
#include "../core/Vec3.h"

class Triangle : public Hittable {
public:
    Point3 v0, v1, v2;
    Vec3 normal;
    std::shared_ptr<Material> mat_ptr;

    Triangle() {}
    Triangle(const Point3& _v0, const Point3& _v1, const Point3& _v2, std::shared_ptr<Material> m)
        : v0(_v0), v1(_v1), v2(_v2), mat_ptr(m) {
        // Wyznaczenie wektora normalnego z iloczynu wektorowego dwóch krawędzi
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        normal = unit_vector(cross(edge1, edge2));
    }

    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override {
        const double EPSILON = 1e-8;
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;

        Vec3 h = cross(r.direction(), edge2);
        double a = dot(edge1, h);

        // Jeśli wyznacznik bliski 0, promień biegnie równolegle do płaszczyzny trójkąta
        if (a > -EPSILON && a < EPSILON)
            return false;

        double f = 1.0 / a;
        Vec3 s = r.origin() - v0;
        double u = f * dot(s, h);

        if (u < 0.0 || u > 1.0)
            return false;

        Vec3 q = cross(s, edge1);
        double v = f * dot(r.direction(), q);

        if (v < 0.0 || u + v > 1.0)
            return false;

        // Wyznaczenie odległości t wzdłuż promienia
        double t = f * dot(edge2, q);
        if (t < t_min || t > t_max)
            return false;

        rec.t = t;
        rec.p = r.at(t);
        rec.set_face_normal(r, normal);
        rec.mat_ptr = mat_ptr;

        return true;
    }

    virtual bool bounding_box(AABB& output_box) const override {
        // Margines 0.0001 zapobiega zerowej grubości pudełka dla płaskich trójkątów
        const double pad = 0.0001;
        Point3 min_pt(
            std::fmin(v0.x(), std::fmin(v1.x(), v2.x())) - pad,
            std::fmin(v0.y(), std::fmin(v1.y(), v2.y())) - pad,
            std::fmin(v0.z(), std::fmin(v1.z(), v2.z())) - pad
        );
        Point3 max_pt(
            std::fmax(v0.x(), std::fmax(v1.x(), v2.x())) + pad,
            std::fmax(v0.y(), std::fmax(v1.y(), v2.y())) + pad,
            std::fmax(v0.z(), std::fmax(v1.z(), v2.z())) + pad
        );
        output_box = AABB(min_pt, max_pt);
        return true;
    }
};