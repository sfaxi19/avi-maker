//
// Created by sfaxi19 on 08.11.17.
//

#include "../bmp_lib/bmp.h"
#include "motion_compensation.h"
#include "cmath"

void mc::subtract_block(TRIPLEYCbCr **base, TRIPLEYCbCr **target, TRIPLEYCbCr **out, mc::block_info block, mc::vect v) {
    for (int i = 0; i < block.height; i++) {
        for (int j = 0; j < block.width; j++) {
            out[block.y + i][block.x + j].Y = clip(
                    target[block.y + i][block.x + j].Y - base[block.y + v.y + i][block.x + v.x + j].Y,
                    0,
                    255);
        }
    }
}

uint32_t mc::sumAbsDiffFrame(TRIPLERGB *base, TRIPLERGB *target, int height, int width) {
    uint32_t sum = 0;
    for (size_t i = 0; i < height * width; i++) {
        TRIPLEYCbCr tarPix = RGB2YCbCrPix(target[i]);
        TRIPLEYCbCr basePix = RGB2YCbCrPix(base[i]);
        sum += abs(tarPix.Y - basePix.Y);
    }
    return sum;
}

uint32_t mc::sumAbsDiff(TRIPLEYCbCr **base, TRIPLEYCbCr **target, mc::pos basePos, mc::block_info tarBlock) {
    uint32_t sum = 0;
    for (size_t i = 0; i < tarBlock.height; i++) {
        for (size_t j = 0; j < tarBlock.width; j++) {
            sum += abs(target[tarBlock.y + i][tarBlock.x + j].Y - base[basePos.y + i][basePos.x + j].Y);
        }
    }
    return sum;
}

uint32_t errorEnergy(TRIPLEYCbCr **base, TRIPLEYCbCr **target, mc::block_info bBlock, mc::block_info tarBlock) {
    uint32_t sum = 0;
    for (size_t i = 0; i < tarBlock.height; i++) {
        for (size_t j = 0; j < tarBlock.width; j++) {
            sum += pow(target[tarBlock.y + i][tarBlock.x + j].Y - base[bBlock.y + i][bBlock.x + j].Y, 2);
        }
    }
    return sum;
}

int findMinFromArray(const uint32_t array[5]) {
    int min = INT32_MAX;
    int min_index = -1;
    for (int i = 0; i < 5; i++) {
        if (array[i] < min) {
            min = array[i];
            min_index = i;
        }
    }
    return min_index;
}

mc::vect
mc::logarithmicSearch(TRIPLEYCbCr **bFrame, TRIPLEYCbCr **tarFrame, size_t h, size_t w, mc::block_info tarBlock) {
    mc::logblock bBlock(tarBlock, 5, h, w);
//    std::cout << "=====================================\n";
//    std::cout << "=====================================\n";
//    std::cout << "=====================================\n";
//    std::cout << "tarBlock:   " << tarBlock.toString();
//    std::cout << "bBlock:     " << bBlock.toString();
//    std::cout << "Pos_top:    " << bBlock.top().toString();
//    std::cout << "Pos_left:   " << bBlock.left().toString();
//    std::cout << "Pos_right:  " << bBlock.right().toString();
//    std::cout << "Pos_bottom: " << bBlock.bottom().toString();
//    std::cout << "Pos_mid:    " << bBlock.mid().toString();
    uint32_t res[5];
    while (bBlock.n > 1) {
        res[0] = sumAbsDiff(bFrame, tarFrame, bBlock.mid(), tarBlock);
        res[1] = sumAbsDiff(bFrame, tarFrame, bBlock.left(), tarBlock);
        res[2] = sumAbsDiff(bFrame, tarFrame, bBlock.right(), tarBlock);
        res[3] = sumAbsDiff(bFrame, tarFrame, bBlock.top(), tarBlock);
        res[4] = sumAbsDiff(bFrame, tarFrame, bBlock.bottom(), tarBlock);
        int min = findMinFromArray(res);
        if (min == 0) bBlock.dev_n();
        if (min == 1) bBlock.gotoLeft();
        if (min == 2) bBlock.gotoRight();
        if (min == 3) bBlock.gotoTop();
        if (min == 4) bBlock.gotoBottom();
        //printf("sum_abs [x: %d ; y: %d] = %d\n", v.x, v.y, resources);
        //resources = errorEnergy(base, target, block, v);
        //printf("energy [x: %d ; y: %d] = %d\n", v.x, v.y, resources);
    }
    return {(int) bBlock.x - (int) tarBlock.x, (int) bBlock.y - (int) tarBlock.y};
}

