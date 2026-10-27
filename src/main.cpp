#include <iostream>
#include <memory>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdint>
#include "core/Vec3.h"
#include "core/Ray.h"
#include "geometry/Sphere.h"
#include "render/Camera.h"

// Oblicza kolor promienia, czyli kolor piksela widocznego na ekranie.
Color ray_color(const Ray& r, const Hittable& world, const Vec3& light_dir) {
    HitRecord rec;

    // Sprawdza, czy promień przecina obiekt w określonym zakresie odległości.
    if (world.hit(r, 0.001, 1000.0, rec)) {
        // Oblicza intensywność światła na podstawie kąta padania światła na powierzchnię.
        double diffuse_intensity = std::max(0.0, dot(rec.normal, light_dir));
        // Ustawia podstawowy kolor obiektu: czerwony.
        Color object_color(1.0, 0.2, 0.2);
        // Zwraca kolor obiektu pomnożony przez intensywność światła.
        return object_color * diffuse_intensity;
    }

    // Jeśli promień nie trafił w żaden obiekt, wyznacza kolor tła
    Vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    // Miesza dwa kolory: biały i jasnoniebieski, tworząc gradient nieba.
    return Color(1.0, 1.0, 1.0)*(1.0-t) + Color(0.5, 0.7, 1.0)*t;
}

int main() {
    // Parametry generowanego obrazu i okna aplikacji
    const auto aspect_ratio = 16.0 / 9.0;
    const unsigned int image_width = 800;
    const unsigned int image_height = static_cast<unsigned int>(image_width / aspect_ratio);

    // Inicjalizacja głównego okna programu z użyciem biblioteki SFML
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(image_width, image_height)), "SnowLight RayTracer");
    
    // sf::Image przechowuje tablicę pikseli w pamięci RAM przed wysłaniem jej do karty graficznej
    sf::Image render_image;
    render_image.resize(sf::Vector2u(image_width, image_height));

    // Definicja sceny 3D: kula, wirtualna kamera i wektor kierunkowy światła
    Sphere world(Point3(0, 0, -1), 0.5);
    Camera cam;
    Vec3 light_dir = unit_vector(Vec3(1.0, 1.0, -1.0));

    // Tekstura znajduje się w pamięci VRAM (karty graficznej), a Sprite służy do jej wyświetlania
    sf::Texture texture;
    sf::Sprite sprite(texture);
    
    // Flaga optymalizacyjna: określa, czy stan sceny się zmienił i wymaga kosztownego ponownego przeliczenia promieni
    bool needs_redraw = true; 

    // Pętla zdarzeń: działa dopóki okno programu nie zostanie zamknięte
    while (window.isOpen()) {

        // Obsługa komunikatów systemowych (zamknięcie okna, wciśnięcie klawisza) przy użyciu std::optional z SFML 3
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            // Reakcja na kliknięcie "X" na pasku okna
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            // Rejestrowanie wciśnięcia klawiszy WASD do poruszania kamerą w przestrzeni
            if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>()) {
                double speed = 0.2; // Stała prędkość ruchu kamery
                if (keyPress->code == sf::Keyboard::Key::W) { cam.move(Vec3(0, 0, -speed)); needs_redraw = true; }
                if (keyPress->code == sf::Keyboard::Key::S) { cam.move(Vec3(0, 0, speed)); needs_redraw = true; }
                if (keyPress->code == sf::Keyboard::Key::A) { cam.move(Vec3(-speed, 0, 0)); needs_redraw = true; }
                if (keyPress->code == sf::Keyboard::Key::D) { cam.move(Vec3(speed, 0, 0)); needs_redraw = true; }
            }
        }

        // Pętla raytracingu uruchamia się tylko, gdy zmieniono pozycję kamery (flaga needs_redraw == true)
        if (needs_redraw) {
            std::cerr << "\rRendering new frame..." << std::flush;
            for (unsigned int j = 0; j < image_height; ++j) {
                for (unsigned int i = 0; i < image_width; ++i) {
                    
                    // Normalizacja współrzędnych ekranu do zakresu 0.0 - 1.0
                    double u = double(i) / (image_width - 1);
                    // Odwrócenie osi Y dla prawidłowego rzutowania matematycznego w raytracingu względem układu SFML
                    double v = double(image_height - 1 - j) / (image_height - 1);
                    
                    Ray r = cam.get_ray(u, v);
                    Color pixel_color = ray_color(r, world, light_dir);
                    
                    // Konwersja wektora kolorów zmiennoprzecinkowych na 8-bitowy format RGBA używany przez bibliotekę
                    auto ir = static_cast<std::uint8_t>(255.999 * pixel_color.x());
                    auto ig = static_cast<std::uint8_t>(255.999 * pixel_color.y());
                    auto ib = static_cast<std::uint8_t>(255.999 * pixel_color.z());
                    
                    render_image.setPixel(sf::Vector2u(i, j), sf::Color(ir, ig, ib));
                }
            }
            
            // Transfer wyrenderowanego obrazu z RAM (sf::Image) do VRAM (sf::Texture) i podpięcie pod obiekt rysujący
            (void)texture.loadFromImage(render_image);
            sprite.setTexture(texture);
            
            // Zapis wygenerowanej klatki na dysk. Rzutowanie (void) ignoruje ostrzeżenie kompilatora [[nodiscard]] o ewentualnym błędzie zapisu.
            (void)render_image.saveToFile("output.png");
            
            // Resetowanie flagi, aby uniknąć zbędnych obliczeń w kolejnej klatce pętli zdarzeń
            needs_redraw = false;
            std::cerr << "\rReady.                " << std::endl;
        }

        // Wyczyszczenie okna (Double Buffering), narysowanie zaktualizowanej tekstury i wyświetlenie jej na ekranie
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}