# SnowLight ❄️🔦

Autorski, pisany od zera w C++17 silnik renderujący oparty na śledzeniu promieni (Ray Tracing). Ostatecznym celem projektu jest wygenerowanie realistycznej sceny 3D przedstawiającej latarnię morską zamkniętą w mikrostrukturze płatka śniegu, uwzględniając złożoną refrakcję lodu i wolumetrykę.

Projekt realizowany w ramach przedmiotu na kierunku Informatyczne Systemy Automatyki (Wroclaw University of Science and Technology).
**Autorzy:** Jakub Włodarski, Mateusz Kozik.

## 📅 Harmonogram i Status Projektu

- **M1: Kamera i przecięcia** – Inicjalizacja wektorów, promieni, obiektów polimorficznych, kamery, oprogramowanie GUI (SFML) i zapis do PNG.

- **M2: Oświetlenie lokalne (Phong)** – Model oświetlenia, cienie twarde (shadow rays), obsługa podstawowych materiałów.

- **M3: Modele OBJ i drzewo BVH** – Wczytywanie własnej geometrii oraz implementacja akceleracji Bounding Volume Hierarchy.

- **M4: Rekurencja: lód i refrakcja** – Prawo Snella, zjawisko całkowitego wewnętrznego odbicia, równania Fresnela.

- **M5: Wolumetryka i render** – Ray Marching (mgła, ośrodki uczestniczące) oraz kompozycja finalnej sceny.

## 🚀 Zrealizowano w Etapie M1 (Obecny Status)

W ramach pierwszego kamienia milowego wdrożono podstawy silnika:

1. Rdzeń matematyczny przestrzeni 3D (`Vec3`, `Ray`) oraz operacje na wektorach (iloczyn skalarny i wektorowy).
2. Abstrakcję geometrii (`Hittable`, `HitRecord`) pozwalającą na polimorficzne liczenie przecięć promieni.
3. System analitycznego rozwiązywania równań przecięć dla sfer (`Sphere`).
4. Kamerę orbitalną (system Look-at), bazującą na matematyce układu sferycznego (`Camera`), umożliwiającą orbitowanie i skalowanie widoku względem celu sceny.
5. Silnik generacji obrazu z cieniowaniem typu diffuse (współczynnik natężenia bazujący na iloczynie skalarnym `dot()`).
6. Integrację biblioteki graficznej SFML: podgląd obrazu na żywo (RenderWindow) oraz obsługę płynnego wieloosiowego wejścia od użytkownika.

## 📁 Architektura katalogów

```text
├── src/
│   ├── core/      # Operacje matematyczne i podstawowe promienie (Vec3.h, Ray.h)
│   ├── geometry/  # Bryły oraz interfejs detekcji kolizji (Hittable.h, Sphere.h)
│   ├── render/    # Wirtualny obserwator, układ "look-at" (Camera.h)
│   └── main.cpp   # Główna pętla renderująca, silnik wejścia i obsługa okna
├── CMakeLists.txt # Plik konfiguracyjny systemu budowania
├── README.md
└── .gitignore
```

## ⚙️ Wymagania systemowe i zależności

Projekt korzysta z systemu budowania **CMake** i jest dostosowany do środowiska **MSYS2 (MinGW64)** na systemach Windows.
Do skompilowania projektu z interfejsem graficznym wymagane są:

- Kompilator wspierający standard **C++17** (np. GCC)
- **CMake** (min. wersja 3.10)
- System budowania **Ninja** (rekomendowany)
- Biblioteka **SFML 2.5+** (do obsługi okna, sterowania i wyświetlania wyrenderowanej ramki)

Instalacja paczek w terminalu MSYS2:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-sfml
```

## 🔨 Kompilacja i Budowanie (MSYS2 MinGW-w64)

Kompilację najwygodniej przeprowadzić z poziomu terminala zintegrowanego w Visual Studio Code lub natywnej konsoli MSYS2.

1. Utwórz i wejdź do katalogu budowania:
   ```bash
   mkdir build
   cd build
   ```

2. Wygeneruj pliki konfiguracyjne za pomocą CMake, wskazując Ninja jako generator:
   ```bash
   cmake -G "Ninja" ..
   ```

3. Skompiluj projekt:
   ```bash
   ninja
   ```

## 🖥️ Uruchamianie i Instrukcja Obsługi

Aby uruchomić silnik, w konsoli (będąc w katalogu `build`) wpisz:
```bash
./SnowLight.exe
```

**Zasada Działania:**
1. Po uruchomieniu, program rozpoczyna renderowanie początkowej klatki i otwiera okno systemowe wyświetlające scenę z domyślnego punktu.
2. Interfejs jest zoptymalizowany: silnik przelicza obraz **tylko** wtedy, kiedy ruszasz kamerą.
3. **Nawigacja Orbitacyjna:**
   * W / S – obrót kamery w osi pionowej (elewacja).
   * A / D – obrót kamery w osi poziomej (azymut, krążenie wokół celu).
   * Q / E – oddalanie (zoom out) i przybliżanie (zoom in) do celu sceny.
4. Aby zamknąć program, po prostu kliknij symbol `[X]` w prawym górnym rogu okna.

## 📝 Historia Zmian (Commity Etapu M1)

Historia rewizji dokumentująca wprowadzanie kluczowych funkcjonalności:

```bash
git commit -m "feat(math): dodanie iloczynu wektorowego do wektorów 3D" -m "Dodanie funkcji cross() w Vec3.h obliczającej iloczyn wektorowy. Funkcja ta jest kluczowa przy wyliczaniu wektorów prostopadłych na potrzeby m.in. budowy bazy ortonormalnej kamery."

git commit -m "feat(camera): implementacja kamery orbitalnej typu look-at" -m "Zastąpienie płaskiej translacji nowym systemem opartym na współrzędnych sferycznych. Dodanie metod orbit() oraz zoom(). Kamera automatycznie przelicza swój lokalny układ współrzędnych (przód/góra/prawo), by stale patrzeć w zdefiniowany punkt sceny, posiadając przy tym bezpiecznik (gimbal lock prevention) dla osi Y."

git commit -m "feat(input): płynne i ciągłe sterowanie kamerą" -m "Przeniesienie odczytu stanów klawiatury poza pętlę zdarzeń (pollEvent) na rzecz sprawdzania stanów klawiszy w czasie rzeczywistym (sf::Keyboard::isKeyPressed). Pozwala to na nieprzerwany, wieloosiowy ruch kamery i reaguje poprawnie na przytrzymanie klawisza. Powiązano logikę z nowymi systemami kamery: zoom (Q,E) oraz orbit (WASD)."

git commit -m "docs: zaktualizowanie projektu o system kamery orbitalnej" -m "Zaktualizowano README.md oraz prezentację LaTeX o informacje na temat zaimplementowania iloczynu wektorowego, kamery orbitalnej, systemu look-at oraz stałego próbkowania wejścia dla płynniejszego działania aplikacji."
```