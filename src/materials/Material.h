#pragma once
#include "../core/Ray.h"
#include "../core/Hittable.h"

class Material {
public:
    virtual ~Material() = default;
    
    // Zwraca true, jeśli promień odbił się od powierzchni, false jeśli został w pełni pochłonięty
    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const = 0;
};

class Lambertian : public Material {
public:
    Lambertian(const Color& a) : albedo(a) {}

    bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        // Obliczenie losowego kierunku odbicia z powierzchni matowej
        auto scatter_direction = rec.normal + random_unit_vector();

        // Ochrona przed błędem dzielenia przez zero, jeśli wektory by się zniosły
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = Ray(rec.p, scatter_direction);
        attenuation = albedo; // Obiekt "oddaje" tylko ułamek energii równy swojemu kolorowi
        return true;
    }

private:
    Color albedo;
};