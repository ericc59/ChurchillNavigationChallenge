//
//  PBM.h
//  ChurchillNavigationChallenge
//
//  Created by Eric Campbell on 2/12/15.
//  Copyright (c) 2015 ECC. All rights reserved.
//

#ifndef ChurchillNavigationChallenge_PBM_h
#define ChurchillNavigationChallenge_PBM_h
#include <fstream>

struct pbm {
public:
    struct pixel {
        int r;
        int g;
        int b;
        pixel() : r(0), g(0), b(0) {}
        pixel(int r, int g, int b) : r(r), g(g), b(b) {
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;
        }
        
        
        friend pixel operator/(const pixel &p1, const int divisor);
        friend pixel operator-(const pixel &p1, const int sub);
        friend pixel operator+(const pixel &p1, const int add);
        friend bool operator==(const pixel &p1, const pixel& p2);
        friend bool operator!=(const pixel &p1, const pixel& p2);
    };
    
    const static pixel white;
    const static pixel black;
    const static pixel blue;
    const static pixel red;
    const static pixel yellow;
    const static pixel green;
    
    std::string version = "P3";
    std::string comment = "yesssssss";
    int width;
    int height;
    std::vector<pixel> pixels;
    
    pbm(int w, int h) : width(w), height(h) { pixels.resize(width*height, pbm::pixel(0, 0, 0)); }
    
};

pbm::pixel operator/(const pbm::pixel &p1, const int divisor) {
    if (divisor <= 0) return pbm::pixel(p1);
    return pbm::pixel(p1.r / divisor, p1.g / divisor, p1.b / divisor);
}

pbm::pixel operator-(const pbm::pixel &p1, const int sub) {
    return pbm::pixel(p1.r - sub, p1.g - sub, p1.b - sub);
}

pbm::pixel operator+(const pbm::pixel &p1, const int add) {
    return pbm::pixel(p1.r + add, p1.g + add, p1.b + add);
}

bool operator==(const pbm::pixel& p1, const pbm::pixel& p2) {
    return p1.r == p2.r && p1.g == p2.g && p1.b == p2.b;
}

bool operator!=(const pbm::pixel& p1, const pbm::pixel& p2) {
    return !(p1 == p2);
}

const pbm::pixel pbm::white = pbm::pixel(255,255,255);
const pbm::pixel pbm::black = pbm::pixel(0,0,0);
const pbm::pixel pbm::blue = pbm::pixel(0,0,255);
const pbm::pixel pbm::red = pbm::pixel(255,0,0);
const pbm::pixel pbm::yellow = pbm::pixel(252,255,0);
const pbm::pixel pbm::green = pbm::pixel(90,255,0);

static int pbm_get_pos(pbm& img, int x, int y) {
    int pos = (y * img.width) + x;
    if (pos >= img.width*img.height) pos = img.width*img.height - 1;
    return pos;
}

static void pbm_draw_line_horiz(pbm& img, int y, int start, int end, pbm::pixel c, bool override=false) {
    if (y < 0) y = 0;
    if (y > img.height-1) y = img.height-1;
    if (start < 0) start = 0;
    if (start > img.width-1) start = img.width - 1;
    if (end < 0) end = 0;
    if (end > img.width - 1) end = img.width - 1;
    
    for (int x = start; x < end; x++) {
        int pos = pbm_get_pos(img, x, y);
        if (img.pixels[pos].r == 0 || override==true)
            img.pixels[pos] = c;
    }
}

static void pbm_draw_line_vert(pbm& img, int x, int start, int end, pbm::pixel c, bool override=false) {
    if (x < 0) x = 0;
    if (x > img.width-1) x = img.width-1;
    if (start < 0) start = 0;
    if (start > img.height-1) start = img.height - 1;
    if (end < 0) end = 0;
    if (end > img.height - 1) end = img.height - 1;
    
    
    for (int y = start; y < end; y++) {
        int pos = pbm_get_pos(img, x, y);
        if (img.pixels[pos].r == 0 || override==true)
            img.pixels[pos] = c;
    }
}

