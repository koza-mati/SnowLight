#include <iostream>
#include <memory>
#include <cmath>
#include <algorithm>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdint>

#include "core/Vec3.h"
#include "core/Ray.h"
#include "geometry/Sphere.h"
#include "core/HittableList.h"
#include "render/Camera.h"
#include "materials/Material.h"

Color ray_color(const Ray& r, const Hittable& world, int depth) {
    HitRecord rec;
    if (depth <= 0) return Color(0,0,0);

    if (world.hit(r, 0.001, 1000.0, rec)) {
        Ray scattered;
        Color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth-1);
        return Color(0,0,0);
    }

    Vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return Color(1.0, 1.0, 1.0)*(1.0-t) + Color(0.5, 0.7, 1.0)*t;
}

int main() {
    const auto aspect_ratio = 16.0 / 9.0;
    const unsigned int image_width = 800;
    const unsigned int image_height = static_cast<unsigned int>(image_width / aspect_ratio);

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(image_width, image_height)), "SnowLight RayTracer");
    sf::Image render_image;
    render_image.resize(sf::Vector2u(image_width, image_height));

// Definicja materiałów matowych
    auto material_ground = std::make_shared<Lambertian>(Color(0.2, 0.3, 0.1));
    auto material_red = std::make_shared<Lambertian>(Color(0.7, 0.1, 0.1));
    auto material_white = std::make_shared<Lambertian>(Color(0.9, 0.9, 0.9));
    auto material_glass_mock = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.3));

    // NOWOŚĆ: Definicja materiałów metalicznych (kolor RGB, chropowatość/fuzz)
    auto material_metal_silver = std::make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.0); // Idealne, gładkie lustro
    auto material_metal_gold   = std::make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.3); // Szczotkowane, matowe złoto

    HittableList world;
    // Ziemia
    world.add(std::make_shared<Sphere>(Point3(0.0, -100.0, -1.0), 100.0, material_ground));
    
    // Latarnia na środku
    world.add(std::make_shared<Sphere>(Point3(0.0, 0.5, -1.0), 0.5, material_red));
    world.add(std::make_shared<Sphere>(Point3(0.0, 1.2, -1.0), 0.4, material_white));
    world.add(std::make_shared<Sphere>(Point3(0.0, 1.8, -1.0), 0.3, material_white));
    world.add(std::make_shared<Sphere>(Point3(0.0, 2.2, -1.0), 0.2, material_glass_mock));
    world.add(std::make_shared<Sphere>(Point3(0.0, 2.45, -1.0), 0.25, material_red));

    // NOWE OBIEKTY: Metalowe ozdoby po bokach latarni
    world.add(std::make_shared<Sphere>(Point3(-1.0, 0.5, -0.3), 0.5, material_metal_silver)); // Srebrna (lewa)
    world.add(std::make_shared<Sphere>(Point3( 1.0, 0.5, -0.3), 0.5, material_metal_gold));   // Złota (prawa)

    Camera cam;
    sf::Texture texture;
    sf::Sprite sprite(texture);
    
    std::vector<Color> accumulation_buffer(image_width * image_height, Color(0,0,0));
    int frames_accumulated = 0;
    const int max_frames = 100; 

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        // Zmniejszone prędkości dla gładkiego sterowania
        double orbit_speed = 0.15; 
        double zoom_speed = 0.15;
        
        double delta_h = 0.0; double delta_v = 0.0; double delta_z = 0.0;
        bool camera_moved = false;

        // NOWE STEROWANIE - Ultra intuicyjne!
        // W/S = Zoom w przód i w tył (dystans do obiektu)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) { delta_z -= zoom_speed; camera_moved = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) { delta_z += zoom_speed; camera_moved = true; }
        
        // A/D = Krążenie wokół latarni (azymut)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) { delta_h -= orbit_speed; camera_moved = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) { delta_h += orbit_speed; camera_moved = true; }
        
        // Q/E = Latanie kamerą wyżej lub niżej nad ziemią (elewacja)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) { delta_v += orbit_speed; camera_moved = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) { delta_v -= orbit_speed; camera_moved = true; }

        if (camera_moved) {
            cam.orbit(delta_h, delta_v);
            cam.zoom(delta_z);
            frames_accumulated = 0;
            std::fill(accumulation_buffer.begin(), accumulation_buffer.end(), Color(0,0,0));
        }

        if (frames_accumulated < max_frames) {
            if (!camera_moved) frames_accumulated++;
            
            int current_depth = camera_moved ? 2 : 10;
            // Magiczna funkcja optymalizacji: skok co 4 piksele podczas ruchu!
            int step = camera_moved ? 4 : 1;
            
            std::vector<sf::Color> frame_colors(image_width * image_height);

// Multi-threading działa gładko, bo wątki nie wchodzą sobie w drogę
#pragma omp parallel for schedule(dynamic)
            for (int j = 0; j < (int)image_height; j += step) {
                for (int i = 0; i < (int)image_width; i += step) {
                    double u = (i + random_double()) / (image_width - 1);
                    double v = ((image_height - 1 - j) + random_double()) / (image_height - 1);
                    Ray r = cam.get_ray(u, v);
                    
                    Color pixel_color = ray_color(r, world, current_depth);
                    
                    if (camera_moved) {
                        // SZYBKI PODGLĄD W RUCHU
                        auto r_c = std::sqrt(pixel_color.x());
                        auto g_c = std::sqrt(pixel_color.y());
                        auto b_c = std::sqrt(pixel_color.z());
                        
                        auto ir = static_cast<std::uint8_t>(256 * std::clamp(r_c, 0.0, 0.999));
                        auto ig = static_cast<std::uint8_t>(256 * std::clamp(g_c, 0.0, 0.999));
                        auto ib = static_cast<std::uint8_t>(256 * std::clamp(b_c, 0.0, 0.999));
                        
                        // Zapisywanie "bloków" bezpiecznie do wektora (redukcja lagów o 94%!)
                        sf::Color sf_col(ir, ig, ib);
                        for (int dy = 0; dy < step && j + dy < (int)image_height; ++dy) {
                            for (int dx = 0; dx < step && i + dx < (int)image_width; ++dx) {
                                frame_colors[(j + dy) * image_width + (i + dx)] = sf_col;
                            }
                        }
                    } else {
                        // CZYSTY OBRAZ NA POSTOJU
                        int idx = j * image_width + i;
                        accumulation_buffer[idx] = accumulation_buffer[idx] + pixel_color;
                        Color avg_color = accumulation_buffer[idx] * (1.0 / frames_accumulated);
                        
                        auto r_c = std::sqrt(avg_color.x());
                        auto g_c = std::sqrt(avg_color.y());
                        auto b_c = std::sqrt(avg_color.z());
                        
                        auto ir = static_cast<std::uint8_t>(256 * std::clamp(r_c, 0.0, 0.999));
                        auto ig = static_cast<std::uint8_t>(256 * std::clamp(g_c, 0.0, 0.999));
                        auto ib = static_cast<std::uint8_t>(256 * std::clamp(b_c, 0.0, 0.999));
                        
                        frame_colors[idx] = sf::Color(ir, ig, ib);
                    }
                }
            }

            // Bezpieczne przepisywanie na ekran
            for (unsigned int j = 0; j < image_height; ++j) {
                for (unsigned int i = 0; i < image_width; ++i) {
                    // Aktualizujemy tylko te piksele, które klatka faktycznie ruszyła
                    if (camera_moved || (!camera_moved && frames_accumulated > 0)) {
                        render_image.setPixel(sf::Vector2u(i, j), frame_colors[j * image_width + i]);
                    }
                }
            }
            
            (void)texture.loadFromImage(render_image);
            sprite.setTexture(texture, true);
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }
    return 0;
}