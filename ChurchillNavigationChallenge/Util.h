//
//  Util.h
//  ChurchillNavigationChallenge
//
//  Created by Eric Campbell on 2/12/15.
//  Copyright (c) 2015 ECC. All rights reserved.
//

#ifndef ChurchillNavigationChallenge_Util_h
#define ChurchillNavigationChallenge_Util_h

#include "Shared.h"

// true if r1 intersects r2
static bool inline rects_intersect(const Rect& r1, const Rect& r2) {
    if (r2.lx > r1.hx || r2.ly > r1.hy || r2.hx < r1.lx || r2.hy < r1.ly)
        return false;
    
    return true;
}

// true if r1 fully contains r2
static bool inline rects_contained(const Rect& r1, const Rect& r2) {
    if ((r1.lx <= r2.lx && r1.hx >= r2.hx) && r1.ly <= r2.ly && r1.hy >= r2.hy) return true;
    
    return false;
}

// true if r contains p
static bool inline pt_contained(const Rect& r, const Point& p) {
    return (r.lx <= p.x && r.hx >= p.x) && (r.ly <= p.y && r.hy >= p.y);
}

static inline void print_point(const Point& p) {
    printf("[Point x=%f y=%f]\n", p.x, p.y);
}

static inline void print_rect(Rect& rect) {
    printf("[Rect  %4.f %4.f  %4.f %4.f]\n", rect.lx, rect.hx, rect.ly, rect.hy);
}

#endif
