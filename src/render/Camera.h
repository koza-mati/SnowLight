#pragma once
#include <cmath>
#include "../core/Vec3.h"
#include "../core/Ray.h"

// Wirtualny obserwator wykorzystujący system "Look-at" oraz układ sferyczny.
// Zamiast poruszać się liniowo, kamera orbituje dookoła wyznaczonego celu, co jest wygodniejsze w inspekcji obiektów 3D.
class Camera {
public:
    Camera() {
        // Wymusza przeliczenie wektorów początkowych przy starcie programu
        update_camera();
    }

    // Transformuje współrzędne 2D z płaszczyzny ekranu na promień w przestrzeni 3D
    Ray get_ray(double u, double v) const {
        return Ray(origin, lower_left_corner + horizontal*u + vertical*v - origin);
    }
    
    // Zmienia dystans kamery od obserwowanego celu (przybliżanie i oddalanie)
    void zoom(double delta) {
        radius += delta;
        // Zabezpieczenie przed "wjechaniem" kamery za punkt centralny (odległość nie może być ujemna ani zerowa)
        if (radius < 0.2) radius = 0.2;
        update_camera();
    }

    // Zmienia kąty obserwacji w układzie sferycznym, obracając kamerę dookoła celu
    void orbit(double delta_h, double delta_v) {
        angle_h += delta_h; // Kąt w poziomie (azymut)
        angle_v += delta_v; // Kąt w pionie (elewacja)
        
        // Blokada pionowa zapobiegająca zjawisku "Gimbal Lock" i nienaturalnemu odwróceniu kamery do góry nogami
        // 1.5707 to w przybliżeniu Pi/2 (90 stopni).
        const double limit = 1.5707; 
        if (angle_v > limit) angle_v = limit;
        if (angle_v < -limit) angle_v = -limit;
        
        // Po każdej zmianie parametrów przeliczamy na nowo kartezjański układ odniesienia
        update_camera();
    }

private:
    // Serce kamery orbitalnej. Przelicza kąty i promień ze sferycznego układu współrzędnych 
    // z powrotem na przestrzeń wektorową 3D (X, Y, Z) potrzebną do raytracingu.
    void update_camera() {
        // Cel kamery, stały punkt, dookoła którego orbitujemy (w tym wypadku nasza kula)
        Point3 lookat(0, 0, -1);
        // Globalny wektor wskazujący absolutną "górę" świata. Potrzebny do wyliczenia osi lokalnych kamery.
        Vec3 vup(0, 1, 0);

        // Konwersja współrzędnych sferycznych (promień, kąt pionowy, kąt poziomy) na pozycję w układzie kartezjańskim 3D
        double horizontal_distance = radius * std::cos(angle_v);
        double cam_x = lookat.x() + horizontal_distance * std::sin(angle_h);
        double cam_y = lookat.y() + radius * std::sin(angle_v);
        double cam_z = lookat.z() + horizontal_distance * std::cos(angle_h);
        
        // Finalne stanowisko wirtualnego obserwatora
        Point3 lookfrom(cam_x, cam_y, cam_z);

        auto aspect_ratio = 16.0 / 9.0;
        auto viewport_height = 2.0;
        auto viewport_width = aspect_ratio * viewport_height;

        // Tworzenie bazy ortonormalnej (lokalnego układu współrzędnych kamery)
        // w - wektor patrzenia (w głąb)
        auto w = unit_vector(lookfrom - lookat);
        // u - wektor "w prawo" (iloczyn wektorowy wektora "góry" i wektora patrzenia daje nam oś X kamery)
        auto u = unit_vector(cross(vup, w));
        // v - wektor "w górę" samej kamery (zapewnia poprawne rzutowanie przy pochyleniach)
        auto v = cross(w, u);

        origin = lookfrom;
        horizontal = u * viewport_width;
        vertical = v * viewport_height;
        lower_left_corner = origin - horizontal*0.5 - vertical*0.5 - w;
    }

    // Stan wewnętrzny układu sferycznego
    double radius = 1.0;
    double angle_h = 0.0;
    double angle_v = 0.0;

    // Obliczona geometria matrycy (viewport) używana w get_ray()
    Point3 origin;
    Point3 lower_left_corner;
    Vec3 horizontal;
    Vec3 vertical;
};