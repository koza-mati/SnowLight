#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../core/BVH.h"
#include "../core/HittableList.h"
#include "../core/Vec3.h"
#include "../materials/Material.h"
#include "Triangle.h"

class ObjLoader {
public:
    // Wczytuje plik .obj z bezpośrednią skalą i przesunięciem
    static std::shared_ptr<Hittable> load_obj(
        const std::string& filepath,
        std::shared_ptr<Material> material,
        double scale = 1.0,
        const Vec3& offset = Vec3(0, 0, 0)
    ) {
        std::vector<Point3> vertices;
        std::vector<std::array<int, 3>> faces;
        if (!parse_obj_data(filepath, vertices, faces)) return nullptr;

        HittableList triangles;
        for (const auto& f : faces) {
            Point3 v0 = vertices[f[0]] * scale + offset;
            Point3 v1 = vertices[f[1]] * scale + offset;
            Point3 v2 = vertices[f[2]] * scale + offset;
            triangles.add(std::make_shared<Triangle>(v0, v1, v2, material));
        }

        std::cout << "[ObjLoader] Wczytano '" << filepath << "': "
                  << vertices.size() << " wierzcholkow, "
                  << triangles.objects.size() << " trojkatow.\n";

        return triangles.objects.empty() ? nullptr : std::make_shared<BVHNode>(triangles);
    }

    // Wczytuje dowolny plik .obj, centruje jego podstawę w (0,0,0), skaluje do target_height
    // i umieszcza w punkcie target_base_pos (idealne dla modeli z Blendera)
    static std::shared_ptr<Hittable> load_obj_fit(
        const std::string& filepath,
        std::shared_ptr<Material> material,
        double target_height,
        const Vec3& target_base_pos = Vec3(0, 0, 0)
    ) {
        std::vector<Point3> vertices;
        std::vector<std::array<int, 3>> faces;
        if (!parse_obj_data(filepath, vertices, faces)) return nullptr;

        double min_x = std::numeric_limits<double>::max();
        double min_y = std::numeric_limits<double>::max();
        double min_z = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double max_y = std::numeric_limits<double>::lowest();
        double max_z = std::numeric_limits<double>::lowest();

        for (const auto& v : vertices) {
            min_x = std::min(min_x, v.x()); max_x = std::max(max_x, v.x());
            min_y = std::min(min_y, v.y()); max_y = std::max(max_y, v.y());
            min_z = std::min(min_z, v.z()); max_z = std::max(max_z, v.z());
        }

        double height = max_y - min_y;
        double scale = (height > 1e-6) ? (target_height / height) : 1.0;
        double center_x = 0.5 * (min_x + max_x);
        double center_z = 0.5 * (min_z + max_z);

        HittableList triangles;
        for (const auto& f : faces) {
            auto transform = [&](const Point3& p) {
                return Point3(
                    (p.x() - center_x) * scale + target_base_pos.x(),
                    (p.y() - min_y)    * scale + target_base_pos.y(),
                    (p.z() - center_z) * scale + target_base_pos.z()
                );
            };
            triangles.add(std::make_shared<Triangle>(
                transform(vertices[f[0]]),
                transform(vertices[f[1]]),
                transform(vertices[f[2]]),
                material
            ));
        }

        std::cout << "[ObjLoader] Dopasowano '" << filepath << "': "
                  << triangles.objects.size() << " trojkatow (skala: " << scale << ").\n";

        return triangles.objects.empty() ? nullptr : std::make_shared<BVHNode>(triangles);
    }

private:
    static bool parse_obj_data(
        const std::string& filepath,
        std::vector<Point3>& vertices,
        std::vector<std::array<int, 3>>& faces
    ) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[ObjLoader] BLAD: Nie mozna otworzyc pliku: " << filepath << "\n";
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                double x, y, z;
                if (iss >> x >> y >> z) {
                    vertices.emplace_back(x, y, z);
                }
            } else if (prefix == "f") {
                std::vector<int> face_indices;
                std::string token;

                while (iss >> token) {
                    size_t slash_pos = token.find('/');
                    std::string idx_str = (slash_pos == std::string::npos) ? token : token.substr(0, slash_pos);

                    if (!idx_str.empty()) {
                        int raw_idx = std::stoi(idx_str);
                        int idx = (raw_idx < 0)
                            ? static_cast<int>(vertices.size()) + raw_idx
                            : raw_idx - 1;
                        face_indices.push_back(idx);
                    }
                }

                for (size_t i = 1; i + 1 < face_indices.size(); ++i) {
                    int i0 = face_indices[0];
                    int i1 = face_indices[i];
                    int i2 = face_indices[i + 1];

                    if (i0 >= 0 && i0 < (int)vertices.size() &&
                        i1 >= 0 && i1 < (int)vertices.size() &&
                        i2 >= 0 && i2 < (int)vertices.size()) {
                        faces.push_back({i0, i1, i2});
                    }
                }
            }
        }
        return !faces.empty();
    }
};