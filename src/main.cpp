#include <iostream>
#include <memory>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdint>

#include "core/Vec3.h"
#include "core/Ray.h"
#include "core/HittableList.h"
#include "core/BVH.h"
#include "geometry/Sphere.h"
#include "geometry/Triangle.h"
#include "geometry/ObjLoader.h"
#include "geometry/MeshGenerator.h"
#include "render/Camera.h"
#include "materials/Material.h"

Color background_sky(const Ray& r) {
    Vec3 d = unit_vector(r.direction());
    double t = std::clamp(d.y(), 0.0, 1.0);

    Color horizon(0.04, 0.10, 0.20);
    Color zenith(0.01, 0.02, 0.05);
    Color sky = horizon * (1.0 - std::pow(t, 0.6)) + zenith * std::pow(t, 0.6);

    if (d.y() > 0.04 && d.y() < 0.45) {
        double wave = std::sin(d.x() * 5.0 + d.z() * 3.0) * 0.5 + 0.5;
        double band = std::exp(-std::pow((d.y() - 0.18) / 0.09, 2.0)) * wave;
        sky = sky + Color(0.02, 0.12, 0.10) * band;
    }

    if (d.y() > 0.03) {
        double scale = 75.0;
        double gx = std::floor(d.x() * scale);
        double gy = std::floor(d.y() * scale);
        double gz = std::floor(d.z() * scale);

        auto hash3 = [](double x, double y, double z) {
            double h = std::sin(x * 127.1 + y * 311.7 + z * 74.7) * 43758.5453123;
            return h - std::floor(h);
        };

        double h1 = hash3(gx, gy, gz);
        if (h1 > 0.94) {
            double cx = (gx + 0.2 + 0.6 * hash3(gx + 1.0, gy, gz)) / scale;
            double cy = (gy + 0.2 + 0.6 * hash3(gx, gy + 1.0, gz)) / scale;
            double cz = (gz + 0.2 + 0.6 * hash3(gx, gy, gz + 1.0)) / scale;
            double dist_sq = (d.x() - cx) * (d.x() - cx) + (d.y() - cy) * (d.y() - cy) + (d.z() - cz) * (d.z() - cz);
            double radius_sq = (0.14 / scale) * (0.14 / scale);
            if (dist_sq < radius_sq) {
                double falloff = 1.0 - (dist_sq / radius_sq);
                sky = sky + Color(0.9, 0.95, 1.0) * (falloff * falloff * 1.8);
            }
        }
    }
    return sky;
}

// Charakterystyka kierunkowa obrotowego reflektora (omywa zarówno ramiona płatka w płaszczyźnie XY, jak i taflę lodu)
inline double lighthouse_beam_intensity(const Vec3& dir_from_bulb, const Vec3& beam_axis) {
    double align = std::abs(dot(dir_from_bulb, beam_axis));
    double focused = (align > 0.72) ? std::pow((align - 0.72) / 0.28, 2.2) * 24.0 : 0.0;
    return 0.12 + focused;
}

Color trace_transmittance(const Ray& shadow_ray, const Hittable& world, double max_dist, int max_steps = 4) {
    Color transmittance(1.0, 1.0, 1.0);
    Ray current_ray = shadow_ray;
    double remaining_dist = max_dist;

    for (int bounce = 0; bounce < max_steps; ++bounce) {
        HitRecord srec;
        if (!world.hit(current_ray, 0.001, remaining_dist, srec)) {
            return transmittance;
        }

        Color glass_tint = srec.mat_ptr->transparency_tint();
        if (glass_tint.length_squared() > 0.0) {
            transmittance = transmittance * glass_tint;
            remaining_dist -= srec.t;
            if (remaining_dist <= 0.002) return transmittance;
            current_ray = Ray(srec.p + current_ray.direction() * 0.001, current_ray.direction());
        } else {
            return Color(0, 0, 0);
        }
    }
    return transmittance;
}

