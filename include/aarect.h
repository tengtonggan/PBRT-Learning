
#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif


#ifndef AARECT_H
#define AARECT_H

#include "rtweekend.h"

#include "hittable.h"
#include "shape.h"

class xy_rect : public hittable {
public:
    xy_rect() {}

    xy_rect(double _x0, double _x1, double _y0, double _y1, double _k,
        shared_ptr<material> mat)
        : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {};

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
        // The bounding box must have non-zero width in each dimension, so pad the Z
        // dimension a small amount.
        output_box = aabb(Point3f(x0, y0, k - 0.0001), Point3f(x1, y1, k + 0.0001));
        return true;
    }

public:
    shared_ptr<material> mp;
    double x0, x1, y0, y1, k;
};

class xz_rect : public hittable {
public:
    xz_rect() {}

    xz_rect(double _x0, double _x1, double _z0, double _z1, double _k,
        shared_ptr<material> mat)
        : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
        // The bounding box must have non-zero width in each dimension, so pad the Y
        // dimension a small amount.
        output_box = aabb(Point3f(x0, k - 0.0001, z0), Point3f(x1, k + 0.0001, z1));
        return true;
    }

    //caculate sample pdf
    virtual double pdf_value(const Point3f& origin, const Vector3f& v) const override {
        hit_record rec;
        if (!this->hit(ray(origin, v), 0.001, infinity, rec))
            return 0;

        auto area = (x1 - x0) * (z1 - z0);
        auto distance_squared = rec.t * rec.t * v.LengthSquared();
        auto cosine = fabs(Dot(v, rec.normal) / v.Length());

        return distance_squared / (cosine * area);
    }

    //uniform sample in plane
    virtual Vector3f random(const Point3f& origin) const override {
        auto random_point = Point3f(random_Float(x0, x1), k, random_Float(z0, z1));
        return random_point - origin;
    }

public:
    shared_ptr<material> mp;
    double x0, x1, z0, z1, k;
};

class yz_rect : public hittable {
public:
    yz_rect() {}

    yz_rect(double _y0, double _y1, double _z0, double _z1, double _k,
        shared_ptr<material> mat)
        : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
        // The bounding box must have non-zero width in each dimension, so pad the X
        // dimension a small amount.
        output_box = aabb(Point3f(k - 0.0001, y0, z0), Point3f(k + 0.0001, y1, z1));
        return true;
    }

public:
    shared_ptr<material> mp;
    double y0, y1, z0, z1, k;
};


//Rect
namespace pbrt {

    // Disk Declarations x0<=x1;y0<=y1
    class XY_Rect : public Shape {
    public:
        // Disk Public Methods
        XY_Rect(shared_ptr<Transform> ObjectToWorld, shared_ptr<Transform> WorldToObject,
            bool reverseOrientation, Float x_0, Float x_1, Float y_0, Float y_1, Float z_ = 0.0)
            : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
            x0(x_0),
            x1(x_1),
            y0(y_0),
            y1(y_1),
            z(z_) {}
        Bounds3f ObjectBound() const;
        bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* isect,
            bool testAlphaTexture) const;
        bool IntersectP(const Ray& ray, bool testAlphaTexture) const;
        Float Area() const;
        //Interaction Sample(const Point2f& u, Float* pdf) const;

    private:
        // Disk Private Data
        const Float x0, x1, y0, y1, z;
    };

    //std::shared_ptr<Disk> CreateDiskShape(const Transform* o2w,
    //    const Transform* w2o,
    //    bool reverseOrientation,
    //    const ParamSet& params);


    class YZ_Rect : public Shape {
    public:
        // Disk Public Methods
        YZ_Rect(shared_ptr<Transform> ObjectToWorld, shared_ptr<Transform> WorldToObject,
            bool reverseOrientation, Float y_0, Float y_1, Float z_0, Float z_1, Float x_ = 0.0)
            : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
            y0(y_0),
            y1(y_1),
            z0(z_0),
            z1(z_1),
            x(x_) {}
        Bounds3f ObjectBound() const;
        bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* isect,
            bool testAlphaTexture) const;
        bool IntersectP(const Ray& ray, bool testAlphaTexture) const;
        Float Area() const;
        //Interaction Sample(const Point2f& u, Float* pdf) const;

    private:
        // Disk Private Data
        const Float y0, y1, z0, z1, x;
    };


    class ZX_Rect : public Shape {
    public:
        // Disk Public Methods
        ZX_Rect(shared_ptr<Transform> ObjectToWorld, shared_ptr<Transform> WorldToObject,
            bool reverseOrientation, Float z_0, Float z_1, Float x_0, Float x_1, Float y_ = 0.0)
            : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
            z0(z_0),
            z1(z_1),
            x0(x_0),
            x1(x_1),
            y(y_) {}
        Bounds3f ObjectBound() const;
        bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* isect,
            bool testAlphaTexture) const;
        bool IntersectP(const Ray& ray, bool testAlphaTexture) const;
        Float Area() const;
        //Interaction Sample(const Point2f& u, Float* pdf) const;

    private:
        // Disk Private Data
        const Float z0, z1, x0, x1, y;
    };

}  // namespace pbrt





#endif