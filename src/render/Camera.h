#pragma once
#include <cmath>
#include "../core/Vec3.h"
#include "../core/Ray.h"

class Camera {
public:
    Camera() {
        update_camera();
    }

    Ray get_ray(double u, double v) const {
        return Ray(origin, lower_left_corner + horizontal*u + vertical*v - origin);
    }
    
    void zoom(double delta) {
        radius += delta;
        if (radius < 0.5) radius = 0.5; // Zapobiega wejściu w środek latarni
        update_camera();
    }

    void orbit(double delta_h, double delta_v) {
        angle_h += delta_h; 
        angle_v += delta_v; 
        
        // GÓRNY LIMIT (klawisz Q): Patrzymy z góry, ale blokujemy wektor zerowy
        if (angle_v > 1.5) angle_v = 1.5;
        
        // DOLNY LIMIT (klawisz E): Zabezpieczenie przed "wlotem pod ziemię"!
        // 0.0 to poziom równego gruntu. Dajemy 0.05, by kamera zatrzymała się tuż nad trawą.
        if (angle_v < 0.05) angle_v = 0.05;
        
        update_camera();
    }

private:
    void update_camera() {
        // Cel kamery podniesiony (patrzymy w połowę wysokości latarni)
        Point3 lookat(0, 1.0, -1);
        Vec3 vup(0, 1, 0);

        double horizontal_distance = radius * std::cos(angle_v);
        double cam_x = lookat.x() + horizontal_distance * std::sin(angle_h);
        double cam_y = lookat.y() + radius * std::sin(angle_v);
        double cam_z = lookat.z() + horizontal_distance * std::cos(angle_h);
        
        Point3 lookfrom(cam_x, cam_y, cam_z);

        auto aspect_ratio = 16.0 / 9.0;
        auto viewport_height = 2.0;
        auto viewport_width = aspect_ratio * viewport_height;

        auto w = unit_vector(lookfrom - lookat);
        auto u = unit_vector(cross(vup, w));
        auto v = cross(w, u);

        origin = lookfrom;
        horizontal = u * viewport_width;
        vertical = v * viewport_height;
        lower_left_corner = origin - horizontal*0.5 - vertical*0.5 - w;
    }

    double radius = 4.0;
    double angle_h = 0.0;
    double angle_v = 0.2; 

    Point3 origin;
    Point3 lower_left_corner;
    Vec3 horizontal;
    Vec3 vertical;
};