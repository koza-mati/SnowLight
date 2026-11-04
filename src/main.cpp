#include <iostream>
#include <memory>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdint>

#include "core/Vec3.h"
#include "core/Ray.h"
#include "geometry/Sphere.h"
#include "render/Camera.h"
#include "materials/Material.h"

// Funkcja rekurencyjna symulująca fizyczne krążenie światła
Color ray_color(const Ray& r, const Hittable& world, int depth) {
    HitRecord rec;

    // Limit odbić - jeśli promień odbija się zbyt wiele razy (np. w ciasnym rogu), wygaszamy go
    if (depth <= 0)
        return Color(0,0,0);

    if (world.hit(r, 0.001, 1000.0, rec)) {
        Ray scattered;
        Color attenuation;
        // Odpytanie materiału o kierunek kolejnego odbicia i pochłanianie koloru
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth-1);
        return Color(0,0,0);
    }

    // Tło (niebo) - jedyne źródło światła w tej scenie
    Vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return Color(1.0, 1.0, 1.0)*(1.0-t) + Color(0.5, 0.7, 1.0)*t;
}

int main() {
    const auto aspect_ratio = 16.0 / 9.0;
    const unsigned int image_width = 800;
    const unsigned int image_height = static_cast<unsigned int>(image_width / aspect_ratio);
    
    // Parametry Path Tracingu (obniżone dla zachowania interaktywności nawigacji okna)
    const int samples_per_pixel = 5; 
    const int max_depth = 10;        

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(image_width, image_height)), "SnowLight RayTracer");
    sf::Image render_image;
    render_image.resize(sf::Vector2u(image_width, image_height));

    auto material_red = std::make_shared<Lambertian>(Color(1.0, 0.2, 0.2));
    Sphere world(Point3(0, 0, -1), 0.5, material_red);
    
    Camera cam;
    sf::Texture texture;
    sf::Sprite sprite(texture);
    
    bool needs_redraw = true; 

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        double speed = 0.2;
        double delta_h = 0.0; double delta_v = 0.0; double delta_z = 0.0;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) delta_v += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) delta_v -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) delta_h -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) delta_h += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) delta_z += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) delta_z -= speed;

        if (delta_h != 0.0 || delta_v != 0.0) { cam.orbit(delta_h, delta_v); needs_redraw = true; }
        if (delta_z != 0.0) { cam.zoom(delta_z); needs_redraw = true; }

        if (needs_redraw) {
            std::cerr << "\rRendering new frame..." << std::flush;
            for (unsigned int j = 0; j < image_height; ++j) {
                for (unsigned int i = 0; i < image_width; ++i) {
                    Color pixel_color(0, 0, 0);
                    
                    // Wielokrotne próbkowanie piksela
                    for (int s = 0; s < samples_per_pixel; ++s) {
                        double u = (i + random_double()) / (image_width - 1);
                        double v = ((image_height - 1 - j) + random_double()) / (image_height - 1);
                        Ray r = cam.get_ray(u, v);
                        pixel_color = pixel_color + ray_color(r, world, max_depth);
                    }
                    
                    // Uśrednienie koloru na podstawie próbek
                    auto scale = 1.0 / samples_per_pixel;
                    auto r = std::sqrt(pixel_color.x() * scale); // Pierwiastek to korekcja Gamma
                    auto g = std::sqrt(pixel_color.y() * scale);
                    auto b = std::sqrt(pixel_color.z() * scale);
                    
                    auto ir = static_cast<std::uint8_t>(256 * std::clamp(r, 0.0, 0.999));
                    auto ig = static_cast<std::uint8_t>(256 * std::clamp(g, 0.0, 0.999));
                    auto ib = static_cast<std::uint8_t>(256 * std::clamp(b, 0.0, 0.999));
                    
                    render_image.setPixel(sf::Vector2u(i, j), sf::Color(ir, ig, ib));
                }
            }
            (void)texture.loadFromImage(render_image);
            sprite.setTexture(texture, true);
            needs_redraw = false;
            std::cerr << "\rReady.                " << std::endl;
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }
    return 0;
}