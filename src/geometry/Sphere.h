#pragma once
#include "Hittable.h"

// Implementuje równania kuli i rozwiązuje układ rónań z prostą zdefiniowańa przez promień

class Sphere : public Hittable {
public:
    Sphere(Point3 cen, double r) : center(cen), radius(r) {}

    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override {
        Vec3 oc = r.origin() - center;
        double a = r.direction().length_squared();
        double half_b = dot(oc, r.direction());
        double c = oc.length_squared() - radius*radius;
        // Obliczenie delty. Wartość ujemna -> brak kolizji z obiektem
        double discriminant = half_b*half_b - a*c;

        if (discriminant < 0) return false;
        double sqrtd = std::sqrt(discriminant);

        // Znajdź najbliższe przecięcie w akceptowalnym zakresie
        double root = (-half_b - sqrtd) / a;
        if (root < t_min || t_max < root) {
            root = (-half_b + sqrtd) / a;
            if (root < t_min || t_max < root)
                return false;
        }

        rec.t = root;
        // Zapisuje współrzędne fizycznego punktu, w którym promień uderzył w powierzchnię kuli na potrzeby dalszych obliczeń cieniowania
        rec.p = r.at(rec.t);
        Vec3 outward_normal = (rec.p - center) * (1.0 / radius);
        rec.set_face_normal(r, outward_normal);

        return true;
    }

private:
    Point3 center;
    double radius;
};