// Szybka, analityczna wolumetria stożka światła (najbliższy punkt między promieniem kamery a osią reflektora)
// Szybka, analityczna wolumetria stożka światła (zabezpieczona przed wypalaniem na powierzchniach)
Color compute_fast_volumetric_beam(
    const Ray& r,
    double max_t,
    const Hittable& world,
    const Point3& lantern_pos,
    double lantern_radius,
    const Vec3& beam_axis,
    bool hit_surface
) {
    // Jeśli promień trafia w obiekt blisko kamery, skracamy zasięg wolumetryki, by nie tworzyć "plam" na szkle
    double march_dist = hit_surface ? std::min(max_t, 8.0) : 8.0;
    if (march_dist <= 0.05) return Color(0, 0, 0);

    Vec3 ro = r.origin() - lantern_pos;
    Vec3 rd = unit_vector(r.direction());

    double b = dot(ro, beam_axis);
    double c = dot(rd, beam_axis);
    double d = dot(ro, rd);
    double denom = 1.0 - c * c;
    if (denom < 1e-5) return Color(0, 0, 0);

    double t_closest = (b * c - d) / denom;
    if (t_closest <= 0.05 || t_closest >= march_dist) return Color(0, 0, 0);

    Point3 pt = r.at(t_closest);
    Vec3 from_bulb = pt - lantern_pos;
    double dist_sq = from_bulb.length_squared();
    double dist = std::sqrt(dist_sq);
    if (dist <= lantern_radius * 1.3) return Color(0, 0, 0);

    Vec3 dir_from_bulb = from_bulb * (1.0 / dist);
    double align = std::abs(dot(dir_from_bulb, beam_axis));
    if (align <= 0.86) return Color(0, 0, 0);

    // Test cienia dla szprosów laterny
    Ray vis_ray(lantern_pos + dir_from_bulb * (lantern_radius + 0.003), dir_from_bulb);
    Color trans = trace_transmittance(vis_ray, world, dist - lantern_radius - 0.005, 3);
    if (trans.length_squared() <= 0.0) return Color(0, 0, 0);

    double cone = std::pow((align - 0.86) / 0.14, 2.2);
    double glow = cone / (1.0 + 0.28 * dist_sq);
    
    // Delikatny, klimatyczny złoty snop światła
    return Color(1.0, 0.78, 0.35) * trans * (glow * 0.32);
}

