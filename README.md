Oto zaktualizowana, kompletna wersja pliku **`**README**.md`**. Zaktualizowano w niej harmonogram projektu na pełne ukończenie (wszystkie etupy od M1 do M5 jako zrealizowane), odświeżono strukturę katalogów oraz dodano sekcje opisujące zaawansowane funkcje zrealizowane w toku prac: drzewo **BVH**, modele **OBJ**, optykę dielektryczną z prawem Snella, proceduralne tekstury lodu (bump mapping i tłumienie Beer-Lamberta), analityczną wolumetrię oraz obrotowy reflektor Fresnela.

---

# SnowLight ❄️🔦

## Autorski silnik renderujący napisany od podstaw w C++17, oparty na technice śledzenia promieni (Ray Tracing).

Realistyczna scena 3D przedstawiająca latarnię morską zamkniętą w mikrostrukturze trójwymiarowego, dendrytycznego płatka śniegu, osadzona na zamarzniętej tafli jeziora pod nocnym niebem z zorzą polarną i oświetlona obrotowym reflektorem z uwzględnieniem zaawansowanej optyki i wolumetryki.

**Autor** : Mateusz Kozik.

# 📅 Harmonogram i status projektu

### *Etap | Zakres prac | Status*

- **M1** - Kamera i przecięcia [ ✅ Zrealizowano ]
- **M2** - Oświetlenie, materiały i optymalizacja [ ✅ Zrealizowano ]
- **M3** - Modele **OBJ** i drzewo **BVH** [ ✅ Zrealizowano ]
- **M4** - Rekurencja: lód, optyka i dyspersja [ ✅ Zrealizowano ]
- **M5** - Wolumetryka, animacja reflektora i finalny render [ ✅ Zrealizowano ]

### **Szczegóły etapów**

- **M1**: Kamera i przecięcia (Zrealizowano) – Inicjalizacja wektorów, promieni, obiektów polimorficznych, kamery, oprogramowanie **GUI** (**SFML**) i zapis obrazu.
- **M2**: Oświetlenie, materiały i optymalizacja (Zrealizowano) – Obsługa materiałów rozproszonych i metalicznych, oświetlenie bezpośrednie (**NEE**), wielowątkowość OpenMP oraz Progressive Rendering (bufor akumulacji).
- **M3**: Modele **OBJ** i drzewo **BVH** (Zrealizowano) – Wczytywanie własnej geometrii proceduralnej i plików `.**OBJ**` oraz implementacja akceleracji przestrzennej Bounding Volume Hierarchy.
- **M4**: Rekurencja: lód i refrakcja (Zrealizowano) – Prawo Snella, zjawisko całkowitego wewnętrznego odbicia (**TIR**), materiały dielektryczne, prawo tłumienia Beer-Lamberta oraz proceduralny bump mapping.
- **M5**: Wolumetryka i animacja (Zrealizowano) – Analityczne rozpraszanie objętościowe, obrotowy reflektor Fresnela w laternie, interaktywne sterowanie i stabilizacja bufora klatek.

# 🚀 Kluczowe cechy i zrealizowane funkcjonalności

### 1. Fizyka światła i optyka dielektryczna

- **Równanie Fresnela i prawo Snella:** Symulacja przejścia światła przez kryształ lodu ($ior = 1.31$) z uwzględnieniem odbić lustrzanych i załamań.
- **Proceduralne tekstury lodu:** Bump mapping oparty na szumie 3D (`noise3d`) generujący mikrozarysowania oraz prawo Bouguera–Lamberta–Beera modelujące tłumienie i błękitne zabarwienie w objętości bryły.
- **Rozbłyski pryzmatyczne i mikro-chropowatość:** Iskrzenie faset kryształu pod wpływem padającego światła.

### 2. Zaawansowane oświetlenie i wolumetria

- **Obrotowy reflektor Fresnela:** Dwukierunkowa wiązka świetlna z laterny latarni morskiej omiatająca ramiona płatka śniegu i taflę jeziora.
- **Analityczna wolumetria (*God Rays*):** Symulacja rozpraszania światła w powietrzu i wnętrzu kryształu z uwzględnieniem cieni rzucanych przez szprosy laterny.
- **Oświetlenie bezpośrednie (**NEE**):** Efektywne próbkowanie świateł oraz oświetlenie nocnym księżycem i arktycznym niebem z gwiazdami i zorzą.

### 3. Geometria i akceleracja (BVH & OBJ)

