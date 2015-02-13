//
//  Shared.h
//  ChurchillNavigationChallenge
//
//  Created by Eric Campbell on 2/9/15.
//  Copyright (c) 2015 ECC. All rights reserved.
//

#ifndef ChurchillNavigationChallenge_Shared_h
#define ChurchillNavigationChallenge_Shared_h

#include <vector>
#include <queue>

struct QuadTree;
struct KdTree;

struct Point
{
    short id;
    int rank;
    float x;
    float y;
    
    Point() : id(0), rank(0), x(0), y(0) {}
    Point(const Point&) {}
    
    bool operator()(const Point& l, const Point& r)
    {
        return l.rank < r.rank;
    }
};

struct Rect
{
    float lx;
    float ly;
    float hx;
    float hy;
    
    Rect() {
        lx = 0;
        ly = 0;
        hx = 0;
        hy = 0;
    }
    
    Rect(float lx, float hx, float ly, float hy) : lx(lx), hx(hx), ly(ly), hy(hy) {}
    Rect(const Rect& other) { lx = other.lx; ly = other.ly; hx = other.hx; hy = other.hy; }
};

#endif
