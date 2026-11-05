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

// NOWOŚĆ: Materiał metaliczny (kierunkowe odbijanie światła)
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