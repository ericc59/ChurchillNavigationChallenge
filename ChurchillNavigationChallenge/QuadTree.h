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

const int QT_MAX_PER_NODE = 32;
const int QT_MAX_DEPTH = 64;


struct QuadTree {
    Rect bounds;
    int depth;
    
    QuadTree *nw;
    QuadTree *ne;
    QuadTree *sw;
    QuadTree *se;
    
    std::vector<Point*> pts;
    
    QuadTree() : depth(0), nw(0), sw(0), ne(0), se(0) { }
};

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

static bool quadtree_insert(QuadTree* node, Point* p);

static void quadtree_insert(QuadTree* root, std::vector<Point*>& points, int& ct) {
    for (std::vector<Point*>::iterator it = points.begin() ; it != points.end(); ++it) {
        quadtree_insert(root, *it);
        ct++;
    }
}

static inline void quadtree_print(QuadTree* node) {
    printf("QT %.*s depth=%d lx=%d hx=%d ly=%d hy=%d pts=%d \n",
           node->depth, "                                               ",
           node->depth,
           node->bounds.lx, node->bounds.hx, node->bounds.ly, node->bounds.hy, node->pts.size());
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

static inline void quadtree_return_subtree(QuadTree* node, std::priority_queue<Point*, std::vector<Point*>>& results, int& ct) {
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
    
    if (node->nw != 0) {
        quadtree_return_subtree(node->nw, results, ct);
        quadtree_return_subtree(node->ne, results, ct);
        quadtree_return_subtree(node->sw, results, ct);
        quadtree_return_subtree(node->se, results, ct);
    }
}

static inline void quadtree_search(QuadTree* node, const Rect query, std::priority_queue<Point*, std::vector<Point*>>& results, int& ct) {
    if (rects_contained(query, node->bounds)) {
        int ct_before = ct;
        quadtree_return_subtree(node, results, ct);
        int diff = ct - ct_before;
    }
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
        
        if (node->nw != 0) {
            quadtree_search(node->nw, query, results, ct);
            quadtree_search(node->ne, query, results, ct);
            quadtree_search(node->sw, query, results, ct);
            quadtree_search(node->se, query, results, ct);
        }
    }
}

// Subdivides assuming a left->right, bottom->up coordinate system
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
        
        nw_bounds.lx = parent->bounds.lx;
        nw_bounds.hx = parent->bounds.lx + ((parent->bounds.hx - parent->bounds.lx) / 2);
        nw_bounds.ly = parent->bounds.ly + ((parent->bounds.hy - parent->bounds.ly) / 2);
        nw_bounds.hy = parent->bounds.hy;
        
        parent->nw = quadtree_construct(nw_bounds, parent->depth+1);
        
        ne_bounds.lx = parent->bounds.lx + ((parent->bounds.hx - parent->bounds.lx) / 2);
        ne_bounds.hx = parent->bounds.hx;
        ne_bounds.ly = parent->bounds.ly + ((parent->bounds.hy - parent->bounds.ly) / 2);
        ne_bounds.hy = parent->bounds.hy;
        
        parent->ne = quadtree_construct(ne_bounds, parent->depth+1);
        
        sw_bounds.lx = parent->bounds.lx;
        sw_bounds.hx = parent->bounds.lx + ((parent->bounds.hx - parent->bounds.lx) / 2);
        sw_bounds.ly = parent->bounds.ly;
        sw_bounds.hy = parent->bounds.ly + ((parent->bounds.hy - parent->bounds.ly) / 2);
        
        parent->sw = quadtree_construct(sw_bounds, parent->depth+1);
        
        se_bounds.lx = parent->bounds.lx + ((parent->bounds.hx - parent->bounds.lx) / 2);
        se_bounds.hx = parent->bounds.hx;
        se_bounds.ly = parent->bounds.ly;
        se_bounds.hy = parent->bounds.ly + ((parent->bounds.hy - parent->bounds.ly) / 2);
        
        parent->se = quadtree_construct(se_bounds, parent->depth+1);
    }
}

static inline bool quadtree_insert(QuadTree* node, Point* p) {
    if (!pt_contained(node->bounds, *p)) {
        return false;
    }
    
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
        if (node->pts.size() < QT_MAX_PER_NODE || node->depth >= QT_MAX_DEPTH) {
            node->pts.push_back(p);
            return true;
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
            
            return quadtree_insert(node, p);
        }
    }
    
    return false;
}

#endif