Color ray_color(
    const Ray& r,
    const Hittable& world,
    const Point3& lantern_pos,
    double lantern_radius,
    const Vec3& beam_axis,
    int depth,
    int bounce = 0,
    bool camera_moving = false
) {
    HitRecord rec;
    if (depth <= 0) return background_sky(r) * 0.25;

    bool hit_world = world.hit(r, 0.001, 1000.0, rec);
    double hit_dist = hit_world ? rec.t : 12.0;

    // Wolumetria liczona tylko w wolnej przestrzeni lub wewnątrz kryształu (pomijamy przednie tafle)
    Color volumetric(0, 0, 0);
    if (!camera_moving && bounce <= 1) {
        bool skip_vol = hit_world && rec.mat_ptr->transparency_tint().length_squared() > 0.0 && rec.front_face;
        if (!skip_vol) {
            volumetric = compute_fast_volumetric_beam(r, hit_dist, world, lantern_pos, lantern_radius, beam_axis, hit_world);
            if (bounce == 1) volumetric = volumetric * 0.5;
        }
    }
    // ... dalsza część funkcji ray_color pozostaje bez zmian
    if (hit_world) {
        Ray scattered;
        Color attenuation;
        Color emitted = rec.mat_ptr->emitted();

        if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return volumetric + emitted;

        Color direct_light(0, 0, 0);
        Vec3 moon_dir = unit_vector(Vec3(-0.45, 0.78, 0.43));

        int max_nee_bounce = camera_moving ? 0 : 1;
        int shadow_steps = camera_moving ? 2 : 4;

        if (bounce <= max_nee_bounce) {
            Vec3 to_lantern = lantern_pos - rec.p;
            double dist_sq = to_lantern.length_squared();
            double dist = std::sqrt(dist_sq);
            Vec3 dir_lantern = to_lantern * (1.0 / dist);
            Vec3 dir_from_bulb = -dir_lantern;

            double beam_gain = lighthouse_beam_intensity(dir_from_bulb, beam_axis);

            if (!rec.mat_ptr->is_specular()) {
                Color albedo = rec.mat_ptr->get_albedo();

                // 1. Oświetlenie rozproszone od obracającego się snopa latarni
                double ndotl_lantern = dot(rec.normal, dir_lantern);
                if (ndotl_lantern > 0.0 && dist > lantern_radius + 0.01) {
                    Ray shadow_ray(rec.p + rec.normal * 0.001, dir_lantern);
                    Color trans = trace_transmittance(shadow_ray, world, dist - lantern_radius - 0.003, shadow_steps);
                    if (trans.length_squared() > 0.0) {
                        Color lantern_radiance(1.5, 1.0, 0.40);
                        double falloff = 1.0 / (1.0 + 0.28 * dist_sq);
                        direct_light = direct_light + albedo * lantern_radiance * trans * (ndotl_lantern * falloff * beam_gain);
                    }
                }

                // 2. Złoty refleks snopa latarni na tafli zamarzniętego jeziora
                Vec3 view_dir = unit_vector(r.direction());
                Vec3 refl_dir = reflect(view_dir, rec.normal);
                double spec_lake = std::max(0.0, dot(refl_dir, dir_lantern));
                if (spec_lake > 0.75 && beam_gain > 0.3 && dist > lantern_radius + 0.01) {
                    Ray shadow_ray(rec.p + rec.normal * 0.001, dir_lantern);
                    Color trans = trace_transmittance(shadow_ray, world, dist - lantern_radius - 0.003, shadow_steps);
                    if (trans.length_squared() > 0.0) {
                        double glint = std::pow(spec_lake, 25.0) * (0.55 * beam_gain / (1.0 + 0.22 * dist_sq));
                        direct_light = direct_light + Color(1.0, 0.80, 0.38) * trans * glint;
                    }
                }

                // 3. Zimowe światło księżyca
                double ndotl_moon = dot(rec.normal, moon_dir);
                if (ndotl_moon > 0.0) {
                    Ray moon_ray(rec.p + rec.normal * 0.001, moon_dir);
                    Color trans = trace_transmittance(moon_ray, world, 100.0, shadow_steps);
                    if (trans.length_squared() > 0.0) {
                        Color moon_radiance(0.22, 0.36, 0.64);
                        direct_light = direct_light + albedo * moon_radiance * trans * ndotl_moon;
                    }
                }
            } else {
                // Optyka krystaliczna lodu: wyraźne załamania, odbicia Fresnela i dyspersja pryzmatyczna na ramionach płatka
                Vec3 view_dir = unit_vector(r.direction());
                Vec3 refl_dir = reflect(view_dir, rec.normal);
                double cos_theta = std::fmin(dot(-view_dir, rec.normal), 1.0);
                double fresnel = 0.12 + 0.88 * std::pow(1.0 - cos_theta, 2.5);

                // Kąt pomiędzy normalną kryształu a kierunkiem z lampy (im bliżej grani/pryzmatu, tym silniejsze załamanie)
                double facet_alignment = std::abs(dot(rec.normal, dir_from_bulb));
                double edge_factor = 1.0 - cos_theta; // Silne na ukośnych ściankach i końcówkach ramion

                // 1. Wewnętrzne rozświetlenie lodu przez obracającą się wiązkę (Subsurface Crystal Glow)
                double crystal_glow = (0.25 + 0.75 * edge_factor) * (0.35 + 0.65 * facet_alignment);
                double falloff = 1.0 / (1.0 + 0.22 * dist_sq);
                double light_power = crystal_glow * falloff * beam_gain * 0.45;

                // Dyspersja chromatyczna: krawędzie kryształu rozszczepiają ciepłe światło na złoto i lodowy błękit
                Color dispersion_color(
                    1.0 * (1.0 - 0.35 * edge_factor),
                    0.82,
                    0.42 + 0.58 * edge_factor
                );
                direct_light = direct_light + dispersion_color * light_power;

                // 2. Ostre odbicia zwierciadlane (Fresnel) na ściankach ramion płatka
                double spec_refl = std::max(0.0, dot(refl_dir, dir_lantern));
                if (spec_refl > 0.50 && dist > lantern_radius + 0.01) {
                    double glint = std::pow(spec_refl, 10.0) * fresnel * (1.4 * beam_gain * falloff);
                    direct_light = direct_light + Color(1.0, 0.85, 0.48) * glint;
                }

                // 3. Stały kontur księżyca na krawędziach lodu (aby kryształ był zawsze trójwymiarowy)
                double spec_moon = std::max(0.0, dot(refl_dir, moon_dir));
                double rim_light = std::pow(edge_factor, 2.2) * 0.28 + std::pow(spec_moon, 40.0) * fresnel * 1.4;
                direct_light = direct_light + Color(0.55, 0.78, 1.0) * rim_light;
            }
        }

        Color indirect = attenuation * ray_color(scattered, world, lantern_pos, lantern_radius, beam_axis, depth - 1, bounce + 1, camera_moving);
        return volumetric + emitted + direct_light + indirect;
    }

    return volumetric + background_sky(r);
}

