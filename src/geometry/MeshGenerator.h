#pragma once
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

class MeshGenerator {
public:
    static void ensure_scene_models_exist(const std::string& dir = "models") {
        std::filesystem::create_directories(dir);
        generate_lighthouse_white(dir + "/lighthouse_white.obj");
        generate_lighthouse_red(dir + "/lighthouse_red.obj");
        generate_lighthouse_metal(dir + "/lighthouse_metal.obj");
        generate_lighthouse_glass(dir + "/lighthouse_glass.obj");
        generate_lighthouse_snow_base(dir + "/lighthouse_snow_base.obj");
        generate_snowflake_crystal(dir + "/snowflake_crystal.obj");
    }

private:
    static constexpr double PI = 3.14159265358979323846;
    static constexpr const char* MESH_VERSION_TAG = "# SnowLight Mesh v8";

    struct ObjBuilder {
        std::vector<std::array<double, 3>> verts;
        std::vector<std::array<int, 3>> tris;

        int add_v(double x, double y, double z) {
            verts.push_back({x, y, z});
            return static_cast<int>(verts.size());
        }

        void add_tri(int a, int b, int c) {
            tris.push_back({a, b, c});
        }

        void add_quad(int a, int b, int c, int d) {
            add_tri(a, b, c);
            add_tri(a, c, d);
        }

        void add_box(double x0, double y0, double z0, double x1, double y1, double z1) {
            int v000 = add_v(x0, y0, z0);
            int v100 = add_v(x1, y0, z0);
            int v110 = add_v(x1, y1, z0);
            int v010 = add_v(x0, y1, z0);
            int v001 = add_v(x0, y0, z1);
            int v101 = add_v(x1, y0, z1);
            int v111 = add_v(x1, y1, z1);
            int v011 = add_v(x0, y1, z1);

            add_quad(v001, v101, v111, v011);
            add_quad(v100, v000, v010, v110);
            add_quad(v101, v100, v110, v111);
            add_quad(v000, v001, v011, v010);
            add_quad(v011, v111, v110, v010);
            add_quad(v000, v100, v101, v001);
        }

        void add_cylinder_segment(
            double x0, double y0, double z0, double r0,
            double x1, double y1, double z1, double r1,
            int sides = 10, bool cap_bottom = true, bool cap_top = true
        ) {
            std::vector<int> bot_ring(sides), top_ring(sides);
            for (int s = 0; s < sides; ++s) {
                double theta = 2.0 * PI * s / sides;
                double dx = std::sin(theta);
                double dz = std::cos(theta);
                bot_ring[s] = add_v(x0 + r0 * dx, y0, z0 + r0 * dz);
                top_ring[s] = add_v(x1 + r1 * dx, y1, z1 + r1 * dz);
            }
            for (int s = 0; s < sides; ++s) {
                int sn = (s + 1) % sides;
                add_quad(bot_ring[s], bot_ring[sn], top_ring[sn], top_ring[s]);
            }
            if (cap_bottom) {
                int cb = add_v(x0, y0, z0);
                for (int s = 0; s < sides; ++s) {
                    int sn = (s + 1) % sides;
                    add_tri(cb, bot_ring[sn], bot_ring[s]);
                }
            }
            if (cap_top) {
                int ct = add_v(x1, y1, z1);
                for (int s = 0; s < sides; ++s) {
                    int sn = (s + 1) % sides;
                    add_tri(ct, top_ring[s], top_ring[sn]);
                }
            }
        }

        void add_torus_ring(double y_center, double major_r, double tube_r, int major_seg = 48, int minor_seg = 8) {
            std::vector<std::vector<int>> rings(major_seg, std::vector<int>(minor_seg));
            for (int i = 0; i < major_seg; ++i) {
                double phi = 2.0 * PI * i / major_seg;
                double sp = std::sin(phi), cp = std::cos(phi);
                for (int j = 0; j < minor_seg; ++j) {
                    double psi = 2.0 * PI * j / minor_seg;
                    double r = major_r + tube_r * std::cos(psi);
                    double y = y_center + tube_r * std::sin(psi);
                    rings[i][j] = add_v(r * sp, y, r * cp);
                }
            }
            for (int i = 0; i < major_seg; ++i) {
                int in = (i + 1) % major_seg;
                for (int j = 0; j < minor_seg; ++j) {
                    int jn = (j + 1) % minor_seg;
                    add_quad(rings[i][j], rings[in][j], rings[in][jn], rings[i][jn]);
                }
            }
        }

