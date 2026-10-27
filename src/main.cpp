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

    while (window.isOpen()) {
        // 1. Pętla zdarzeń systemowych (tylko zdarzenia jednorazowe, np. kliknięcie 'X')
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // 2. Odczyt stanu klawiatury w czasie rzeczywistym (poza pętlą zdarzeń).
        // Umożliwia to płynny i ciągły ruch kamery (odczyt co klatkę), w przeciwieństwie do przerywanych zdarzeń (pollEvent).
        double speed = 0.2; // Stała prędkość obrotu/przybliżania
        double delta_h = 0.0; // Zmiana kąta w poziomie (azymut)
        double delta_v = 0.0; // Zmiana kąta w pionie (elewacja)
        double delta_z = 0.0; // Zmiana promienia (zoom)

        // Sumowanie wektorów ruchu, pozwala na poruszanie się po skosie, gdy wciśnięto dwa klawisze
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) delta_v += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) delta_v -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) delta_h -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) delta_h += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) delta_z += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) delta_z -= speed;

        // Jeśli zarejestrowano ruch orbitalny (zmianę kątów), wywołaj metodę i wymuś przeliczenie
        if (delta_h != 0.0 || delta_v != 0.0) {
            cam.orbit(delta_h, delta_v);
            needs_redraw = true;
        }
        // Jeśli zarejestrowano chęć przybliżenia/oddalenia, wywołaj zoom i wymuś przeliczenie
        if (delta_z != 0.0) {
            cam.zoom(delta_z);
            needs_redraw = true;
        }

        // 3. Przeliczanie nowej klatki (tylko gdy pozycja uległa zmianie)
        if (needs_redraw) {
            std::cerr << "\rRendering new frame..." << std::flush;
            for (unsigned int j = 0; j < image_height; ++j) {
                for (unsigned int i = 0; i < image_width; ++i) {
                    double u = double(i) / (image_width - 1);
                    // Odwrócona oś Y względem przestrzeni obrazu SFML
                    double v = double(image_height - 1 - j) / (image_height - 1);
                    Ray r = cam.get_ray(u, v);
                    
                    Color pixel_color = ray_color(r, world, light_dir);
                    
                    auto ir = static_cast<std::uint8_t>(255.999 * pixel_color.x());
                    auto ig = static_cast<std::uint8_t>(255.999 * pixel_color.y());
                    auto ib = static_cast<std::uint8_t>(255.999 * pixel_color.z());
                    
                    render_image.setPixel(sf::Vector2u(i, j), sf::Color(ir, ig, ib));
                }
            }
            
            // Transfer wyrenderowanego obrazu z RAM (sf::Image) do VRAM (sf::Texture) 
            (void)texture.loadFromImage(render_image);
            sprite.setTexture(texture, true);
            needs_redraw = false;
            std::cerr << "\rReady.                " << std::endl;
        }

        // Podwójne buforowanie: wyczyść -> narysuj nową teksturę -> wyświetl
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}