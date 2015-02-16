//
//  PBM.h
//  ChurchillNavigationChallenge
//
//  Created by Eric Campbell on 2/12/15.
//  Copyright (c) 2015 ECC. All rights reserved.
//
//
//
//  Simple struct for drawing and saving a PPM image
//  http://en.wikipedia.org/wiki/Netppm_format#PPM_example

#ifndef ChurchillNavigationChallenge_ppm_h
#define ChurchillNavigationChallenge_ppm_h
#include <fstream>

// RGB 0-255 pixel struct
struct ppm {
public:
    struct pixel {
        unsigned char r;
        unsigned g;
        unsigned b;
        pixel() : r(0), g(0), b(0) {}
        pixel(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {
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
    
    // ppm constructor, NxN pixel vector
    ppm(int w, int h) : width(w), height(h) { pixels.resize(width*height, ppm::pixel(0, 0, 0)); }
    
};

ppm::pixel inline operator/(const ppm::pixel &p1, const int divisor) {
    if (divisor <= 0) return ppm::pixel(p1);
    return ppm::pixel(p1.r / divisor, p1.g / divisor, p1.b / divisor);
}

ppm::pixel inline operator-(const ppm::pixel &p1, const int sub) {
    return ppm::pixel(p1.r - sub, p1.g - sub, p1.b - sub);
}

ppm::pixel inline operator+(const ppm::pixel &p1, const int add) {
    return ppm::pixel(p1.r + add, p1.g + add, p1.b + add);
}

bool operator==(const ppm::pixel& p1, const ppm::pixel& p2) {
    return p1.r == p2.r && p1.g == p2.g && p1.b == p2.b;
}

bool inline operator!=(const ppm::pixel& p1, const ppm::pixel& p2) {
    return !(p1 == p2);
}

const ppm::pixel ppm::white = ppm::pixel(255,255,255);
const ppm::pixel ppm::black = ppm::pixel(0,0,0);
const ppm::pixel ppm::blue = ppm::pixel(0,0,255);
const ppm::pixel ppm::red = ppm::pixel(255,0,0);
const ppm::pixel ppm::yellow = ppm::pixel(252,255,0);
const ppm::pixel ppm::green = ppm::pixel(90,255,0);

// Get the position within the NxN array for this pixel
static inline int ppm_get_pos(ppm& img, int x, int y) {
    int pos = (y * img.width) + x;
    if (pos >= img.width*img.height) pos = img.width*img.height - 1;
    return pos;
}

// Draw a horizontal line on the image at the y pixel from x start to end
// Arguments
//    img - referene to ppm image to draw on
//    y - y axis to draw along
//    start - x position to start at
//    end - x position to end at
//    c - draw color
//    override - draw over whatever pixels have already been drawn
static inline void ppm_draw_line_horiz(ppm& img, int y, int start, int end, ppm::pixel c, bool override=false) {
    // out of bounds error checking
    if (y < 0) y = 0;
    if (y > img.height-1) y = img.height-1;
    if (start < 0) start = 0;
    if (start > img.width-1) start = img.width - 1;
    if (end < 0) end = 0;
    if (end > img.width - 1) end = img.width - 1;
    
    for (int x = start; x < end; x++) {
        int pos = ppm_get_pos(img, x, y);
        if (img.pixels[pos].r == 0 || override==true)
            img.pixels[pos] = c;
    }
}

// Draw a vertical line on the image at the x pixel from y start to end
// Arguments
//    img - referene to ppm image to draw on
//    x - x axis to draw along
//    start - y position to start at
//    end - y position to end at
//    c - draw color
//    override - draw over whatever pixels have already been drawn
static inline void ppm_draw_line_vert(ppm& img, int x, int start, int end, ppm::pixel c, bool override=false) {
    // out of bounds error checking
    if (x < 0) x = 0;
    if (x > img.width-1) x = img.width-1;
    if (start < 0) start = 0;
    if (start > img.height-1) start = img.height - 1;
    if (end < 0) end = 0;
    if (end > img.height - 1) end = img.height - 1;
    
    for (int y = start; y < end; y++) {
        int pos = ppm_get_pos(img, x, y);
        if (img.pixels[pos].r == 0 || override==true)
            img.pixels[pos] = c;
    }
}

// Draw a rectangle(unfilled) on the image
static void ppm_draw_rect(ppm& img, Rect& rect, ppm::pixel c, bool override=false) {
    ppm_draw_line_horiz(img, rect.ly, rect.lx, rect.hx, c, override);
    ppm_draw_line_horiz(img, rect.hy, rect.lx, rect.hx, c, override);
    ppm_draw_line_vert(img, (int)rect.lx, (int)rect.ly, (int)rect.hy, c, override);
    ppm_draw_line_vert(img, (int)rect.hx, (int)rect.ly, (int)rect.hy, c, override);
}

// Draw a rectangle(filled) on the image
static inline void ppm_draw_rect_fill(ppm& img, Rect& rect, ppm::pixel c) {
    for (int i = rect.lx; i < rect.hx; i++) {
        ppm_draw_line_vert(img, i, (int)rect.ly, (int)rect.hy, c, true);
    }
    ppm_draw_rect(img, rect, ppm::pixel(c.r + 20, c.g + 20, c.b + 20), true);
}

// Set the RGB value for an individual pixel
static inline void ppm_set_pt(ppm& img, int x, int y, ppm::pixel c) {
    if (x < 0) x = 0;
    if (x > img.width-1) x = img.width-1;
    if (y < 0) y = 0;
    if (y > img.height-1) y = img.height-1;
    int pos = ppm_get_pos(img, x, y);
    if (img.pixels[pos] != ppm::yellow)
        img.pixels[pos] = c;
}

// Helper method for drawing out QuadTree node boundaries. PPM image should be the same size
// as the root node of the quadtree, or point values should be converted to match the scale
// This method is completely optional and not associated with a PPM image
static void ppm_draw_quadtree(ppm& img, QuadTree* tree, ppm::pixel point_color) {
    
    // Draw all points within this leaf
    for (int i = 0; i < tree->pts.size(); i++) {
        ppm_set_pt(img, tree->pts[i]->x, tree->pts[i]->y, point_color);
    }
    
    // recursively draw all 4 sub nodes of this tree node
    if (tree->nw != 0) {
        ppm_draw_quadtree(img, tree->nw, point_color);
        ppm_draw_quadtree(img, tree->ne, point_color);
        ppm_draw_quadtree(img, tree->sw, point_color);
        ppm_draw_quadtree(img, tree->se, point_color);
    }
    
    // draw the boundary rectangle last so it overrides points and child nodes
    ppm_draw_rect(img, tree->bounds, ppm::white - (tree->depth * 30), true);
}

// Helper method for drwaing out KdTree node boundaries. PPM image should be the same size
// as the root node of the KdTree, or point values should be converted to match the scale
// This method is completely optional and not associated with a PPM image
static void ppm_draw_kdtree(ppm& img, KdTree* tree, ppm::pixel point_color) {
    
    // Draw all points within this leaf
    for (int i = 1; i < tree->points.size(); i++) {
        ppm_set_pt(img, tree->points[i]->x, tree->points[i]->y, point_color);
    }
    
    // Recursively draw the left subdivision
    if (tree->left != 0)
        ppm_draw_kdtree(img, tree->left, point_color);
    
    // Recursively draw the right subdivision
    if (tree->right != 0)
        ppm_draw_kdtree(img, tree->right, point_color);
    
    // Draw the median axis lines, depending on the depth (even - X, odd - Y)
    if (tree->points.size() != 0) {
        if (tree->depth % 2 == 0)
            ppm_draw_line_vert(img, tree->points[0]->x, tree->bounds.ly, tree->bounds.hy, ppm::white - (tree->depth * 15), true);
        else
            ppm_draw_line_horiz(img, tree->points[0]->y, tree->bounds.lx, tree->bounds.hx, ppm::white - (tree->depth * 15), true);
        
//        ppm_draw_rect(img, tree->bounds, ppm::red, true);
    }
    
    // Draw the median point of this node in yellow
    if (tree->points.size() > 0) {
        for (int i = 0; i < 1; i++) {
            ppm_set_pt(img, tree->points[i]->x, tree->points[i]->y, ppm::yellow);
        }
    }
}

static void ppm_write_pbm(ppm& img, std::string outfile) {
    // Open output stream
    std::ofstream ofs (outfile, std::ofstream::out);
    
    // Write the header
    // [Version]
    // [Width] [Height]
    // [Max Color]
    ofs << img.version << '\n';
    ofs << "#" << img.comment << '\n';
    ofs << img.width << " " << img.height << '\n';
    ofs << "255" << '\n';
    
    // Write pixels
    // Each row should have a newline character at the end;
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            int pos = y * img.width + x;
            ofs << (int)img.pixels[pos].r << " " << (int)img.pixels[pos].g << " " << (int)img.pixels[pos].b << " ";
        }
        ofs << '\n';
    }
    
    ofs.close();
}

#endif
