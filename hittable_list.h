#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "kdtree.h"

#include <memory>
#include <vector>
#include <stdio.h>

using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable {
    public:
        hittable_list() {}
        hittable_list(shared_ptr<hittable> object) { add(object); }

        void clear() { objects.clear(); }
        void add(shared_ptr<hittable> object) { objects.push_back(object); }
        void build() { 
            objtree.build_tree(objects); 
            fprintf(stderr, "kD-Tree is built!\n");
        }

        virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override;
        virtual pair<double, double> bound(int dim) const override { return pair<double, double>(-1, -1); }

    public:
        std::vector<shared_ptr<hittable>> objects;
        kdtree objtree;
};

bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    // return objtree.hit(r, t_min, t_max, rec);

    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;

    for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

#endif