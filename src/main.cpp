#include <iostream>
#include <fstream>
#include <memory>
#include "core/Vec3.h"
#include "core/Ray.h"
#include "geometry/Sphere.h"
#include "render/Camera.h"

// Funkcja obliczająca ostateczny kolor dla danego promienia światła. Sprawdza przecięcie promienia 
// z obiektami w świecie w określonym zakresie odległości. W przypadku trafienia oblicza oświetlenie 
// rozproszone na podstawie kąta między wektorem normalnym a wektorem światła, zwracając kolor. 
// W przypadku braku trafienia generuje tło w postaci płynnego, błękitno-białego gradientu.
Color ray_color(const Ray& r, const Hittable& world, const Vec3& light_dir) {
    HitRecord rec;
    if (world.hit(r, 0.001, 1000.0, rec)) {
        double diffuse_intensity = std::max(0.0, dot(rec.normal, light_dir));
        Color object_color(1.0, 0.2, 0.2); // Czerwona kula
        return object_color * diffuse_intensity;
    }
    Vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return Color(1.0, 1.0, 1.0)*(1.0-t) + Color(0.5, 0.7, 1.0)*t;
}

int main() {
    // Ustawienia obrazu
    // Ustala proporcje ekranu na 16:9 oraz 
    // matematycznie wylicza wysokość w pikselach na podstawie zadanej szerokości (800 pikseli).
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 800;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    // Inicjalizacja sceny i oświetlenia
    // Tworzenie wirtualnego środowiska. Umieszcza na scenie jeden obiekt (kulę o promieniu 0.5), 
    // inicjalizuje domyślną kamerę oraz definiuje globalny wektor kierunkowy dla źródła światła (słońce).
    Sphere world(Point3(0, 0, -1), 0.5);
    Camera cam;
    Vec3 light_dir = unit_vector(Vec3(1.0, 1.0, -1.0));

    // Przygotowanie pliku wyjściowego
    // Otwarcie strumienia zapisu do pliku output.ppm. Zapisuje obowiązkowy nagłówek tekstowego 
    // formatu obrazu PPM (P3), definiując jego rozdzielczość oraz maksymalną wartość koloru (255).
    std::ofstream out_file("output.ppm");
    out_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    // Główna pętla renderująca
    // Podwójna iteracja przechodząca przez każdy piksel obrazu (wiersz po wierszu, od góry do dołu, 
    // od lewej do prawej). Raportuje w konsoli liczbę pozostałych linii do wyrenderowania.
    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            
            // Generowanie promieni i próbkownie piksela
            // Matematyczne mapowanie pozycji piksela na znormalizowane współrzędne ekranu (U i V). 
            // Wykorzystuje te współrzędne do wygenerowania promienia z kamery do sceny.
            double u = double(i) / (image_width - 1);
            double v = double(j) / (image_height - 1);
            Ray r = cam.get_ray(u, v);
            
            Color pixel_color = ray_color(r, world, light_dir);
            
            // Konwersja i zapis koloru
            // Formatowanie obliczonych danych na potrzeby pliku. Przekształca zmiennoprzecinkowe 
            // składowe koloru na 8-bitowe wartości całkowite RGB (0-255) i zapisuje w pliku.
            int ir = static_cast<int>(255.999 * pixel_color.x());
            int ig = static_cast<int>(255.999 * pixel_color.y());
            int ib = static_cast<int>(255.999 * pixel_color.z());
            
            out_file << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }

    std::cerr << "\nDone.\n";
    return 0;
}