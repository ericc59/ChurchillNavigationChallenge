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
ppm quadtree_img(1024, 1024);
ppm kdtree_img(1024, 1024);
#endif

std::vector<QueryResult> query_results;
std::vector<Rect> queries;
QuadTree* qt;
KdTree* kdt;
std::vector<Point*> points;

void setup_data(int num_search_queries, int num_points, int max_point_range);
void execute_searches();
void display_search_results();

int main(int argc, const char * argv[])
{
    /* initialize random seed: */
    srand (1000000000000);//time(NULL)
    
    // Setup the Search Query, Points, QuadTree, and KdTree data
    setup_data(1, NUM_PTS, MAX_PT_RANGE);
    
#ifdef RENDER_QUADTREE
    // Render the QuadTree and KdTree to simple PPM images for visual debugging of subdivision and point dispersement
    ppm_draw_quadtree(quadtree_img, qt, ppm::pixel(75, 75, 145));
    ppm_draw_kdtree(kdtree_img, kdt, ppm::pixel(75, 75, 145));
    
    // Output the quadtree and kdtree PPM images
    ppm_write(quadtree_img, "/Users/ericc/Desktop/quadtree.pbm");
    ppm_write(kdtree_img, "/Users/ericc/Desktop/kdtree.pbm");
#endif
    
    execute_searches();
    display_search_results();
    
    // Clean up heap allocations
    quadtree_delete(qt);
    kdtree_delete(kdt);
    
    for (int i = 0; i < points.size(); i++) {
        delete points[i];
        points[i] = 0;
    }
    
    return 0;
}

void setup_data(int num_search_queries, int num_points, int max_point_range) {
    // Generate random 2D rectangular search queries
    generate_queries(num_search_queries, queries);
    
    // Generate NUM_PTS points
    generate_points(num_points, points);
    
    // Time measurement
    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    
    // Create the QuadTree and insert the points vector
    /////
    qt = quadtree_construct(Rect(0, max_point_range, 0, max_point_range), 0);
    int insert_ct = 0;
    quadtree_insert(qt, points, insert_ct);
    /////
    end = std::chrono::steady_clock::now();
    diff = end - start;
    std::cout << "QuadTree Creation Time: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
    
    
    start = std::chrono::steady_clock::now();
    // Create the KdTree and insert the points vector
    /////
    kdt = kdtree_construct(Rect(0, max_point_range, 0, max_point_range), 0);
    kdtree_insert(kdt, points);
    /////
    end = std::chrono::steady_clock::now();
    diff = end - start;
    std::cout << "KdTree Creation Time: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
    
}

void execute_searches() {
    // Measure search times
    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    
    int i = 0;
    
    for (std::vector<Rect>::iterator q = queries.begin() ; q != queries.end(); ++q) {
#ifdef RENDER_QUADTREE
        ppm_draw_rect_fill(quadtree_img, *q, ppm::pixel(15, 15, 15));
        ppm_draw_rect_fill(kdtree_img, *q, ppm::pixel(15, 15, 15));
#endif
        i++;
        
        // Priority queues for keeping a sorted list of the 20 lowest ranked points
        std::priority_queue<Point*, std::vector<Point*>> results, results2, results3;
        
        QueryResult qr;
        qr.i = i;
        
        start = std::chrono::steady_clock::now();
        
        // Do a brute force search of all points in O(N^2) time
        for (int i = 0; i < points.size(); i++) {
            if (pt_contained(*q, *points[i]))
                results.push(points[i]);
        }
        
        end = std::chrono::steady_clock::now();
        diff = end - start;
        qr.bf = std::chrono::duration <double, std::milli> (diff).count();
        qr.ct_bf = results.size();
        
        // Search quadtree in ~O(log N) time
        start = std::chrono::steady_clock::now();
        int ct = 0;
        quadtree_search(qt, *q, results2, ct);
        end = std::chrono::steady_clock::now();
        diff = end - start;
        qr.qt = std::chrono::duration <double, std::milli> (diff).count();
        qr.ct_qt = results2.size();
        
        // Search KdTree in O(n^(1-1/k) + m) time, where m is the number of the reported points, and k the dimension of the k-d tree
        start = std::chrono::steady_clock::now();
        kdtree_search(kdt, *q, results3, ct);
        end = std::chrono::steady_clock::now();
        diff = end - start;
        qr.kd = std::chrono::duration <double, std::milli> (diff).count();
        qr.ct_kd = results3.size();
        
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
}

void display_search_results() {
    float avg_bf = 0;
    float avg_qt = 0;
    float avg_kd = 0;
    
    // Display search results
    for (std::vector<QueryResult>::iterator q = query_results.begin() ; q != query_results.end(); ++q) {
        std::cout << "=============================================" << std::endl;
        std::cout << "QUERY " << (*q).i << std::endl;
        std::cout << " " << std::endl;
        std::cout << "Brute Force Time: " << (*q).bf << " ms" << std::endl;
        std::cout << "Brute Force Results: " << (*q).ct_bf << std::endl;
        std::cout << " " << std::endl;
        std::cout << "QuadTree Time: " << (*q).qt << " ms" << std::endl;
        std::cout << "QuadTree Results: " << (*q).ct_qt << std::endl;
        std::cout << " " << std::endl;
        std::cout << "KdTree Time: " << (*q).kd << " ms" << std::endl;
        std::cout << "KdTree Results: " << (*q).ct_kd << std::endl;
        std::cout << " " << std::endl;
        
        avg_bf += (*q).bf;
        avg_qt += (*q).qt;
        avg_kd += (*q).kd;
    }
    
    // Calculate search time averages
    avg_bf /= query_results.size();
    avg_qt /= query_results.size();
    avg_kd /= query_results.size();
    
    // Display search averages
    std::cout << "AVG Brute Force Search Time: " << avg_bf << " ms" << std::endl;
    std::cout << "AVG Quad Tree Search Time: " << avg_qt << " ms" << std::endl;
    std::cout << "AVG KdTree Search Time : " << avg_kd << " ms" << std::endl;
}


