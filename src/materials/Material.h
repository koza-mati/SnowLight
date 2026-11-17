#pragma once
#include "../core/Ray.h"
#include "../core/Hittable.h"
#include <cmath>

// Prosty proceduralny szum 3D do bump mappingu lodu i tafli jeziora
inline double noise3d(double x, double y, double z) {
    auto fract = [](double n) { return n - std::floor(n); };
    double dot_prod = x * 12.9898 + y * 78.233 + z * 45.164;
    return fract(std::sin(dot_prod) * 43758.5453123);
}

// Proceduralne mikrozarysowania i nierówności lodu
inline Vec3 procedural_ice_bump(const Point3& p, const Vec3& normal) {
    double n1 = noise3d(p.x() * 22.0, p.y() * 22.0, p.z() * 22.0);
    double n2 = noise3d(p.x() * 64.0, p.y() * 64.0, p.z() * 64.0);
    double bump = (n1 * 0.75 + n2 * 0.25) * 0.045;
    return unit_vector(normal + Vec3(bump, bump, bump));
}

class Material {
public:
    virtual ~Material() = default;

    virtual Color emitted() const {
        return Color(0, 0, 0);
    }

    virtual Color transparency_tint() const {
        return Color(0, 0, 0);
    }

    virtual Color get_albedo() const {
        return Color(0, 0, 0);
    }

    virtual bool is_specular() const {
        return false;
    }

    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const = 0;
};

class DiffuseLight : public Material {
public:
    DiffuseLight(const Color& emit) : emit_color(emit) {}

    Color emitted() const override {
        return emit_color;
    }

    bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        return false;
    }

private:
    Color emit_color;
};

class Lambertian : public Material {
public:
    Lambertian(const Color& a) : albedo(a) {}

    Color get_albedo() const override {
        return albedo;
    }

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

class Metal : public Material {
public:
    Metal(const Color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    bool is_specular() const override {
        return true;
    }

    bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        Vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = Ray(rec.p, reflected + random_in_unit_sphere() * fuzz);
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }
private:
    Color albedo;
    double fuzz;
};

// Zamarznięta tafla jeziora z bump mappingiem szumowym i silnym odbiciem Fresnela
class FrozenLake : public Material {
public:
    FrozenLake(const Color& base_albedo, double ior = 1.31, double roughness = 0.015)
        : albedo(base_albedo), ir(ior), fuzz(roughness) {}

    Color get_albedo() const override {
        return albedo;
    }

    bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        Vec3 bumped_normal = procedural_ice_bump(rec.p, rec.normal);
        Vec3 unit_dir = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_dir, bumped_normal), 1.0);

        double r0 = (1.0 - ir) / (1.0 + ir);
        r0 = r0 * r0;
        double fresnel = r0 + (1.0 - r0) * std::pow(1.0 - cos_theta, 5.0);

        double reflect_prob = std::clamp(0.68 + fresnel * 0.32, 0.0, 0.96);

        if (random_double() < reflect_prob) {
            Vec3 reflected = reflect(unit_dir, bumped_normal);
            scattered = Ray(rec.p, reflected + random_in_unit_sphere() * fuzz);
            attenuation = Color(0.84, 0.92, 1.0);
            return (dot(scattered.direction(), bumped_normal) > 0);
        } else {
            auto scatter_direction = bumped_normal + random_unit_vector();
            if (scatter_direction.near_zero()) scatter_direction = bumped_normal;
            scattered = Ray(rec.p, scatter_direction);
            attenuation = albedo;
            return true;
        }
    }

private:
    Color albedo;
    double ir;
    double fuzz;
};

// Zaawansowany materiał kryształu lodu z tłumieniem Beer-Lamberta i mikro-chropowatością
class Dielectric : public Material {
public:
    double ir;
    Color tint;
    double roughness;
    double absorption; // Współczynnik absorpcji światła w objętości lodu

    Dielectric(double index_of_refraction, const Color& ice_tint = Color(0.82, 0.93, 1.0), double rough = 0.008, double absorb = 0.18)
        : ir(index_of_refraction), tint(ice_tint), roughness(rough), absorption(absorb) {}

    Color transparency_tint() const override {
        return tint;
    }

    bool is_specular() const override {
        return true;
    }

    bool scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        // Prawo Bouguera-Lamberta-Beera: tłumienie energii zależne od drogi wewnątrz lodu (rec.t)
        Color beer_lambert = Color(
            std::exp(-absorption * rec.t * (1.0 - tint.x())),
            std::exp(-absorption * rec.t * (1.0 - tint.y())),
            std::exp(-absorption * rec.t * (1.0 - tint.z()))
        );
        attenuation = tint * beer_lambert;

        // Bump mapping na fasetach kryształu
        Vec3 bumped_normal = procedural_ice_bump(rec.p, rec.normal);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

        Vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, bumped_normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        Vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double()) {
            direction = reflect(unit_direction, bumped_normal);
        } else {
            direction = refract(unit_direction, bumped_normal, refraction_ratio);
        }

        // Mikro-chropowatość faset (iskrzenie i mienienie się kryształu)
        if (roughness > 0.0) {
            direction = unit_vector(direction + random_in_unit_sphere() * roughness);
        }

        scattered = Ray(rec.p, direction);
        return true;
    }

private:
    static double reflectance(double cosine, double ref_idx) {
        auto r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1.0 - r0) * std::pow((1.0 - cosine), 5);
    }
};