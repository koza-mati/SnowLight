#pragma once
#include "../core/Vec3.h"
#include "../core/Ray.h"

// Zarządza wirutalnym obserwatorem, odpowiedzialnym za generowanie promieni przechodzących przez siatkę pikseli matrycy (viewpoint)

class Camera {
public:
    Camera() {
        auto aspect_ratio = 16.0 / 9.0;
        auto viewport_height = 2.0;
        auto viewport_width = aspect_ratio * viewport_height;
        auto focal_length = 1.0;

        origin = Point3(0, 0, 0);
        horizontal = Vec3(viewport_width, 0.0, 0.0);
        vertical = Vec3(0.0, viewport_height, 0.0);
        lower_left_corner = origin - horizontal*(0.5) - vertical*(0.5) - Vec3(0, 0, focal_length);
    }

    // Transformuje współrzędne 2D z płaszczyzny ekarny (u,v), na konkrenty wektor 3D wychodzący z soczewki kamery w świat sceny
    Ray get_ray(double u, double v) const {
        return Ray(origin, lower_left_corner + horizontal*u + vertical*v - origin);
    }

private:
    Point3 origin;
    Point3 lower_left_corner;
    Vec3 horizontal;
    Vec3 vertical;
};