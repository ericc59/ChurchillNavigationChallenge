//
//  main.cpp
//  ChurchillNavigationChallenge
//
//  Created by Eric Campbell on 2/9/15.
//  Copyright (c) 2015 ECC. All rights reserved.
//

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cassert>
#include "Shared.h"
#include "Util.h"
#include "QuadTree.h"
#include "KdTree.h"
#include "Gen.h"

#define RENDER_QUADTREE

#ifdef RENDER_QUADTREE
#include "PPM.h"
#endif

std::vector<QueryResult> query_results;
std::vector<Rect> queries;
QuadTree* qt;
KdTree* kdt;
std::vector<Point*> points;

void setup_data() {
    generate_queries(0, queries);
    
    generate_points(NUM_PTS, points);
    
    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;

    /////
    qt = quadtree_construct(Rect(0, MAX_PT_RANGE, 0, MAX_PT_RANGE), 0);
    int insert_ct = 0;
    quadtree_insert(qt, points, insert_ct);
    /////
    
    end = std::chrono::steady_clock::now();
    diff = end - start;
    std::cout << "quadtree creation time: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
    
    
    
    start = std::chrono::steady_clock::now();
    
    /////
    kdt = kdtree_construct(Rect(0, MAX_PT_RANGE, 0, MAX_PT_RANGE), 0);
    kdtree_insert(kdt, points);
    /////
    
    end = std::chrono::steady_clock::now();
    diff = end - start;
    std::cout << "kdtree creation time: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;

}


int main(int argc, const char * argv[])
{
    /* initialize random seed: */
    srand (1000000000000);//time(NULL)
    
    setup_data();
    
    
#ifdef RENDER_QUADTREE
    ppm quadtree_img(1024, 1024);
    ppm_draw_quadtree(quadtree_img, qt, ppm::pixel(75, 75, 145));
    
    ppm kdtree_img(1024, 1024);
    ppm_draw_kdtree(kdtree_img, kdt, ppm::pixel(75, 75, 145));
#endif
    
    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;

    int i = 0;


    for (std::vector<Rect>::iterator q = queries.begin() ; q != queries.end(); ++q) {
#ifdef RENDER_QUADTREE
        ppm_draw_rect_fill(quadtree_img, *q, ppm::pixel(15, 15, 15));
        ppm_draw_rect_fill(kdtree_img, *q, ppm::pixel(15, 15, 15));
#endif
        
        std::priority_queue<Point*, std::vector<Point*>> results, results2, results3;
        
        i++;
        
        QueryResult qr;
        qr.i = i;

        start = std::chrono::steady_clock::now();
        
        for (int i = 0; i < points.size(); i++) {
            if (pt_contained(*q, *points[i]) && results.size() < 20)
                    results.push(points[i]);
        }
        
        end = std::chrono::steady_clock::now();
        diff = end - start;
        qr.bf = std::chrono::duration <double, std::milli> (diff).count();
        qr.ct_bf = results.size();
        
        
        start = std::chrono::steady_clock::now();
        int ct = 0;
        quadtree_search(qt, *q, results2, ct);
        end = std::chrono::steady_clock::now();
        diff = end - start;
        qr.qt = std::chrono::duration <double, std::milli> (diff).count();
        qr.ct_qt = results2.size();
        
        
        start = std::chrono::steady_clock::now();
        kdtree_search(kdt, *q, results3, ct);
        end = std::chrono::steady_clock::now();
        diff = end - start;
        qr.kd = std::chrono::duration <double, std::milli> (diff).count();
        qr.ct_kd = results3.size();

//        while (results2.size() > 0) {
//            std::cout << "result: " << results2.top()->rank << std::endl;
//            results2.pop();
//        }
        
#ifdef RENDER_QUADTREE
        while (results2.size() > 0) {
            ppm_set_pt(quadtree_img, results2.top()->x, results2.top()->y, ppm::green);
            results2.pop();
        }
        
        while (results3.size() > 0) {
            ppm_set_pt(kdtree_img, results3.top()->x, results3.top()->y, ppm::green);
            results3.pop();
        }
#endif
        
        query_results.push_back(qr);
    }
    std::cout << std::endl;
    
#ifdef RENDER_QUADTREE
    ppm_write_pbm(quadtree_img, "/Users/ericc/Desktop/quadtree.pbm");
    ppm_write_pbm(kdtree_img, "/Users/ericc/Desktop/kdtree.pbm");
#endif
    
    float avg_bf = 0;
    float avg_qt = 0;
    float avg_kd = 0;
    
    for (std::vector<QueryResult>::iterator q = query_results.begin() ; q != query_results.end(); ++q) {
        std::cout << "=============================================" << std::endl;
        std::cout << "QUERY " << (*q).i << std::endl;
        std::cout << " " << std::endl;
        std::cout << "brute force time: " << (*q).bf << " ms" << std::endl;
        std::cout << "brute force results: " << (*q).ct_bf << std::endl;
        std::cout << " " << std::endl;
        std::cout << "quadtree time: " << (*q).qt << " ms" << std::endl;
        std::cout << "quadtree results: " << (*q).ct_qt << std::endl;
        std::cout << " " << std::endl;
        std::cout << "kdtree time: " << (*q).kd << " ms" << std::endl;
        std::cout << "kdtree results: " << (*q).ct_kd << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        
        avg_bf += (*q).bf;
        avg_qt += (*q).qt;
        avg_kd += (*q).kd;
    }
    
    avg_bf /= query_results.size();
    avg_qt /= query_results.size();
    avg_kd /= query_results.size();
    
    std::cout << "AVG bf : " << avg_bf << " ms" << std::endl;
    std::cout << "AVG qt : " << avg_qt << " ms" << std::endl;
    std::cout << "AVG kd : " << avg_kd << " ms" << std::endl;
    
    
    
    
    // Clean up
    quadtree_delete(qt);
    kdtree_delete(kdt);
    
    for (int i = 0; i < points.size(); i++) {
        delete points[i];
        points[i] = 0;
    }
    
    return 0;
}