- **Proceduralny generator siatek (`MeshGenerator`):** Automatyczne budowanie skomplikowanej geometrii 3D: centralnej komory heksagonalnej, sześciu głównych ramion dendrytycznych z piętrami odnóg oraz modelu latarni morskiej.
- **Drzewo przyspieszające **BVH**:** Drzwi hierarchiczne ograniczające złożoność testów przecięć promieni z $O(N)$ do $O(\log N)$.

### 4. Wydajność i Progressive Rendering

- **Paralelizacja OpenMP:** Wykorzystanie wszystkich rdzeni procesora przy wsparciu bezpiecznych dla wątków generatorów `thread_local std::mt19937`.
- **Interlacing 2x2 i bufor akumulacji:** Dynamiczna optymalizacja trybu interaktywnego podczas ruchu kamery oraz wygładzanie czasowe (**EMA**) dla płynnej animacji obrotu światła.

# 📁 Architektura katalogów

SnowLight/

├── src/

│   ├── core/         # Macierze, wektory, promienie, **AABB** i **BVH**

│   │   ├── Vec3.h

│   │   ├── Ray.h

│   │   ├── **AABB**.h

│   │   ├── Hittable.h

│   │   ├── HittableList.h

│   │   └── **BVH**.h

│   │

│   ├── geometry/    # Bryły, parser **OBJ** i generator siatek

│   │   ├── Sphere.h

│   │   ├── Triangle.h

│   │   ├── ObjLoader.h

│   │   └── MeshGenerator.h

│   │

│   ├── materials/        # Modele materiałów (lód, metal, szkło, światło)

│   │   └── Material.h

│   │

│   ├── render/          # Kamera i bufor akumulacji

│   │   └── Camera.h

│   │

│   └── main.cpp        # Główna pętla renderująca, wolumetria i OpenMP

│

├── CMakeLists.txt    # Konfiguracja systemu budowania

├── **README**.md

└── .gitignore

# ⚙️ Wymagania systemowe i zależności

Projekt wykorzystuje system budowania CMake i jest dostosowany do środowiska **MSYS2** (MinGW64) na systemach Windows.

### Do skompilowania projektu wymagane są:

- Kompilator C++	**GCC** obsługujący standard C++17
- CMake	Minimum 3.10
- Ninja	Zalecany system budowania
- OpenMP	Obsługa wielowątkowości
- **SFML**	Wersja 2.5 lub nowsza

## **Instalacja MSYS2**

Uruchom terminal **MSYS2** MinGW64 i wykonaj polecenie:

```bash pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-sfml

```

# 🔨 Kompilacja i budowanie

Projekt należy kompilować w środowisku **MSYS2** MinGW-w64.

### Utworzenie katalogu budowania

```bash mkdir build && cd build

```

### Konfiguracja projektu za pomocą CMake

```bash cmake -G *Ninja* ..

```

### Kompilacja

```bash ninja

```

Po poprawnej kompilacji plik wykonywalny `SnowLight.exe` znajdzie się w katalogu bieżącym.

# 🖥️ Uruchamianie i instrukcja obsługi

**1. Aby uruchomić silnik:**

```bash ./SnowLight.exe

```

**2. Działanie programu:**

- Automatycznie generuje i zapisuje modele `.obj` do folderu `models/`.
- Inicjalizuje struktury **BVH** i uruchamia okno systemowe **SFML**.
- Renderuje interaktywną scenę w czasie rzeczywistym z obsługą bufora akumulacji.

# 🎮 Sterowanie i nawigacja

### 1. Klawisze ruchu kamery (Orbitalna):

- **W / S** – Zoom / zbliżanie i oddalanie
- **A / D** – Obrót w osi poziomej (azymut)
- **Q / E** – Regulacja elewacji kamery

### 2. Sterowanie oświetleniem:

- **Spacja** – Wstrzymanie / wznowienie automatycznego obrotu reflektora latarni.
- **Strzałki Lewo / Prawo** – Ręczny obrót osi wiązki światła latarni.

### 3. Zamykanie programu:

Aby zamknąć aplikację, kliknij symbol `[X]` w prawym górnym rogu okna.

# 🎯 Podsumowanie

Projekt **SnowLight** stanowi w pełni zrealizowane połączenie fizycznie poprawnej symulacji optycznej (ray tracing, refrakcja, wolumetria) z wydajnym, wielowątkowym silnikiem napisanym od podstaw w C++17.

*Projekt realizowany w ramach przedmiotu na kierunku Informatyczne Systemy Automatyki – Politechnika Wrocławska.*