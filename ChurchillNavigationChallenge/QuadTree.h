//
//  QuadTree.h
//  ChurchillNavigationChallenge
//
//  Created by Eric Campbell on 2/11/15.
//  Copyright (c) 2015 ECC. All rights reserved.
//

#ifndef ChurchillNavigationChallenge_QuadTree_h
#define ChurchillNavigationChallenge_QuadTree_h

#include "Shared.h"

const int QT_MAX_PER_NODE = 32; // Maximun number of points per node before subdividing
const int QT_MAX_DEPTH = 64;    // Maximum depth to allow before dumping all additional points into the leaf node

// QuadTree node struct
struct QuadTree {
    Rect bounds;  // 2D Rectangular bounds of this node
    int depth;    // Depth of this node in the tree
    
    QuadTree *nw;  // Northwest subdivision quadrant
    QuadTree *ne;  // Northeast subdivision quadrant
    QuadTree *sw;  // Southwest subdivision quadrant
    QuadTree *se;  // Southeast subdivision quadrant
    
    std::vector<Point*> pts; // Points in this node
    
    QuadTree() : depth(0), nw(0), sw(0), ne(0), se(0) { }
};

// Create a quadtree node, initialized with bounds and depth
static QuadTree* quadtree_construct(Rect bounds, int depth) {
    QuadTree* node = new QuadTree();
    node->bounds = bounds;
    node->depth = depth;
    node->nw = 0;
    node->ne = 0;
    node->sw = 0;
    node->se = 0;
    return node;
}

// Delete a root quadtree node and all child nodes
static void quadtree_delete(QuadTree* root) {
    if (root != 0) {
        quadtree_delete(root->nw);
        if (root->ne != 0)
            quadtree_delete(root->ne);
        if (root->sw != 0)
            quadtree_delete(root->sw);
        if (root->se != 0)
            quadtree_delete(root->se);
        
        delete root;
        root = 0;
    }
}

// Forward declaration of internal quadtree_insert method (below)
static bool quadtree_insert(QuadTree* node, Point* p);

// Helper method to insert a vector of points at once
// For the Churchill Navigation challenge, these points are sorted in ascending order by their Rank value,
// assuring we always have the lowest ranked nodes at the top level of the tree when searching breadth first
static void quadtree_insert(QuadTree* root, std::vector<Point*>& points, int& ct) {
    for (std::vector<Point*>::iterator it = points.begin() ; it != points.end(); ++it) {
        quadtree_insert(root, *it);
        ct++;
    }
}

// Helper method to print out a quadtree node and it's child nodes
static inline void quadtree_print(QuadTree* node) {
    printf("QT %.*s depth=%d lx=%d hx=%d ly=%d hy=%d pts=%d \n",
           node->depth, "                                               ",
           node->depth, node->bounds.lx, node->bounds.hx, node->bounds.ly, node->bounds.hy, node->pts.size());
    if (node->nw != 0) {
        quadtree_print(node->nw);
        if (node->ne != 0)
            quadtree_print(node->ne);
        if (node->sw != 0)
            quadtree_print(node->sw);
        if (node->se != 0)
            quadtree_print(node->se);
    }
}

// Return all points within this node and all of it's children
// This is used when the boundary is fully contained within the search
// range and further rect intersection/containment checks are no longer needed
static inline void quadtree_return_subtree(QuadTree* node, std::priority_queue<Point*, std::vector<Point*>>& results, int& ct) {
    
    // Add all points within this node to the search results container
    for (std::vector<Point*>::iterator it = node->pts.begin() ; it != node->pts.end(); ++it) {
        if (results.size() < 20) {
            results.push(*it);
            ct++;
        }
        else if (results.top()->rank > (*it)->rank) {
            results.pop();
            results.push(*it);
            ct++;
        }
    }
    
    // Return all results in the child nodes of this node
    if (node->nw != 0) {
        quadtree_return_subtree(node->nw, results, ct);
        quadtree_return_subtree(node->ne, results, ct);
        quadtree_return_subtree(node->sw, results, ct);
        quadtree_return_subtree(node->se, results, ct);
    }
}

