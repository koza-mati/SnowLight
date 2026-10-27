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