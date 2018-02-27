//
// Created by sfaxi19 on 08.11.17.
//

#ifndef MULTITECH_MOTION_COMPENSATION_H
#define MULTITECH_MOTION_COMPENSATION_H

//#include "AVIMaker/AVIMaker.h"

#include "iostream"
#include "../bmp_lib/bmp_headers.hpp"

namespace mc {

    struct vect {
        vect(int x, int y) : x(x), y(y) {};
        int x = 0;
        int y = 0;

        std::string toString() {
            std::string str = "[" + std::to_string(x) + " ; " + std::to_string(y) + "]\n";
            return str;
        }
    };

    struct pos {
        pos() {}

        pos(size_t x, size_t y)
                : x(x), y(y) {};
        size_t x = 0;
        size_t y = 0;

        std::string toString() {
            std::string str = "[" + std::to_string(x) + " ; " + std::to_string(y) + "]\n";
            return str;
        }
    };

    struct block_info : pos {
        block_info(size_t x, size_t y, size_t height, size_t width) : pos(x, y), height(height), width(width) {};
        size_t width = 0;
        size_t height = 0;

        std::string toString() {
            std::string str;
            str = "[" + std::to_string(x) + " ; " + std::to_string(y) + "] : " + std::to_string(height) + "x" +
                  std::to_string(width) + "\n";
            return str;
        }
    };

    struct logblock : block_info {
        size_t n = 0;
        size_t height_max = 0;
        size_t width_max = 0;

        logblock(block_info &b, size_t n, size_t height_max, size_t width_max) : block_info(b) {
            this->n = n;
            this->height_max = height_max - this->height;
            this->width_max = width_max - this->width;
        }

        logblock(size_t x, size_t y, size_t height, size_t width) : block_info(x, y, height, width) {};

        void dev_n() { n /= 2; };

        pos top() {
            return {x, (y + n >= height_max) ? height_max - 1 : y + n};
        }

        pos bottom() {
            return {x, (y < n) ? 0 : y - n};
        }

        pos left() {
            return {(x < n) ? 0 : x - n, y};
        }

        pos right() {
            return {(x + n >= width_max) ? width_max - 1 : x + n, y};
        }

        pos mid() {
            return {x, y};
        };

        logblock gotoLeft() { return new_pos(left()); }

        logblock gotoRight() { return new_pos(right()); }

        logblock gotoTop() { return new_pos(top()); }

        logblock gotoBottom() { return new_pos(bottom()); }


        logblock new_pos(size_t x, size_t y) {
            this->x = x;
            this->y = y;
            return *this;
        };

        logblock new_pos(pos p) { return new_pos(p.x, p.y); }
    };

    vect logarithmicSearch(TRIPLEYCbCr **base, TRIPLEYCbCr **target, size_t h, size_t w, block_info block);

    uint32_t sumAbsDiff(TRIPLEYCbCr **base, TRIPLEYCbCr **target, mc::pos basePos, mc::block_info tarBlock);

    uint32_t sumAbsDiffFrame(TRIPLERGB *base, TRIPLERGB *target, int height, int width);

    void subtract_block(TRIPLEYCbCr **base, TRIPLEYCbCr **target, TRIPLEYCbCr **out, mc::block_info block, mc::vect v);

    void addition_block(TRIPLEYCbCr **base, TRIPLEYCbCr **target, TRIPLEYCbCr **out, mc::block_info block, mc::vect v);
}
#endif //MULTITECH_MOTION_COMPENSATION_H
