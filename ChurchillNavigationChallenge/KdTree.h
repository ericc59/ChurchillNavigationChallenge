//
//  KdTree.h
//  ChurchillNavigationChallenge
//
//  Created by Eric Campbell on 2/12/15.
//  Copyright (c) 2015 ECC. All rights reserved.
//

#ifndef ChurchillNavigationChallenge_KdTree_h
#define ChurchillNavigationChallenge_KdTree_h

#include "Shared.h"
#include <algorithm>
#include <vector>

const int KD_MAX_DEPTH = 8;

struct KdTree {
    
    Rect bounds;
    int depth;
    std::vector<Point*> points;
    KdTree* left;
    KdTree* right;
    
    KdTree() : depth(0), left(0), right(0) { }
    
};

static KdTree* kdtree_construct(Rect bounds, int depth) {
    KdTree* tree = new KdTree();
    tree->depth = depth;
    tree->bounds = bounds;
    return tree;
}

static void kdtree_delete(KdTree* tree) {
    
}

static bool kd_compare_pts_rank(const Point* p1, const Point* p2) {
    return p1->rank < p2->rank;
}

static bool kd_compare_pts_x(const Point* p1, const Point* p2) {
    return p1->x < p2->x;
}

static bool kd_compare_pts_y(const Point* p1, const Point* p2) {
    return p1->y < p2->y;
}

static void kdtree_insert(KdTree* tree, std::vector<Point*>& pts) {
    
    if (pts.size() == 1) {
        tree->points.push_back(pts[0]);
        return;
    }
    
    if (tree->depth >= KD_MAX_DEPTH) {
        
        for (int i = 0; i < pts.size(); i++) {
            tree->points.push_back(pts[i]);
        }
        
        std::sort(tree->points.begin(), tree->points.end(), kd_compare_pts_rank);
        
        return;
    }
    
    const size_t median_index = pts.size() / 2;
    
    bool (*comparator)(const Point* p1, const Point* p2);
    
    if (tree->depth % 2 == 0)
        comparator = &kd_compare_pts_x;
    else
        comparator = &kd_compare_pts_y;
    
    std::nth_element(pts.begin(), pts.begin()+median_index, pts.end(), comparator);

    tree->points.push_back(pts[median_index]);
    
    std::vector<Point*> left_pts = std::vector<Point*>(pts.begin(), pts.begin() + median_index);
    std::vector<Point*> right_pts = std::vector<Point*>(pts.begin() + median_index + 1, pts.end());

    if (left_pts.size() > 0) {
        Rect left_rect;
        
        if (tree->depth % 2 == 0) {
            
            left_rect.lx = tree->bounds.lx;
            left_rect.hx = pts[median_index]->x;
            left_rect.ly = tree->bounds.ly;
            left_rect.hy = tree->bounds.hy;

        } else {
            
            left_rect.lx = tree->bounds.lx;
            left_rect.hx = tree->bounds.hx;
            left_rect.ly = tree->bounds.ly;
            left_rect.hy = pts[median_index]->y;
            
        }
        
        tree->left = kdtree_construct(left_rect, tree->depth+1);
        kdtree_insert(tree->left, left_pts);
    }
    
    if (right_pts.size() > 0) {
        Rect right_rect;
        
        if (tree->depth % 2 == 0) {
            
            right_rect.lx = pts[median_index]->x;
            right_rect.hx = tree->bounds.hx;
            right_rect.ly = tree->bounds.ly;
            right_rect.hy = tree->bounds.hy;
            
        } else {
            
            right_rect.lx = tree->bounds.lx;
            right_rect.hx = tree->bounds.hx;
            right_rect.ly = pts[median_index]->y;
            right_rect.hy = tree->bounds.hy;
            
        }
        
        tree->right = kdtree_construct(right_rect, tree->depth+1);
        kdtree_insert(tree->right, right_pts);
    }
}

static inline void kdtree_return_subtree(KdTree* tree, std::priority_queue<Point*, std::vector<Point*>>& results, int& ct) {
    for (std::vector<Point*>::iterator it = tree->points.begin() ; it != tree->points.end(); ++it) {
    
        if (results.size() < 20) {
            results.push(*it);
            ct++;
        }
        else if (results.top()->rank > (*it)->rank) {
            results.pop();
            results.push(*it);
            ct++;
        } else {
            break;
        }
        
    }
    

    if (tree->left != 0) {
        kdtree_return_subtree(tree->left, results, ct);
    }
    
    if (tree->right != 0) {
        kdtree_return_subtree(tree->right, results, ct);
    }
}

static inline void kdtree_search(KdTree* tree, const Rect& query, std::priority_queue<Point*, std::vector<Point*>>& results, int& ct) {
    if (rects_contained(query, tree->bounds)) {
        kdtree_return_subtree(tree, results, ct);
    }
    else if (rects_intersect(tree->bounds, query)) {
        for (std::vector<Point*>::iterator it = tree->points.begin() ; it != tree->points.end(); ++it) {
            if (pt_contained(query, **it)) {
                if (results.size() < 20) {
                    results.push(*it);
                    ct++;
                }
                else if (results.top()->rank > (*it)->rank) {
                    results.pop();
                    results.push(*it);
                    ct++;
                } else {
                    break;
                }
            }
        }
        
        if (tree->left != 0)
            kdtree_search(tree->left, query, results, ct);
    
        if (tree->right != 0)
            kdtree_search(tree->right, query, results, ct);
    }
}

#endif
