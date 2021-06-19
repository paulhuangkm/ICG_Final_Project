#ifndef KDTREE_H
#define KDTREE_H

#include "vec3.h"
#include "hittable.h"

#include <memory>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stack>
#include <iostream>

using std::shared_ptr;
using std::vector;
using std::pair;
using std::min;
using std::max;
using std::stack;

class kdnode{
    public:
        int axis;   // split axis
        double pos; // split position
        bool leaf = false;
        vector<shared_ptr<hittable>> objects;
        kdnode *left = nullptr, *right = nullptr;
};

class box{
    public:
        vector<double> hit(const ray &r, double t_min, double t_max);
        bool on(point3 x);
    public:
        point3 lower;
        point3 upper;
};

class kdtree{
    public:
        kdtree() = default;
        kdtree(vector<shared_ptr<hittable>> &objects) { build_tree(objects); };
        void build_tree(vector<shared_ptr<hittable>> &objects);
        bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const;
        
    private:
        kdnode *__build_tree(vector<shared_ptr<hittable>> &objects, int depth, box bbox);

    public:
        kdnode *root = nullptr;
        box bbox_r;   // bounding box (root)
};

void kdtree::build_tree(vector<shared_ptr<hittable>> &objects){
    bbox_r.lower = point3(objects[0]->bound(0).first, objects[0]->bound(1).first, objects[0]->bound(2).first);
    bbox_r.upper = point3(objects[0]->bound(0).second, objects[0]->bound(1).second, objects[0]->bound(2).second);
    for (auto &object : objects){
        for (int i = 0; i != 3; ++i){
            bbox_r.lower[i] = min(bbox_r.lower[i], object->bound(i).first);
            bbox_r.upper[i] = max(bbox_r.upper[i], object->bound(i).second);
        }
    }
    root = __build_tree(objects, 0, bbox_r);
}

kdnode *kdtree::__build_tree(vector<shared_ptr<hittable>> &objects, int depth, box bbox){
    kdnode *cur = new kdnode;
    if (objects.size() <= 1){
        cur->leaf = true;
        cur->objects = objects;
        return cur;
    }
    for (int i = 0; i != 3; ++i){
        cur->axis = depth % 3;
        vector<pair<double, int>> candidates;
        for (int i = 0; i != objects.size(); ++i){
            auto bound = objects[i]->bound(cur->axis);
            candidates.emplace_back(bound.first, i);
            candidates.emplace_back(bound.second, i);
        }
        double width = bbox.upper[cur->axis] - bbox.lower[cur->axis];
        for (double i = 0.01; i <= 0.99; i += 0.01)
            candidates.emplace_back(bbox.lower[cur->axis] + width * i, -1);
        sort(candidates.begin(), candidates.end());
        
        vector<bool> visit(objects.size(), false);
        int lnum = 0, rnum = objects.size();
        double lb = bbox.lower[cur->axis], rb = bbox.upper[cur->axis];
        pair<double, double> min_t(infinity, 0.0);
        for (int i = 0; i != candidates.size(); ++i){
            auto &cand = candidates[i];
            if (cand.second != -1 && visit[cand.second])
                --rnum;
            if (lb + epsilon < cand.first && cand.first < rb - epsilon)
                min_t = min(min_t, {(cand.first - lb) * lnum + (rb - cand.first) * rnum, cand.first});
            if (cand.second != -1 && !visit[cand.second])
                ++lnum, visit[cand.second] = true;
        }

        cur->pos = min_t.second;
        vector<shared_ptr<hittable>> lobjects, robjects;
        for (auto &object : objects){
            auto bound = object->bound(cur->axis);
            if (bound.first < cur->pos - epsilon)
                lobjects.push_back(object);
            if (bound.second > cur->pos + epsilon)
                robjects.push_back(object);
        }

        if ( ((lobjects.size() + robjects.size()) >= objects.size() * 1.4) ||
             min_t.first == infinity ){
            ++depth;
            continue;
        }

        auto lbox = bbox, rbox = bbox;
        lbox.upper[cur->axis] = rbox.lower[cur->axis] = cur->pos;
        cur->left = __build_tree(lobjects, depth + 1, lbox);
        cur->right = __build_tree(robjects, depth + 1, rbox);
        return cur;
    }
    cur->leaf = true;
    cur->objects = objects;
    return cur;
}

bool kdtree::hit(const ray &r, double t_min, double t_max, hit_record &rec) const{
    auto origin = r.origin();
    auto dir = r.direction();
    
    stack<pair<kdnode *, box>> kdnodes;
    kdnodes.push({root, bbox_r});

    while (!kdnodes.empty()){
        auto cur = kdnodes.top();
        kdnodes.pop();

        auto &node = cur.first;
        auto &bbox = cur.second;
        if (node->leaf){
            hit_record temp_rec;
            bool hit_anything = false;
            auto closest_so_far = t_max;
            for (const auto& object : node->objects) {
                if (object->hit(r, t_min, closest_so_far, temp_rec) && bbox.on(temp_rec.p)) {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }
            if (hit_anything)
                return hit_anything;
            continue;
        }
        pair<kdnode *, box> near(node->left, bbox), far(node->right, bbox);
        near.second.upper[node->axis] = far.second.lower[node->axis] = node->pos;
        if (dir[node->axis] < 0)
            swap(near, far);

        auto ts = bbox.hit(r, t_min, t_max);
        if (ts.empty()) continue;
        double t_split = (node->pos - origin[node->axis]) / dir[node->axis];
        
        if (t_split <= ts[1] && ts[1] > t_min)
            kdnodes.push(far);
        if (ts[0] <= t_split && t_split > t_min)
            kdnodes.push(near);
    }
    return false;
}

vector<double> box::hit(const ray &r, double t_min, double t_max){
    vector<double> ret;
    auto origin = r.origin();
    auto dir = r.direction();

    for (int i = 0; i != 3; ++i){
        double t = (lower[i] - origin[i]) / dir[i];
        if (on(r.at(t)))
            ret.push_back(t);
        t = (upper[i] - origin[i]) / dir[i];
        if (on(r.at(t)))
            ret.push_back(t);
    }
    if (ret.size() == 1)
        ret.push_back(ret.back());
    if (!ret.empty())
        sort(ret.begin(), ret.end());
    return ret;
}

bool box::on(point3 x){
    for (int i = 0; i != 3; ++i)
        if (x[i] < lower[i] - epsilon || upper[i] + epsilon < x[i])
            return false;
    return true;            
}


#endif