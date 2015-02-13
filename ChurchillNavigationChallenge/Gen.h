//
//  Gen.h
//  ChurchillNavigationChallenge
//
//  Created by Eric Campbell on 2/12/15.
//  Copyright (c) 2015 ECC. All rights reserved.
//

#ifndef ChurchillNavigationChallenge_Gen_h
#define ChurchillNavigationChallenge_Gen_h

#include "Shared.h"

const int NUM_PTS = 50000;
const int MAX_PT_RANGE = 1024;

//const int NUM_PTS = 100000;
//const int MAX_PT_RANGE = std::numeric_limits<int>::max();

struct QueryResult {
    int i;
    int ct_bf;
    int ct_qt;
    int ct_kd;
    float bf;
    float qt;
    float kd;
    
    QueryResult() : i(0), ct_bf(0), ct_qt(0), bf(0), qt(0), kd(0), ct_kd(0) {}
};

static int rand_num() {
    return rand() % MAX_PT_RANGE;
}

float random_float(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

void generate_points(int ct, std::vector<Point*>& points)
{
    points.resize(ct);
    
    for (int i = 0; i < ct; i++) {
        Point* p = new Point();
        p->id = rand() % 10000;
        p->rank = i;
        
        while (p->x == 0) {
            p->x = random_float(0, MAX_PT_RANGE);
        }
        
        while (p->y == 0) {
            p->y = random_float(0, MAX_PT_RANGE);
        }
        points[i] = p;
    }
}

void generate_queries(int ct, std::vector<Rect>& queries) {
    for (int i = 0; i < ct; i++) {
        int r = rand_num();
        int r2 = rand_num();
        queries.push_back(Rect(r/2, r, r2/2, r2));
    }
}

#endif