        void add_lathe(const std::vector<std::pair<double, double>>& profile, int segments = 48, bool cap_bot = true, bool cap_top = true) {
            int n = static_cast<int>(profile.size());
            std::vector<std::vector<int>> rings(n, std::vector<int>(segments));
            for (int r = 0; r < n; ++r) {
                for (int s = 0; s < segments; ++s) {
                    double theta = 2.0 * PI * s / segments;
                    rings[r][s] = add_v(profile[r].second * std::sin(theta), profile[r].first, profile[r].second * std::cos(theta));
                }
            }
            for (int r = 0; r < n - 1; ++r) {
                for (int s = 0; s < segments; ++s) {
                    int sn = (s + 1) % segments;
                    add_quad(rings[r][s], rings[r][sn], rings[r + 1][sn], rings[r + 1][s]);
                }
            }
            if (cap_bot) {
                int cb = add_v(0.0, profile.front().first, 0.0);
                for (int s = 0; s < segments; ++s) {
                    int sn = (s + 1) % segments;
                    add_tri(cb, rings[0][sn], rings[0][s]);
                }
            }
            if (cap_top) {
                int ct = add_v(0.0, profile.back().first, 0.0);
                for (int s = 0; s < segments; ++s) {
                    int sn = (s + 1) % segments;
                    add_tri(ct, rings.back()[s], rings.back()[sn]);
                }
            }
        }

        // Pełny, dwustronnie fazowany pryzmat heksagonalny (dla płytek na ramionach płatka)
        void add_upright_hex_prism(
            double cx, double cy,
            double r_outer, double r_inner,
            double z_edge, double z_center,
            double angle_offset = 0.0
        ) {
            const int sides = 6;
            std::vector<int> fi(sides), fo(sides), bi(sides), bo(sides);

            for (int k = 0; k < sides; ++k) {
                double theta = k * (PI / 3.0) + angle_offset;
                double cs = std::cos(theta), sn = std::sin(theta);
                fi[k] = add_v(cx + r_inner * cs, cy + r_inner * sn,  z_center);
                fo[k] = add_v(cx + r_outer * cs, cy + r_outer * sn,  z_edge);
                bi[k] = add_v(cx + r_inner * cs, cy + r_inner * sn, -z_center);
                bo[k] = add_v(cx + r_outer * cs, cy + r_outer * sn, -z_edge);
            }

            int fc = add_v(cx, cy,  z_center);
            int bc = add_v(cx, cy, -z_center);

            for (int k = 0; k < sides; ++k) {
                int kn = (k + 1) % sides;
                add_tri(fc, fi[k], fi[kn]);
                add_quad(fi[k], fo[k], fo[kn], fi[kn]);
                add_quad(fo[k], bo[k], bo[kn], fo[kn]);
                add_quad(bo[k], bi[k], bi[kn], bo[kn]);
                add_tri(bc, bi[kn], bi[k]);
            }
        }

