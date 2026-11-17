#pragma once
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include "Hittable.h"
#include "HittableList.h"
#include "AABB.h"

class BVHNode : public Hittable {
public:
    BVHNode() {}

    BVHNode(const HittableList& list)
        : BVHNode(list.objects, 0, list.objects.size()) {}

    BVHNode(const std::vector<std::shared_ptr<Hittable>>& src_objects, size_t start, size_t end) {
        // Kopiujemy zakres obiektów, aby móc je posortować bez modyfikacji oryginalnej listy
        auto objects = src_objects;

        // Losowy wybór osi podziału (0 = X, 1 = Y, 2 = Z)
        int axis = std::min(2, static_cast<int>(random_double() * 3.0));
        auto comparator = (axis == 0) ? box_x_compare
                        : (axis == 1) ? box_y_compare
                                      : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1) {
            left = right = objects[start];
        } else if (object_span == 2) {
            if (comparator(objects[start], objects[start + 1])) {
                left = objects[start];
                right = objects[start + 1];
            } else {
                left = objects[start + 1];
                right = objects[start];
            }
        } else {
            std::sort(objects.begin() + start, objects.begin() + end, comparator);

            auto mid = start + object_span / 2;
            left = std::make_shared<BVHNode>(objects, start, mid);
            right = std::make_shared<BVHNode>(objects, mid, end);
        }

        AABB box_left, box_right;
        if (!left->bounding_box(box_left) || !right->bounding_box(box_right)) {
            std::cerr << "Blad BVH: Obiekt nie posiada bryly brzegowej AABB.\n";
        }

        box = surrounding_box(box_left, box_right);
    }

    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override {
        // 1. Szybkie odrzucenie: jeśli promień nie trafia w pudełko węzła, ignorujemy wszystkie dzieci
        if (!box.hit(r, t_min, t_max))
            return false;

        // 2. Rekurencyjne zejście w głąb drzewa
        bool hit_left = left->hit(r, t_min, t_max, rec);
        bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

        return hit_left || hit_right;
    }

    virtual bool bounding_box(AABB& output_box) const override {
        output_box = box;
        return true;
    }

private:
    std::shared_ptr<Hittable> left;
    std::shared_ptr<Hittable> right;
    AABB box;

    static bool box_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b, int axis) {
        AABB box_a, box_b;
        if (!a->bounding_box(box_a) || !b->bounding_box(box_b)) {
            std::cerr << "Blad BVH: Brak pudelek brzegowych podczas sortowania.\n";
        }
        return box_a.min().e[axis] < box_b.min().e[axis];
    }

    static bool box_x_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) {
        return box_compare(a, b, 2);
    }
};