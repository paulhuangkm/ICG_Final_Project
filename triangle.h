#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "vec3.h"

class triangle : public hittable {
    public:
        triangle() {}
        //triangle(point3 cen, double r, shared_ptr<material> m)
        //    : center(cen), radius(r), mat_ptr(m) {};
	    triangle(point3 p1, point3 p2, point3 p3, shared_ptr<material> m)
		    : vertex{p1, p2, p3}, mat_ptr(m) {};
        virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override;

        double deter(double x00, double x01, double x02, double x10, double x11, double x12, double x20, double x21, double x22) const;

    public:
        //point3 center;
        //double radius;
        point3 vertex[3];
    	shared_ptr<material> mat_ptr;
};

double triangle::deter(double x00, double x01, double x02, double x10, double x11, double x12, double x20, double x21, double x22) const {
    return x00*x11*x22 + x01*x12*x20 + x02*x10*x21 - x00*x12*x21 - x01*x10*x22 - x02*x11*x20;
}

bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    bool found_t = false;
    vec3 outward_normal;

    vec3 v1 = vertex[2] - vertex[0], v2 = vertex[1] - vertex[0];

    // double a, b, t; 
    // r.orig + t*r.dir = vertex[0] + a*v1 + b*v2
    // a*v1 + b*v2 + t*(-r.dir) = r.orig - vertex[0];
    double delta  = deter(v1[0], v2[0], -r.dir[0], v1[1], v2[1], -r.dir[1], v1[2], v2[2], -r.dir[2]);
    
    if(abs(delta) <= 10e-8) {
        return false;
    }
    
    double delta_a = deter(r.orig[0] - vertex[0][0], v2[0], -r.dir[0], r.orig[1] - vertex[0][1], v2[1], -r.dir[1], r.orig[2]-vertex[0][2], v2[2], -r.dir[2]);
    double delta_b = deter(v1[0], r.orig[0] - vertex[0][0], -r.dir[0], v1[1], r.orig[1] - vertex[0][1], -r.dir[1], v1[2], r.orig[2]-vertex[0][2], -r.dir[2]);
    double delta_t = deter(v1[0], v2[0], r.orig[0] - vertex[0][0], v1[1], v2[1], r.orig[1] - vertex[0][1], v1[2], v2[2], r.orig[2]-vertex[0][2]);

    double a = delta_a/delta;
    double b = delta_b/delta;
    double t = delta_t/delta;

    if(a >= 0 && b >= 0 && a+b <= 1 && t_min <= t && t <= t_max) {
        found_t = true;
        outward_normal = cross(v1, v2);
        if(dot(outward_normal, r.direction()) > 0)
            outward_normal *= -1;
    }

    if(!found_t)
        return false;
    //fprintf(stderr, "succeed to hit triangle\n");
	
    rec.t = t;
    rec.p = r.at(rec.t);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;

    return true;
}


#endif