// Depth first search the quadtree node (root) for all points within query Rect, add them to the results container
// Top level points in the tree will always have the lowest ranks in that boundary, if we get to 20 (max search result count) we can stop searching
static inline void quadtree_search(QuadTree* node, const Rect query, std::priority_queue<Point*, std::vector<Point*>>& results, int& ct) {
    
    // If this node is fully contained within the search query, return all points in tree below this node
    if (rects_contained(query, node->bounds)) {
        quadtree_return_subtree(node, results, ct);
    }
    // If this node's boundary rectangle intersects with the query rectangle, then check all points in this node for containment
    // and add to the results container when inside the search rect
    else if (rects_intersect(node->bounds, query)) {
        for (std::vector<Point*>::iterator it = node->pts.begin() ; it != node->pts.end(); ++it) {
            if (pt_contained(query, **it)) {
                if (results.size() < 20) {
                    results.push(*it);
                    ct++;
                }
                else if (results.top()->rank > (*it)->rank) {
                    results.pop();
                    results.push(*it);
                    ct++;
                }
            }
        }
        
        // If there was an intersection, then recursively search the child nodes
        if (node->nw != 0) {
            quadtree_search(node->nw, query, results, ct);
            quadtree_search(node->ne, query, results, ct);
            quadtree_search(node->sw, query, results, ct);
            quadtree_search(node->se, query, results, ct);
        }
    }
    // Else no intersection and no containment, stop recursing the tree
}

// Subdivides this quadtree node assuming a left->right, bottom->up coordinate system
//       |
//       . (0,1)      . (1,1)
//       |
//       |
//       |
//_______.____________. (1,0)
//       | (0,0)
//       |
static inline void quadtree_subdivide(QuadTree* parent) {
    if (parent->nw == 0) {
        Rect nw_bounds, ne_bounds, sw_bounds, se_bounds;
        
        // Calculate the child NW bounds
        nw_bounds.lx = parent->bounds.lx;
        nw_bounds.hx = parent->bounds.lx + ((parent->bounds.hx - parent->bounds.lx) / 2);
        nw_bounds.ly = parent->bounds.ly + ((parent->bounds.hy - parent->bounds.ly) / 2);
        nw_bounds.hy = parent->bounds.hy;
        
        parent->nw = quadtree_construct(nw_bounds, parent->depth+1);
        
        // Calculate the child NE bounds
        ne_bounds.lx = parent->bounds.lx + ((parent->bounds.hx - parent->bounds.lx) / 2);
        ne_bounds.hx = parent->bounds.hx;
        ne_bounds.ly = parent->bounds.ly + ((parent->bounds.hy - parent->bounds.ly) / 2);
        ne_bounds.hy = parent->bounds.hy;
        
        parent->ne = quadtree_construct(ne_bounds, parent->depth+1);
        
        // Calculate the child SW bounds
        sw_bounds.lx = parent->bounds.lx;
        sw_bounds.hx = parent->bounds.lx + ((parent->bounds.hx - parent->bounds.lx) / 2);
        sw_bounds.ly = parent->bounds.ly;
        sw_bounds.hy = parent->bounds.ly + ((parent->bounds.hy - parent->bounds.ly) / 2);
        
        parent->sw = quadtree_construct(sw_bounds, parent->depth+1);
        
        // Calculate the child SE bounds
        se_bounds.lx = parent->bounds.lx + ((parent->bounds.hx - parent->bounds.lx) / 2);
        se_bounds.hx = parent->bounds.hx;
        se_bounds.ly = parent->bounds.ly;
        se_bounds.hy = parent->bounds.ly + ((parent->bounds.hy - parent->bounds.ly) / 2);
        
        parent->se = quadtree_construct(se_bounds, parent->depth+1);
    }
}

// Insert a point into the quadtree
static inline bool quadtree_insert(QuadTree* node, Point* p) {
    // If this point is outside the bounds of this node, return false early
    if (!pt_contained(node->bounds, *p)) {
        return false;
    }
    
    // If we have subdivided, add to the children
    if (node->nw != 0) {
        
        if (quadtree_insert(node->nw, p))
            return true;
        
        if (node->ne != 0)
            if (quadtree_insert(node->ne, p))
                return true;
            
        if (node->sw != 0)
            if (quadtree_insert(node->sw, p))
                return true;
            
        if (node->se != 0)
            if (quadtree_insert(node->se, p))
                return true;
            
        return false;
        
    } else {
        // If we haven't subdivided and we haven't reached the max number of points for this node OR we have reached the maximum depth already,
        // then add the point to this leaf node
        if (node->pts.size() < QT_MAX_PER_NODE || node->depth >= QT_MAX_DEPTH) {
            node->pts.push_back(p);
            return true;
        
        // Otherwise we subdivide and add the point to the child node it belongs in
        } else {
            quadtree_subdivide(node);
            
            // Don't re-assign the points to new nodes;
            // since the points are inserted already sorted by rank, the lowest ranks will always be first to be checked
            //
            //            while (node->pts.size() > 0) {
            //                Point* p = node->pts.back();
            //                node->pts.pop_back();
            //                quadtree_insert(node, *p);
            //            }
            
            // Re-call the insert on this node -- since we have now subdivided the tree,
            // it will go into one of the new child nodes
            return quadtree_insert(node, p);
        }
    }
    
    return false;
}

#endif
