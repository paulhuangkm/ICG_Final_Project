#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "hittable.h"
#include "vec3.h"

class rectangle : public hittable {
    public:
        rectangle() {}
        rectangle(double xa, double xb, double ya, double yb, double za, double zb, int nd, double k0, shared_ptr<material> m)
            : x0(xa), x1(xb), y0(ya), y1(yb), z0(za), z1(zb), norm_direction(nd), k(r0), mat_ptr(m) {};

        virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override;

    public:
        int norm_direction; // 1: x=k,  2: y=k,  3: z=k
        double x0, x1, y0, y1, z0, z1, k;
        shared_ptr<material> mat_ptr;
};

bool rectangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    double t, x, y, z;
    switch(norm_direction) {
        case 1:
            t = (k - r.origin().x()) / r.direction().x();
            break;
        case 2:
            t = (k - r.origin().y()) / r.direction().y();
            break;
        case 3:
            t = (k - r.origin().z()) / r.direction().z();
            break;
    }
    if(t<t_min || t>t_max)
        return false;

    x = r.origin().x() + t*r.direction().x();
    y = r.origin().y() + t*r.direction().y();
    z = r.origin().z() + t*r.direction().z();
    switch(norm_direction) {
        case 1:
            if(y<y0 || y>y1 || z<z0 || z>z1)
                return false;
            break;
        case 2:
            if(x<x0 || x>x1 || z<z0 || z>z1)
                return false;
            break;
        case 3:
            if(x<x0 || x>x1 || y<y0 || y>y1)
                return false;
            break;
    }

    rec.t = t;
    rec.p = r.at(rec.t);
    vec3 outward_normal;
    switch(norm_direction) {
        case 1:
            outward_normal = vec3(1, 0, 0);
            break;
        case 2:
            outward_normal = vec3(0, 1, 0);
            break;
        case 3:
            outward_normal = vec3(0, 0, 1);
            break;
    }
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;

    return true;
}

#endif