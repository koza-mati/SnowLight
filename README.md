# SnowLight ❄️🔦

## Autorski silnik renderujący napisany od podstaw w C++17, oparty na technice śledzenia promieni (Ray Tracing).

Ostatecznym celem projektu jest wygenerowanie realistycznej sceny 3D przedstawiającej latarnię morską zamkniętą w mikrostrukturze płatka śniegu, z uwzględnieniem złożonych zjawisk refrakcji światła w lodzie.

**Autorzy** : Mateusz Kozik.

# 📅 Harmonogram i status projektu
### *Etap Zakres prac	Status*
* **M1** -	Kamera i przecięcia [	✅ Zrealizowano ]

* **M2** -	Oświetlenie, materiały i optymalizacja	[ ✅ Zrealizowano ]

* **M3** -	Modele **OBJ** i drzewo **BVH**	[⏳ W planach ]

* **M4** -	Rekurencja: lód i refrakcja	[⏳ W planach ]

* **M5** -	Wolumetryka i finalny render	[⏳ W planach ]

### **Szczegóły etapów**
* **M1**: Kamera i przecięcia (Zrealizowano) – Inicjalizacja wektorów, promieni, obiektów polimorficznych, kamery, oprogramowanie **GUI** (**SFML**) i zapis obrazu do **PNG**.

* **M2**: Oświetlenie, materiały i optymalizacja (Zrealizowano) – Obsługa materiałów rozproszonych i metalicznych, wykorzystanie twierdzenia cosinusów do obliczania kątów padania światła, wielowątkowość OpenMP oraz Progressive Rendering (bufor akumulacji).

* **M3**: Modele **OBJ** i drzewo **BVH** (W planach) – Wczytywanie własnej geometrii oraz implementacja akceleracji Bounding Volume Hierarchy.

* **M4**: Rekurencja: lód i refrakcja (W planach) – Prawo Snella, zjawisko całkowitego wewnętrznego odbicia oraz materiały dielektryczne.

* **M5**: Wolumetryka i render (W planach) – Ostateczne złożenie sceny z latarnią morską i płatkiem śniegu.

# 🚀 Zrealizowane w etapie M2

###  1. Fundament matematyczny i fizyka światła 

Zaimplementowano operacje wektorowe, w tym iloczyn skalarny dot(), uwzględniające relacje kątowe oraz twierdzenie cosinusów wykorzystywane przy wyznaczaniu oświetlenia powierzchni.

### **2. System materiałów**
Wdrożono polimorficzną obsługę materiałów, obejmującą: 
Materiał rozproszony (matowy). Materiał metaliczny (Metal) z kontrolą chropowatości (fuzz). 

 ### **3. Wielowątkowość i wydajność**
Zastosowano paralelizację obliczeń za pomocą OpenMP. 
Wprowadzono również bezpieczne dla wątków generatory liczb losowych: cpp thread_local std::mt19937 \
Pozwala to na niezależne generowanie liczb losowych przez poszczególne wątki.

### **4. Bufor akumulacji (Progressive Rendering)**
Wdrożono mechanizm Progressive Rendering, obejmujący: 
- Dynamiczne skalowanie rozdzielczości. Uśrednianie kolejnych klatek. 
- Bufor akumulacji wyników renderowania. 
- Płynną eksplorację sceny podczas ruchu kamery. 

### **5. Stabilizacja kamery**
Poprawiono logikę sterowania orbitalnego, zabezpieczając system przed:
 - Występowaniem wektora zerowego.
 - Błędami numerycznymi NaN. 
 - Przemieszczaniem kamery pod powierzchnię podłoża. 

# 📁 Architektura katalogów 
 
SnowLight/ \
├── src/ \
│   ├── core/    &emsp;&emsp;   # Operacje matematyczne i generatory losowe\
│   │   ├── Vec3.h \
│   │   └── Ray.h \
│   │\
│   ├── geometry/  &emsp;&emsp;# Bryły i interfejs detekcji kolizji \
│   │   ├── Hittable.h \
│   │   └── Sphere.h \
│   │ \
│   ├── material/    &emsp;&emsp;  # Definicje materiałów i odbić światła \
│   │   ├── Material.h \
│   │   ├── Lambertian.h\
│   │   └── Metal.h\
│   │ \
│   ├── render/     &emsp;&emsp;   # Kamera i bufor akumulacji \
│   │   └── Camera.h \
│   │ \
│   └── main.cpp  &emsp;&emsp;    # Główna pętla renderująca, \
│                 &emsp;&emsp; &emsp;&emsp; &emsp;&emsp; &emsp;&emsp;    # OpenMP i obsługa okna \
│ \
├── CMakeLists.txt  &emsp; # Konfiguracja systemu budowania\
├── README.md \
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
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-sfml
```


# 🔨 Kompilacja i budowanie

Projekt należy kompilować w środowisku **MSYS2** MinGW-w64.

### Utworzenie katalogu budowania

```bash 
mkdir build cd build
```
### Konfiguracja projektu za pomocą CMake

```bash
 cmake -G *Ninja* ..
```
### Kompilacja

```bash
 ninja
 ```

Po poprawnej kompilacji plik wykonywalny powinien znajdować się w katalogu **./build**.

# 🖥️ Uruchamianie i instrukcja obsługi

**1. Aby uruchomić silnik, będąc w katalogu build, wpisz:**
```bash 
./SnowLight.exe
```

**2. Po uruchomieniu program:**
 1. Inicjalizuje silnik renderujący.
 1. Rozpoczyna renderowanie początkowej klatki. 
 1. Otwiera okno systemowe wyświetlające scenę z domyślnego punktu obserwacji.
 1. Silnik wykorzystuje bufor akumulacji oraz dynamicznie przelicza obraz podczas ruchu kamery i w tle, zapewniając płynniejszą eksplorację sceny.

# 🎮 Nawigacja orbitalna

### 1. Sterowanie kamerą odbywa się za pomocą klawiatury:
- **W / S**	- Obrót kamery w osi pionowej / zoom
- **A / D**	- Obrót kamery w osi poziomej (azymut)
- **Q / E** -	Regulacja elewacji i odległości

Sterowanie zostało zabezpieczone przed występowaniem wektora zerowego oraz przemieszczaniem kamery pod powierzchnię gruntu.

###  2. Zamykanie programu

Aby zamknąć aplikację, kliknij symbol [X] w prawym górnym rogu okna.

# 🎯 Cel końcowy projektu

Docelowym rezultatem prac nad SnowLight jest stworzenie realistycznego renderu 3D przedstawiającego latarnię morską zamkniętą wewnątrz struktury płatka śniegu.

**Projekt zakłada wykorzystanie zaawansowanych technik renderowania, takich jak:**

- Ray Tracing. 
- Rekurencyjne śledzenie promieni. 
- Refrakcja światła zgodna z prawem Snella. 
- Całkowite wewnętrzne odbicie. 
- Materiały dielektryczne. 
- Akceleracja obliczeń za pomocą **BVH**. 
- Renderowanie wolumetryczne.

SnowLight ma na celu połączenie fizycznie poprawnej symulacji światła z wydajnym, autorskim silnikiem renderującym napisanym w C++17.
 
 #
 *Projekt realizowany w ramach przedmiotu na kierunku Informatyczne Systemy Automatyki – Politechnika Wrocławska.*
