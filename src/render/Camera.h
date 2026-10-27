#pragma once
#include "../core/Vec3.h"
#include "../core/Ray.h"

// Zarządza wirtualnym obserwatorem, odpowiedzialnym za generowanie promieni przechodzących przez siatkę pikseli matrycy (viewpoint)
class Camera {
public:
    Camera() {
        // Definiowanie proporcji obrazu i rozmiaru wirtualnej matrycy (viewport) w świecie 3D
        auto aspect_ratio = 16.0 / 9.0;
        auto viewport_height = 2.0;
        auto viewport_width = aspect_ratio * viewport_height;
        
        // Odległość matrycy od punktu początkowego kamery (tzw. ogniskowa)
        auto focal_length = 1.0;

        // Środek układu współrzędnych kamery (położenie "oka")
        origin = Point3(0, 0, 0);
        
        // Wektory rozpinające płaszczyznę ekranu w poziomie i w pionie
        horizontal = Vec3(viewport_width, 0.0, 0.0);
        vertical = Vec3(0.0, viewport_height, 0.0);
        
        // Wyznaczenie lewego dolnego rogu wirtualnego ekranu w przestrzeni 3D.
        // Od punktu oka odejmujemy połowę szerokości, połowę wysokości oraz przesuwamy w głąb (oś Z) o ogniskową.
        lower_left_corner = origin - horizontal*(0.5) - vertical*(0.5) - Vec3(0, 0, focal_length);
    }

    // Transformuje znormalizowane współrzędne 2D z płaszczyzny ekranu (u,v w zakresie 0.0-1.0) 
    // na konkretny wektor 3D wychodzący z soczewki kamery w świat sceny.
    Ray get_ray(double u, double v) const {
        // Promień zaczyna się w origin, a jego kierunek to wektor od origin do konkretnego punktu na płaszczyźnie ekranu
        return Ray(origin, lower_left_corner + horizontal*u + vertical*v - origin);
    }
    
    // Umożliwia translację (przesunięcie) kamery w przestrzeni o zadany wektor.
    void move(const Vec3& offset) {
        // Aktualizacja głównej pozycji kamery
        origin = origin + offset;
        // Płaszczyzna rzutowania (ekran) musi poruszać się razem z kamerą, 
        // dlatego lewy dolny róg matrycy przesuwamy o ten sam wektor.
        lower_left_corner = lower_left_corner + offset;
    }

private:
    Point3 origin;
    Point3 lower_left_corner;
    Vec3 horizontal;
    Vec3 vertical;
};