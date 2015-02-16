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

const int KD_MAX_DEPTH = 8; // Max depth of the KD tree -- stops subdividing once it reaches this depth

struct KdTree {
    
    Rect bounds;
    int depth;
    std::vector<Point*> points;
    KdTree* left;
    KdTree* right;
    
    KdTree() : depth(0), left(0), right(0) { }
    
};

// Create a new KdTree node and initialize with bounds and dpeth
static KdTree* kdtree_construct(Rect bounds, int depth) {
    KdTree* tree = new KdTree();
    tree->depth = depth;
    tree->bounds = bounds;
    return tree;
}

// Delete this kdtree and it's child nodes
static void kdtree_delete(KdTree* tree) {
    if (tree->left != 0) {
        kdtree_delete(tree->left);
    }
    
    if (tree->right != 0) {
        kdtree_delete(tree->right);
    }
    
    delete tree;
    tree = 0;
}

// Method for comparing points by their rank
static inline bool kd_compare_pts_rank(const Point* p1, const Point* p2) {
    return p1->rank < p2->rank;
}

// Method for comparing points by their X value
static inline bool kd_compare_pts_x(const Point* p1, const Point* p2) {
    return p1->x < p2->x;
}

// Method for comparing points by their Y value
static inline bool kd_compare_pts_y(const Point* p1, const Point* p2) {
    return p1->y < p2->y;
}

// Insert a vector of points into the tree
static void kdtree_insert(KdTree* tree, std::vector<Point*>& pts) {
    
    // If we're down to one point, insert it into this leaf node
    if (pts.size() == 1) {
        tree->points.push_back(pts[0]);
        return;
    }
    
    // If we've reached the maximum depth, add all remaining points into this leaf node and stop subdividing
    if (tree->depth >= KD_MAX_DEPTH) {
        for (int i = 0; i < pts.size(); i++) {
            tree->points.push_back(pts[i]);
        }
        
        // Sort the points by rank so we can get the lowest ranks first when searching
        std::sort(tree->points.begin(), tree->points.end(), kd_compare_pts_rank);
        
        // Stop subdividing
        return;
    }
    
    // Median index for splitting the points
    const size_t median_index = pts.size() / 2;
    
    // Select comparison method based on the depth - (even=X, odd=Y)
    // Since this is a 2D kdtree, we only have 2 axes
    bool (*comparator)(const Point* p1, const Point* p2);
    
    if (tree->depth % 2 == 0)
        comparator = &kd_compare_pts_x;
    else
        comparator = &kd_compare_pts_y;

    // Reorder the points vector such that all points with an index value LEFT of the median index
    // have an X or Y value (depending on depth) less than the value at the median index.
    // Doesn't need to completely sort the vector, which makes it much faster for large data sets
    std::nth_element(pts.begin(), pts.begin()+median_index, pts.end(), comparator);

    // Add the selected median point to this tree's points vector
    tree->points.push_back(pts[median_index]);
    
    // Split the points into the left and right vectors for child noes
    std::vector<Point*> left_pts = std::vector<Point*>(pts.begin(), pts.begin() + median_index);
    std::vector<Point*> right_pts = std::vector<Point*>(pts.begin() + median_index + 1, pts.end());

    // If there are still points on the left side then calculate the new bounds for the left child node and recursively
    // add the left points down the tree
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
    
    // If there are still points on the right side then calculate the new bounds for the right child node and recursively
    // add the riht points down the tree
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

// Returns the entire subtree with no bounds checking - Used when this node's bounds are fully contained with the search rect
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

// Depth-first recursive searching the tree with a 2D rectangular range query and return the results in the results container
static inline void kdtree_search(KdTree* tree, const Rect& query, std::priority_queue<Point*, std::vector<Point*>> & results, int& ct) {
    
    // If this node's bounds are fully contained within the search query bounds, then return the entire subtree
    if (rects_contained(query, tree->bounds)) {
        kdtree_return_subtree(tree, results, ct);
    }
    // Else, if there's an intersection between this node's bounds and the search query bounds, keep searching
    else if (rects_intersect(tree->bounds, query)) {
        // Check all points in this leaf node for containment
        for (std::vector<Point*>::iterator it = tree->points.begin() ; it != tree->points.end(); ++it) {
            if (pt_contained(query, **it)) {
                // For this challenge, we only want the 20 points with the lowest rank value
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
        
        // Recursively search the left node
        if (tree->left != 0)
            kdtree_search(tree->left, query, results, ct);
    
        // Recursively search the right node
        if (tree->right != 0)
            kdtree_search(tree->right, query, results, ct);
    }
}

#endif