        // Cienkościenna, fazowana komora heksagonalna kryształu zamykająca latarnię we wnętrzu płatka
        void add_hollow_hex_crystal_chamber(
            double r_out, double r_out_bevel, double z_out_edge, double z_out_face,
            double r_in,  double r_in_bevel,  double z_in_edge,  double z_in_face,
            double angle_offset = 0.0
        ) {
            const int sides = 6;
            std::vector<int> ofi(sides), ofo(sides), obi(sides), obo(sides);
            std::vector<int> ifi(sides), ifo(sides), ibi(sides), ibo(sides);

            for (int k = 0; k < sides; ++k) {
                double theta = k * (PI / 3.0) + angle_offset;
                double cs = std::cos(theta), sn = std::sin(theta);

                // Powłoka zewnętrzna
                ofi[k] = add_v(r_out_bevel * cs, r_out_bevel * sn,  z_out_face);
                ofo[k] = add_v(r_out       * cs, r_out       * sn,  z_out_edge);
                obi[k] = add_v(r_out_bevel * cs, r_out_bevel * sn, -z_out_face);
                obo[k] = add_v(r_out       * cs, r_out       * sn, -z_out_edge);

                // Powłoka wewnętrzna (pustka kryształu wokół latarni)
                ifi[k] = add_v(r_in_bevel * cs, r_in_bevel * sn,  z_in_face);
                ifo[k] = add_v(r_in       * cs, r_in       * sn,  z_in_edge);
                ibi[k] = add_v(r_in_bevel * cs, r_in_bevel * sn, -z_in_face);
                ibo[k] = add_v(r_in       * cs, r_in       * sn, -z_in_edge);
            }

            int ofc = add_v(0.0, 0.0,  z_out_face + 0.07);
            int obc = add_v(0.0, 0.0, -z_out_face - 0.07);
            int ifc = add_v(0.0, 0.0,  z_in_face);
            int ibc = add_v(0.0, 0.0, -z_in_face);

            for (int k = 0; k < sides; ++k) {
                int kn = (k + 1) % sides;

                // Ścianki zewnętrzne (normalne skierowane na zewnątrz bryły lodu)
                add_tri(ofc, ofi[k], ofi[kn]);
                add_quad(ofi[k], ofo[k], ofo[kn], ofi[kn]);
                add_quad(ofo[k], obo[k], obo[kn], ofo[kn]);
                add_quad(obo[k], obi[k], obi[kn], obo[kn]);
                add_tri(obc, obi[kn], obi[k]);

                // Ścianki wewnętrzne (odwrócona kolejność -> normalne skierowane do środka komory)
                add_tri(ifc, ifi[kn], ifi[k]);
                add_quad(ifi[kn], ifo[kn], ifo[k], ifi[k]);
                add_quad(ifo[kn], ibo[kn], ibo[k], ifo[k]);
                add_quad(ibo[kn], ibi[kn], ibi[k], ibo[k]);
                add_tri(ibc, ibi[k], ibi[kn]);
            }
        }

        // Pionowe ramię kryształu 3D z obustronną granią
        void add_upright_crystal_limb(
            double start_x, double start_y,
            double dir_x, double dir_y,
            double length,
            double half_width,
            double z_edge,
            double z_ridge
        ) {
            double perp_x = -dir_y;
            double perp_y =  dir_x;

            auto pt = [&](double along, double across, double z) {
                return add_v(
                    start_x + dir_x * along + perp_x * across,
                    start_y + dir_y * along + perp_y * across,
                    z
                );
            };

            double l_mid = length * 0.78;
            double w_mid = half_width * 1.15;

            int fC0 = pt(0.0,  0.0,         z_ridge);
            int fL0 = pt(0.0,  half_width,  z_edge);
            int fR0 = pt(0.0, -half_width,  z_edge);
            int bC0 = pt(0.0,  0.0,        -z_ridge);
            int bL0 = pt(0.0,  half_width, -z_edge);
            int bR0 = pt(0.0, -half_width, -z_edge);

            int fC1 = pt(l_mid,  0.0,     z_ridge * 0.92);
            int fL1 = pt(l_mid,  w_mid,   z_edge * 0.92);
            int fR1 = pt(l_mid, -w_mid,   z_edge * 0.92);
            int bC1 = pt(l_mid,  0.0,    -z_ridge * 0.92);
            int bL1 = pt(l_mid,  w_mid,  -z_edge * 0.92);
            int bR1 = pt(l_mid, -w_mid,  -z_edge * 0.92);

            int tipF = pt(length, 0.0,  z_edge * 0.55);
            int tipB = pt(length, 0.0, -z_edge * 0.55);

            add_quad(bL0, bC0, fC0, fL0);
            add_quad(bC0, bR0, fR0, fC0);

            add_quad(fC0, fC1, fL1, fL0);
            add_quad(fR0, fR1, fC1, fC0);
            add_quad(bL0, bL1, bC1, bC0);
            add_quad(bC0, bC1, bR1, bR0);
            add_quad(fL0, fL1, bL1, bL0);
            add_quad(bR0, bR1, fR1, fR0);

            add_tri(fC1, tipF, fL1);
            add_tri(fR1, tipF, fC1);
            add_tri(bL1, tipB, bC1);
            add_tri(bC1, tipB, bR1);
            add_quad(fL1, tipF, tipB, bL1);
            add_quad(bR1, tipB, tipF, fR1);
        }