int main() {
    const auto aspect_ratio = 16.0 / 9.0;
    const unsigned int image_width = 800;
    const unsigned int image_height = static_cast<unsigned int>(image_width / aspect_ratio);

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(image_width, image_height)), "SnowLight RayTracer [SPACJA: Pauza/Start | Strzalki: Obrot swiatla]");

    sf::Image render_image;
    render_image.resize(sf::Vector2u(image_width, image_height));

    // Materiały sceny
    auto material_frozen_lake  = std::make_shared<FrozenLake>(Color(0.03, 0.08, 0.16), 1.31, 0.012);
    auto material_inner_snow   = std::make_shared<Lambertian>(Color(0.88, 0.93, 0.99));
    auto material_tower_white  = std::make_shared<Lambertian>(Color(0.95, 0.96, 0.99));
    auto material_tower_red    = std::make_shared<Lambertian>(Color(0.88, 0.14, 0.14));
    auto material_metal_dark   = std::make_shared<Metal>(Color(0.24, 0.27, 0.32), 0.16);
    auto material_lantern_bulb = std::make_shared<DiffuseLight>(Color(6.0, 4.4, 1.8));
    auto material_glass        = std::make_shared<Dielectric>(1.50, Color(0.97, 0.99, 1.0));
    auto material_ice          = std::make_shared<Dielectric>(1.31, Color(0.82, 0.93, 1.0));

    MeshGenerator::ensure_scene_models_exist("models");

    HittableList world;
    world.add(std::make_shared<Sphere>(Point3(0.0, -10000.38, -1.0), 10000.0, material_frozen_lake));

    const double scene_scale = 0.68;
    const Vec3 snowflake_center(0.0, 1.22, -1.0);

    auto snowflake_mesh = ObjLoader::load_obj("models/snowflake_crystal.obj", material_ice, scene_scale, snowflake_center);
    if (snowflake_mesh) world.add(snowflake_mesh);

    auto inner_snow = ObjLoader::load_obj("models/lighthouse_snow_base.obj", material_inner_snow, scene_scale, snowflake_center);
    if (inner_snow) world.add(inner_snow);

    const double lh_scale = 0.62 * scene_scale;
    const Vec3 lighthouse_pos(snowflake_center.x(), snowflake_center.y() - 0.74 * scene_scale, snowflake_center.z());

    auto lh_white = ObjLoader::load_obj("models/lighthouse_white.obj", material_tower_white, lh_scale, lighthouse_pos);
    auto lh_red   = ObjLoader::load_obj("models/lighthouse_red.obj",   material_tower_red,   lh_scale, lighthouse_pos);
    auto lh_metal = ObjLoader::load_obj("models/lighthouse_metal.obj", material_metal_dark,  lh_scale, lighthouse_pos);
    auto lh_glass = ObjLoader::load_obj("models/lighthouse_glass.obj", material_glass,       lh_scale, lighthouse_pos);

    if (lh_white) world.add(lh_white);
    if (lh_red)   world.add(lh_red);
    if (lh_metal) world.add(lh_metal);
    if (lh_glass) world.add(lh_glass);

    const Point3 lantern_bulb_pos(lighthouse_pos.x(), lighthouse_pos.y() + 1.83 * lh_scale, lighthouse_pos.z());
    const double lantern_bulb_radius = 0.115 * lh_scale;
    world.add(std::make_shared<Sphere>(lantern_bulb_pos, lantern_bulb_radius, material_lantern_bulb));

    BVHNode scene_bvh(world);

    Camera cam;
    sf::Texture texture;
    sf::Sprite sprite(texture);

    std::vector<Color> accumulation_buffer(image_width * image_height, Color(0, 0, 0));
    std::vector<sf::Color> frame_colors(image_width * image_height, sf::Color::Black);
    int frames_accumulated = 0;
    const int max_frames = 160;
    unsigned int frame_parity = 0;

    double beam_angle = 0.45;
    bool beam_rotating = true;
    sf::Clock frame_clock;

    while (window.isOpen()) {
        double dt = frame_clock.restart().asSeconds();
        dt = std::min(dt, 0.1);

        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::Space) {
                    beam_rotating = !beam_rotating;
                    frames_accumulated = 0;
                    std::fill(accumulation_buffer.begin(), accumulation_buffer.end(), Color(0, 0, 0));
                }
            }
        }

        double orbit_speed = 0.08;
        double zoom_speed = 0.12;

        double delta_h = 0.0; double delta_v = 0.0; double delta_z = 0.0;
        bool camera_moved = false;
        bool beam_moved = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) { delta_z -= zoom_speed; camera_moved = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) { delta_z += zoom_speed; camera_moved = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) { delta_h -= orbit_speed; camera_moved = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) { delta_h += orbit_speed; camera_moved = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) { delta_v += orbit_speed; camera_moved = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) { delta_v -= orbit_speed; camera_moved = true; }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  { beam_angle -= 1.8 * dt; beam_moved = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) { beam_angle += 1.8 * dt; beam_moved = true; }

        if (beam_rotating) {
            beam_angle += 1.1 * dt;
            beam_moved = true;
        }

        // Oś obracającego się reflektora pochylona w 3D, aby omiatała kolejne z 6 ramion płatka i taflę lodu
        Vec3 beam_axis = unit_vector(Vec3(
            std::cos(beam_angle),
            0.42 * std::sin(beam_angle * 2.0) - 0.12,
            std::sin(beam_angle)
        ));

        if (camera_moved) {
            cam.orbit(delta_h, delta_v);
            cam.zoom(delta_z);
            frames_accumulated = 0;
            std::fill(accumulation_buffer.begin(), accumulation_buffer.end(), Color(0, 0, 0));
        }

        if (frames_accumulated < max_frames) {
            if (!camera_moved && !beam_moved) frames_accumulated++;

            int current_depth = camera_moved ? 4 : (beam_moved ? 8 : 18);
            int step = camera_moved ? 5 : 1;
            int phase = frame_parity % 4;
            frame_parity++;

            auto soft_tonemap = [](double x) {
                x = std::max(0.0, x);
                return x / (1.0 + 0.18 * x);
            };

#pragma omp parallel for schedule(dynamic, 4)
            for (int j = 0; j < (int)image_height; j += step) {
                for (int i = 0; i < (int)image_width; i += step) {
                    // Interlacing 2x2 podczas obrotu wiązki: przeliczamy 25% pikseli na klatkę -> 4x wyższy FPS bez pikselozy!
                    if (!camera_moved && beam_moved && frames_accumulated > 0) {
                        int sub = (j % 2) * 2 + (i % 2);
                        if (sub != phase) continue;
                    }

                    double u = (i + (camera_moved ? 0.5 : random_double())) / (image_width - 1);
                    double v = ((image_height - 1 - j) + (camera_moved ? 0.5 : random_double())) / (image_height - 1);
                    Ray r = cam.get_ray(u, v);

                    Color pixel_color = ray_color(r, scene_bvh, lantern_bulb_pos, lantern_bulb_radius, beam_axis, current_depth, 0, camera_moved);

                    if (camera_moved) {
                        auto r_c = std::sqrt(soft_tonemap(pixel_color.x()));
                        auto g_c = std::sqrt(soft_tonemap(pixel_color.y()));
                        auto b_c = std::sqrt(soft_tonemap(pixel_color.z()));

                        auto ir = static_cast<std::uint8_t>(256 * std::clamp(r_c, 0.0, 0.999));
                        auto ig = static_cast<std::uint8_t>(256 * std::clamp(g_c, 0.0, 0.999));
                        auto ib = static_cast<std::uint8_t>(256 * std::clamp(b_c, 0.0, 0.999));

                        sf::Color sf_col(ir, ig, ib);
                        for (int dy = 0; dy < step && j + dy < (int)image_height; ++dy) {
                            for (int dx = 0; dx < step && i + dx < (int)image_width; ++dx) {
                                frame_colors[(j + dy) * image_width + (i + dx)] = sf_col;
                            }
                        }
                    } else if (beam_moved) {
                        int idx = j * image_width + i;
                        if (frames_accumulated == 0) {
                            accumulation_buffer[idx] = pixel_color;
                        } else {
                            accumulation_buffer[idx] = accumulation_buffer[idx] * 0.35 + pixel_color * 0.65;
                        }

                        Color disp = accumulation_buffer[idx];
                        auto r_c = std::sqrt(soft_tonemap(disp.x()));
                        auto g_c = std::sqrt(soft_tonemap(disp.y()));
                        auto b_c = std::sqrt(soft_tonemap(disp.z()));

                        auto ir = static_cast<std::uint8_t>(256 * std::clamp(r_c, 0.0, 0.999));
                        auto ig = static_cast<std::uint8_t>(256 * std::clamp(g_c, 0.0, 0.999));
                        auto ib = static_cast<std::uint8_t>(256 * std::clamp(b_c, 0.0, 0.999));

                        frame_colors[idx] = sf::Color(ir, ig, ib);
                    } else {
                        int idx = j * image_width + i;
                        accumulation_buffer[idx] = accumulation_buffer[idx] + pixel_color;
                        Color avg_color = accumulation_buffer[idx] * (1.0 / frames_accumulated);

                        auto r_c = std::sqrt(soft_tonemap(avg_color.x()));
                        auto g_c = std::sqrt(soft_tonemap(avg_color.y()));
                        auto b_c = std::sqrt(soft_tonemap(avg_color.z()));

                        auto ir = static_cast<std::uint8_t>(256 * std::clamp(r_c, 0.0, 0.999));
                        auto ig = static_cast<std::uint8_t>(256 * std::clamp(g_c, 0.0, 0.999));
                        auto ib = static_cast<std::uint8_t>(256 * std::clamp(b_c, 0.0, 0.999));

                        frame_colors[idx] = sf::Color(ir, ig, ib);
                    }
                }
            }

            if (beam_moved && !camera_moved) {
                frames_accumulated = 1;
            }

            for (unsigned int j = 0; j < image_height; ++j) {
                for (unsigned int i = 0; i < image_width; ++i) {
                    render_image.setPixel(sf::Vector2u(i, j), frame_colors[j * image_width + i]);
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