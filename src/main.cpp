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
// Sprawdza, czy promień trafia w obiekt, a następnie uwzględnia oświetlenie.
Color ray_color(const Ray& r, const Hittable& world, const Vec3& light_dir) {
    HitRecord rec;

    // Sprawdza, czy promień przecina obiekt w określonym zakresie odległości.
    // Wartość 0.001 zapobiega błędom związanym z ponownym trafieniem w tę samą powierzchnię.
    if (world.hit(r, 0.001, 1000.0, rec)) {

        // Oblicza intensywność światła na podstawie kąta padania światła na powierzchnię.
        // Iloczyn skalarny normalnej i kierunku światła określa, jak mocno oświetlony jest punkt.
        // std::max(0.0, ...) zapobiega uzyskaniu ujemnego oświetlenia.
        double diffuse_intensity = std::max(0.0, dot(rec.normal, light_dir));

        // Ustawia podstawowy kolor obiektu: czerwony.
        Color object_color(1.0, 0.2, 0.2);

        // Zwraca kolor obiektu pomnożony przez intensywność światła.
        // Im słabsze oświetlenie, tym ciemniejszy będzie obiekt.
        return object_color * diffuse_intensity;
    }

    // Jeśli promień nie trafił w żaden obiekt, wyznacza kolor tła
    // na podstawie kierunku, w którym porusza się promień.
    Vec3 unit_direction = unit_vector(r.direction());

    // Przekształca współrzędną Y kierunku promienia na wartość od 0 do 1.
    // Dzięki temu kolor tła może płynnie zmieniać się od białego do niebieskiego.
    auto t = 0.5 * (unit_direction.y() + 1.0);

    // Miesza dwa kolory: biały i jasnoniebieski, tworząc gradient nieba.
    return Color(1.0, 1.0, 1.0)*(1.0-t) + Color(0.5, 0.7, 1.0)*t;
}

int main() {
    // Określa proporcje obrazu: szerokość do wysokości wynosi 16:9.
    const auto aspect_ratio = 16.0 / 9.0;

    // Ustala szerokość obrazu w pikselach.
    const unsigned int image_width = 800;

    // Oblicza wysokość na podstawie szerokości i proporcji obrazu.
    // static_cast<unsigned int> zamienia wynik na liczbę całkowitą bez znaku.
    const unsigned int image_height = static_cast<unsigned int>(image_width / aspect_ratio);

    // Tworzy okno SFML, w którym zostanie wyświetlony wyrenderowany obraz.
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(image_width, image_height)), "SnowLight RayTracer");
    
    // Tworzy obraz przechowujący kolory wszystkich pikseli.
    sf::Image render_image;
    render_image.resize(sf::Vector2u(image_width, image_height));

    // Tworzy kulę o środku (0, 0, -1) i promieniu 0.5.
    // Jest to obiekt, który będzie widoczny na renderowanym obrazie.
    Sphere world(Point3(0, 0, -1), 0.5);

    // Tworzy kamerę, z której perspektywy generowany jest obraz.
    Camera cam;

    // Określa kierunek padania światła i normalizuje wektor,
    // aby jego długość wynosiła 1.
    Vec3 light_dir = unit_vector(Vec3(1.0, 1.0, -1.0));

    std::cerr << "Rendering..." << std::endl;

    // Przechodzi przez wszystkie wiersze obrazu.
    for (unsigned int j = 0; j < image_height; ++j) {

        // Przechodzi przez wszystkie piksele w bieżącym wierszu.
        for (unsigned int i = 0; i < image_width; ++i) {

            // Zamienia pozycję piksela na współrzędną poziomą u z zakresu 0–1.
            // Dzielenie przez szerokość pomniejszoną o 1 pozwala uwzględnić oba krańce obrazu.
            double u = double(i) / (image_width - 1);

            // Odwrócenie osi Y dla prawidłowego rzutowania kamery.
            // Dzięki temu górna część obrazu odpowiada górnej części widoku kamery.
            double v = double(image_height - 1 - j) / (image_height - 1);

            // Tworzy promień biegnący od kamery przez punkt obrazu określony przez u i v.
            // Dla każdego piksela generowany jest osobny promień.
            Ray r = cam.get_ray(u, v);
            
            // Oblicza kolor piksela na podstawie trafienia promienia w obiekt
            // lub kierunku promienia, jeśli nie trafił on w żaden obiekt.
            Color pixel_color = ray_color(r, world, light_dir);
            
            // Zamienia składową czerwieni z zakresu 0–1 na wartość z zakresu 0–255.
            // static_cast<std::uint8_t> konwertuje wynik na 8-bitową liczbę bez znaku.
            auto ir = static_cast<std::uint8_t>(255.999 * pixel_color.x());

            // Zamienia składową zieleni na wartość wymaganą przez format RGB.
            auto ig = static_cast<std::uint8_t>(255.999 * pixel_color.y());

            // Zamienia składową niebieską na wartość wymaganą przez format RGB.
            auto ib = static_cast<std::uint8_t>(255.999 * pixel_color.z());
            
            // Zapisuje obliczony kolor w odpowiednim pikselu obrazu.
            render_image.setPixel(sf::Vector2u(i, j), sf::Color(ir, ig, ib));
        }
    }

    std::cerr << "Done.\n";

    // Zapisuje wyrenderowany obraz do pliku output.png.
    // Rzutowanie wyniku na void pozwala pominąć warning [[nodiscard]].
    (void)render_image.saveToFile("output.png");

    // Tekstura przechowuje obraz w formie, którą można wykorzystać do rysowania w oknie.
    sf::Texture texture;

    // Wczytuje piksele wyrenderowanego obrazu do tekstury.
    // Rzutowanie na void pomija warning [[nodiscard]].
    (void)texture.loadFromImage(render_image);

    // Tworzy sprite, czyli obiekt wyświetlający teksturę w oknie.
    sf::Sprite sprite(texture);

    // Główna pętla programu — okno pozostaje otwarte, dopóki użytkownik go nie zamknie.
    while (window.isOpen()) {

        // Nowy system obsługi zdarzeń SFML 3 (std::optional).
        // pollEvent() pobiera kolejne zdarzenia, jeśli są dostępne.
        while (const std::optional<sf::Event> event = window.pollEvent()) {

            // Sprawdza, czy użytkownik kliknął przycisk zamknięcia okna.
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Czyści okno przed narysowaniem kolejnej klatki.
        window.clear();

        // Rysuje sprite, czyli wyrenderowany obraz, w oknie.
        window.draw(sprite);

        // Wyświetla narysowaną zawartość okna.
        window.display();
    }

    // Zwraca 0, informując system, że program zakończył działanie poprawnie.
    return 0;
}