        void save_to_file(const std::string& filepath) const {
            std::ofstream out(filepath);
            if (!out.is_open()) return;
            out << MESH_VERSION_TAG << "\n";
            for (const auto& v : verts) {
                out << "v " << v[0] << " " << v[1] << " " << v[2] << "\n";
            }
            for (const auto& f : tris) {
                out << "f " << f[0] << " " << f[1] << " " << f[2] << "\n";
            }
        }
    };

    static bool is_up_to_date(const std::string& filepath) {
        std::ifstream in(filepath);
        if (!in.is_open()) return false;
        std::string first_line;
        std::getline(in, first_line);
        return first_line.find(MESH_VERSION_TAG) != std::string::npos;
    }

    static void generate_lighthouse_white(const std::string& filepath) {
        if (is_up_to_date(filepath)) return;
        ObjBuilder b;

        b.add_lathe({
            {0.00, 0.38},
            {0.06, 0.38},
            {0.08, 0.34},
            {0.26, 0.32}
        }, 48);

        b.add_lathe({
            {0.58, 0.285},
            {0.91, 0.250}
        }, 48);

        b.add_lathe({
            {1.24, 0.215},
            {1.54, 0.185},
            {1.61, 0.275},
            {1.67, 0.275},
            {1.68, 0.180}
        }, 48);

        b.save_to_file(filepath);
    }

    static void generate_lighthouse_red(const std::string& filepath) {
        if (is_up_to_date(filepath)) return;
        ObjBuilder b;

        b.add_lathe({
            {0.26, 0.320},
            {0.58, 0.285}
        }, 48);

        b.add_lathe({
            {0.91, 0.250},
            {1.24, 0.215}
        }, 48);

        b.add_lathe({
            {1.98, 0.175},
            {1.99, 0.210},
            {2.02, 0.210},
            {2.14, 0.115},
            {2.23, 0.028}
        }, 48);

        b.save_to_file(filepath);
    }

    static void generate_lighthouse_metal(const std::string& filepath) {
        if (is_up_to_date(filepath)) return;
        ObjBuilder b;

        const int num_posts = 16;
        const double rail_r = 0.262;
        for (int i = 0; i < num_posts; ++i) {
            double theta = 2.0 * PI * i / num_posts;
            double px = rail_r * std::sin(theta);
            double pz = rail_r * std::cos(theta);
            b.add_cylinder_segment(px, 1.67, pz, 0.007, px, 1.81, pz, 0.007, 8, false, true);
        }
        b.add_torus_ring(1.74, rail_r, 0.006, 48, 8);
        b.add_torus_ring(1.81, rail_r, 0.008, 48, 8);

        const int num_mullions = 8;
        const double mullion_r = 0.172;
        for (int i = 0; i < num_mullions; ++i) {
            double theta = 2.0 * PI * i / num_mullions;
            double mx = mullion_r * std::sin(theta);
            double mz = mullion_r * std::cos(theta);
            b.add_cylinder_segment(mx, 1.68, mz, 0.008, mx, 1.98, mz, 0.008, 8, false, false);
        }
        b.add_torus_ring(1.685, mullion_r, 0.010, 48, 8);
        b.add_torus_ring(1.975, mullion_r, 0.010, 48, 8);

        b.add_box(-0.075, 0.06, 0.30,  0.075, 0.34, 0.35);
        b.add_box(-0.095, 0.34, 0.30,  0.095, 0.37, 0.36);

        b.add_box(-0.045, 0.70, 0.24,  0.045, 0.84, 0.285);
        b.add_box(-0.040, 1.12, 0.20,  0.040, 1.25, 0.245);

        b.add_lathe({
            {2.23, 0.022},
            {2.25, 0.036},
            {2.28, 0.036},
            {2.30, 0.014},
            {2.42, 0.003}
        }, 24);

        b.save_to_file(filepath);
    }