static void pbm_draw_rect(pbm& img, Rect& rect, pbm::pixel c, bool override=false) {
    pbm_draw_line_horiz(img, rect.ly, rect.lx, rect.hx, c, override);
    pbm_draw_line_horiz(img, rect.hy, rect.lx, rect.hx, c, override);
    pbm_draw_line_vert(img, (int)rect.lx, (int)rect.ly, (int)rect.hy, c, override);
    pbm_draw_line_vert(img, (int)rect.hx, (int)rect.ly, (int)rect.hy, c, override);
}

static void pbm_draw_rect_fill(pbm& img, Rect& rect, pbm::pixel c) {
    for (int i = rect.lx; i < rect.hx; i++) {
        pbm_draw_line_vert(img, i, (int)rect.ly, (int)rect.hy, c, true);
    }
    pbm_draw_rect(img, rect, pbm::pixel(c.r + 20, c.g + 20, c.b + 20), true);
}

static void pbm_set_pt(pbm& img, int x, int y, pbm::pixel c) {
    if (x < 0) x = 0;
    if (x > img.width-1) x = img.width-1;
    if (y < 0) y = 0;
    if (y > img.height-1) y = img.height-1;
    int pos = pbm_get_pos(img, x, y);
    if (img.pixels[pos] != pbm::yellow)
        img.pixels[pos] = c;
}

static void pbm_incr_pt(pbm& img, int x, int y, pbm::pixel c) {
    int pos = pbm_get_pos(img, x, y);
    
}

static void pbm_draw_quadtree(pbm& img, QuadTree* tree, pbm::pixel point_color) {
    for (int i = 0; i < tree->pts.size(); i++) {
        pbm_set_pt(img, tree->pts[i]->x, tree->pts[i]->y, point_color);
    }
    
    if (tree->nw != 0) {
        pbm_draw_quadtree(img, tree->nw, point_color);
        pbm_draw_quadtree(img, tree->ne, point_color);
        pbm_draw_quadtree(img, tree->sw, point_color);
        pbm_draw_quadtree(img, tree->se, point_color);
    }
    
    pbm_draw_rect(img, tree->bounds, pbm::white - (tree->depth * 30), true);
}

static void pbm_draw_kdtree(pbm& img, KdTree* tree, pbm::pixel point_color) {
    for (int i = 1; i < tree->points.size(); i++) {
        pbm_set_pt(img, tree->points[i]->x, tree->points[i]->y, point_color);
    }
    
    if (tree->left != 0)
        pbm_draw_kdtree(img, tree->left, point_color);
    
    if (tree->right != 0)
        pbm_draw_kdtree(img, tree->right, point_color);
    
    if (tree->points.size() != 0) {
        if (tree->depth % 2 == 0)
            pbm_draw_line_vert(img, tree->points[0]->x, tree->bounds.ly, tree->bounds.hy, pbm::white - (tree->depth * 15), true);
        else
            pbm_draw_line_horiz(img, tree->points[0]->y, tree->bounds.lx, tree->bounds.hx, pbm::white - (tree->depth * 15), true);
        
//        pbm_draw_rect(img, tree->bounds, pbm::red, true);
    }
    
    if (tree->points.size() > 0)
        for (int i = 0; i < 1; i++) {
            pbm_set_pt(img, tree->points[i]->x, tree->points[i]->y, pbm::yellow);
        }
    
}

static int pbm_fsel( int a, int b, int c ) {
    return a >= 0 ? b : c;
}

static inline int pbm_clamp ( int a, int min, int max )
{
    a = pbm_fsel( a - min , a, min );
    return pbm_fsel( a - max, max, a );
}

static void pbm_write_pbm(pbm& img, std::string outfile) {
    std::ofstream ofs (outfile, std::ofstream::out);
    
    ofs << img.version << "\n";
    ofs << img.width << " " << img.height << "\n";
    ofs << "255\n";
    
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            int pos = y * img.width + x;
            img.pixels[pos].r = pbm_clamp(img.pixels[pos].r, 0, 255);
            img.pixels[pos].g = pbm_clamp(img.pixels[pos].g, 0, 255);
            img.pixels[pos].b = pbm_clamp(img.pixels[pos].b, 0, 255);
        
            ofs << img.pixels[pos].r << " " << img.pixels[pos].g << " " << img.pixels[pos].b << " ";
        }
        ofs << "\n";
    }
    
    ofs.close();
}

#endif
