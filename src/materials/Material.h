#pragma once
#include "../core/Ray.h"
#include "../core/Hittable.h"

class Material {
public:
    virtual ~Material() = default;
    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const = 0;
};

class Lambertian : public Material {
public:
    Lambertian(const Color& a) : albedo(a) {}

    bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        auto scatter_direction = rec.normal + random_unit_vector();
        if (scatter_direction.near_zero()) scatter_direction = rec.normal;

        scattered = Ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }
private:
    Color albedo;
};

// Materiał metaliczny (kierunkowe odbijanie światła)
class Metal : public Material {
public:
    // Parametr fuzz (0.0 to idealne lustro, 1.0 to maksymalne zmatowienie)
    Metal(const Color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        // Obliczamy idealne odbicie
        Vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        
        // Generujemy nowy promień (z ewentualnym zmatowieniem dodanym do wektora)
        scattered = Ray(rec.p, reflected + random_in_unit_sphere() * fuzz);
        attenuation = albedo;
        
        // Zwracamy true tylko wtedy, gdy odbity promień faktycznie leci na zewnątrz powierzchni
        return (dot(scattered.direction(), rec.normal) > 0);
    }
private:
    Color albedo;
    double fuzz;
};

// Materiał dielektryczny (lód, szkło - refrakcja i efekt Fresnela)
class Dielectric : public Material {
public:
    double ir; // Współczynnik załamania światła (np. 1.31 dla lodu, 1.5 dla szkła)

    Dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        attenuation = Color(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

        Vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        Vec3 direction;

        // Sprawdzenie całkowitego wewnętrznego odbicia lub odbicia Fresnela (Schlick)
        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double()) {
            direction = reflect(unit_direction, rec.normal);
        } else {
            direction = refract(unit_direction, rec.normal, refraction_ratio);
        }

        scattered = Ray(rec.p, direction);
        return true;
    }

private:
    // Aproksymacja Schlicka dla współczynnika odbicia Fresnela
    static double reflectance(double cosine, double ref_idx) {
        auto r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1.0 - r0) * std::pow((1.0 - cosine), 5);
    }
};