    static void generate_lighthouse_glass(const std::string& filepath) {
        if (is_up_to_date(filepath)) return;
        ObjBuilder b;

        const int segs = 48;
        const double y0 = 1.685, y1 = 1.975;
        const double r_out = 0.168, r_in = 0.156;

        std::vector<int> ob(segs), ot(segs), ib(segs), it(segs);
        for (int s = 0; s < segs; ++s) {
            double theta = 2.0 * PI * s / segs;
            double sn = std::sin(theta), cs = std::cos(theta);
            ob[s] = b.add_v(r_out * sn, y0, r_out * cs);
            ot[s] = b.add_v(r_out * sn, y1, r_out * cs);
            ib[s] = b.add_v(r_in  * sn, y0, r_in  * cs);
            it[s] = b.add_v(r_in  * sn, y1, r_in  * cs);
        }

        for (int s = 0; s < segs; ++s) {
            int sn = (s + 1) % segs;
            b.add_quad(ob[s], ob[sn], ot[sn], ot[s]);
            b.add_quad(ib[sn], ib[s], it[s], it[sn]);
            b.add_quad(ot[s], ot[sn], it[sn], it[s]);
            b.add_quad(ib[s], ib[sn], ob[sn], ob[s]);
        }

        b.save_to_file(filepath);
    }

    // Miniaturowa zaspa śnieżna wewnątrz dolnej części kryształu, na której stoi latarnia
    static void generate_lighthouse_snow_base(const std::string& filepath) {
        if (is_up_to_date(filepath)) return;
        ObjBuilder b;

        b.add_lathe({
            {-0.92, 0.03},
            {-0.85, 0.16},
            {-0.77, 0.28},
            {-0.74, 0.26}
        }, 36);

        b.save_to_file(filepath);
    }

// Płatek śniegu 3D z centralną, cienkościenną komorą heksagonalną zamykającą latarnię wewnątrz
    static void generate_snowflake_crystal(const std::string& filepath) {
        if (is_up_to_date(filepath)) return;
        ObjBuilder b;

        // 1. Centralna komora kryształu z fazowanymi krawędziami pryzmatycznymi
        b.add_hollow_hex_crystal_chamber(
            1.10, 0.66, 0.14, 0.48,
            1.00, 0.60, 0.10, 0.42,
            PI / 2.0
        );

        // 2. Sześć ramion dendrytycznych wyrastających z wierzchołków komory kryształu
        const double cos60 = 0.5;
        const double sin60 = 0.8660254037844386;

        for (int arm = 0; arm < 6; ++arm) {
            double phi = arm * (PI / 3.0) + (PI / 2.0);
            double dx = std::cos(phi);
            double dy = std::sin(phi);
            double px = -dy;
            double py =  dx;

            // Główny trzpień ramienia kryształu (od R = 1.02 do R = 2.24)
            b.add_upright_crystal_limb(
                dx * 1.02, dy * 1.02,
                dx, dy,
                1.22,
                0.072,
                0.032, 0.088
            );

            // Heksagonalna płytka kryształu na końcu każdego głównego ramienia
            b.add_upright_hex_prism(dx * 2.18, dy * 2.18, 0.145, 0.075, 0.024, 0.062, phi);

            auto add_branch_pair = [&](double dist_along, double branch_len, double branch_w, double ze, double zr) {
                double bx = dx * dist_along;
                double by = dy * dist_along;

                double r_dx = dx * cos60 + px * sin60;
                double r_dy = dy * cos60 + py * sin60;
                b.add_upright_crystal_limb(bx, by, r_dx, r_dy, branch_len, branch_w, ze, zr);

                double l_dx = dx * cos60 - px * sin60;
                double l_dy = dy * cos60 - py * sin60;
                b.add_upright_crystal_limb(bx, by, l_dx, l_dy, branch_len, branch_w, ze, zr);
            };

            // Trzy piętra symetrycznych odnóg bocznych pod kątem +-60°
            add_branch_pair(1.24, 0.56, 0.048, 0.026, 0.070);
            add_branch_pair(1.56, 0.42, 0.040, 0.022, 0.058);
            add_branch_pair(1.86, 0.28, 0.032, 0.018, 0.046);
        }

        // 3. Obrót 3D kryształu (eksponuje grubość ścianek i boczne fasety ramion bez kolizji z latarnią)
        const double rx = -6.0 * PI / 180.0;
        const double ry = 15.0 * PI / 180.0;
        const double cx = std::cos(rx), sx = std::sin(rx);
        const double cy = std::cos(ry), sy = std::sin(ry);

        for (auto& v : b.verts) {
            double x = v[0], y = v[1], z = v[2];
            double y1 = y * cx - z * sx;
            double z1 = y * sx + z * cx;
            v[0] =  x * cy + z1 * sy;
            v[1] =  y1;
            v[2] = -x * sy + z1 * cy;
        }

        b.save_to_file(filepath);
    }
};