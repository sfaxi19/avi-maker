//
// Created by sfaxi19 on 26.04.17.
//

#include <vector>
#include "bmp.h"
#include "filter.h"
#include "algorithm"


Filter::~Filter() {

}

Filter::Filter() {

}

TRIPLEYCbCr **Filter::apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius, double sigma) {
    return nullptr;
}

int Filter::searchRadius(TRIPLEYCbCr **mrx, TRIPLEYCbCr **mrxNoize, int min, int max, BITMAPFILEHEADER bmFile,
                         BITMAPINFOHEADER bmInfo, const char *filepath, double sigma) {
    printf("Search radius(%s):\n", filepath);
    int h = bmInfo.biHeight;
    int w = bmInfo.biWidth;
    double maxPSNR = 0;
    int maxRadius = 0;
    double psnr;
    std::string path = filepath;
    system(("mkdir -p " + path).c_str());
    for (int r = min; r < max; r++) {
        TRIPLERGB **mrxf = (TRIPLERGB **) apply_filter(mrxNoize, h, w, r, sigma);
        psnr = getPSNR((TRIPLEBYTES **) mrx, (TRIPLEBYTES **) mrxf, r, r, h - r, w - r, COMPONENT_Y);
        if (maxPSNR < psnr) {
            maxPSNR = psnr;
            maxRadius = r;
        }
        printf("R = %3d\tPSNR[f,f] = %5.3f\n", r, psnr);
        std::string filename = path + "/radius_";//.bmp";
        save_component_to_files((TRIPLEBYTES **) mrxf, bmFile, bmInfo, COMPONENT_Y,
                                (filename + std::to_string(r) + ".bmp").c_str());
        for (int i = 0; i < h; i++) {
            delete[]mrxf[i];
        }
        delete[] mrxf;
    }
    return maxRadius;
}

//----------------------------
//--- MovingAveradgeFilter ---
//----------------------------
void MovingAveradgeFilter::moving_averadge(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int radius) {
    double pix_new = 0;
    for (int i = 0; i < (radius * 2 + 1); i++) {
        for (int j = 0; j < (radius * 2 + 1); j++) {
            pix_new += mrx[row + ((-1) * radius + i)][col + ((-1) * radius + j)].Y;
        }
    }
    pix_new = pix_new / pow((2 * radius + 1), 2);
    pix_new = round(pix_new);
    pix_new = clip((int) pix_new, 0, 255);
    mrxf[row][col].Y = (uint8_t) pix_new;
}

TRIPLEYCbCr **MovingAveradgeFilter::apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius, double sigma) {
    TRIPLEYCbCr **mrxf = new TRIPLEYCbCr *[h];
    for (int i = 0; i < h; i++) {
        mrxf[i] = new TRIPLEYCbCr[w];
        memset(mrxf[i], 0, sizeof(TRIPLEYCbCr) * w);
    }
    for (int i = radius; i < h - radius; i++) {
        for (int j = radius; j < w - radius; j++) {
            moving_averadge(&mrxf[0], mrx, i, j, radius);
        }
    }
    return mrxf;
}

//----------------------------
//-------- GauseFilter -------
//----------------------------
TRIPLEYCbCr **GauseFilter::apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius, double sigma) {
    TRIPLEYCbCr **mrxf = new TRIPLEYCbCr *[h];
    for (int i = 0; i < h; i++) {
        mrxf[i] = new TRIPLEYCbCr[w];
        memset(mrxf[i], 0, sizeof(TRIPLEYCbCr) * w);
    }
    double **floatWeights = new double *[radius * 2 + 1];
    int **weights = new int *[radius * 2 + 1];
    double sumWeight = 0;
    for (int i = 0; i < (radius * 2 + 1); i++) {
        floatWeights[i] = new double[radius * 2 + 1];
        for (int j = 0; j < (radius * 2 + 1); j++) {
            floatWeights[i][j] = exp(
                    (-1) * (pow(((-1) * radius + i), 2) + pow(((-1) * radius + j), 2)) / (2 * pow(sigma, 2)));
            sumWeight += floatWeights[i][j];
        }
    }
    int sum = 0;
    for (int i = 0; i < (radius * 2 + 1); i++) {
        weights[i] = new int[radius * 2 + 1];
        for (int j = 0; j < (radius * 2 + 1); j++) {
            weights[i][j] = (int) round(pow(2, 8) * floatWeights[i][j] / sumWeight);
            printf("%5d", weights[i][j]);
            sum += weights[i][j];
        }
        printf("\n");
    }
    //printf("sum: %d\n", sum);
    for (int i = radius; i < h - radius; i++) {
        for (int j = radius; j < w - radius; j++) {
            gause_filter(&mrxf[0], mrx, i, j, radius, weights);
        }
    }
    return mrxf;
}

void GauseFilter::gause_filter(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int radius, int **weights) {
    double pix_new = 0;
    double sumWeight = 0;
    for (int i = 0; i < (radius * 2 + 1); i++) {
        for (int j = 0; j < (radius * 2 + 1); j++) {
            sumWeight += weights[i][j];
            pix_new += weights[i][j] * mrx[row + ((-1) * radius + i)][col + ((-1) * radius + j)].Y;
        }
    }
    pix_new = pix_new / (sumWeight);
    pix_new = round(pix_new);
    pix_new = clip((int) pix_new, 0, 255);
    mrxf[row][col].Y = (uint8_t) pix_new;
}

double
GauseFilter::searchSigma(TRIPLEYCbCr **mrx, TRIPLEYCbCr **mrxNoize, double min, double max, BITMAPFILEHEADER bmFile,
                         BITMAPINFOHEADER bmInfo, const char *filepath, int radius) {
    printf("Gause filter. Search sigma:\n");
    int h = bmInfo.biHeight;
    int w = bmInfo.biWidth;
    double maxPSNR = 0;
    double maxSigma = 0;
    double psnr;
    std::string path = filepath;
    system(("mkdir -p " + path).c_str());
    for (double sigma = min; sigma < max; sigma += 0.1) {
        TRIPLERGB **mrxf = (TRIPLERGB **) apply_filter(mrxNoize, h, w, radius, sigma);
        psnr = getPSNR((TRIPLEBYTES **) mrx, (TRIPLEBYTES **) mrxf, radius, radius, h - radius, w - radius,
                       COMPONENT_Y);
        if (maxPSNR < psnr) {
            maxPSNR = psnr;
            maxSigma = sigma;
        }
        std::string filename = path + "/sigma_";//.bmp";
        save_component_to_files((TRIPLEBYTES **) mrxf, bmFile, bmInfo, COMPONENT_Y,
                                (filename + std::to_string(sigma) + ".bmp").c_str());
        for (int i = 0; i < h; i++) {
            delete[]mrxf[i];
        }
        delete[] mrxf;
        printf("R = %3d\tSigma = %3.2f\tPSNR[f,f] = %5.3f\n", radius, sigma, psnr);
    }
    return maxSigma;
}

//----------------------------
//------- MedianFilter -------
//----------------------------

void MedianFilter::median_filter(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int radius, double sigma) {
    double pix_new = 0;
    std::vector<uint8_t> medianSearch;
    for (int i = 0; i < (radius * 2 + 1); i++) {
        for (int j = 0; j < (radius * 2 + 1); j++) {
            medianSearch.push_back(mrx[row + ((-1) * radius + i)][col + ((-1) * radius + j)].Y);
        }
    }
    std::sort(medianSearch.begin(), medianSearch.end());
    std::vector<uint8_t>::iterator it;
    it = medianSearch.begin();
    pix_new = *(it + floor(medianSearch.size() / 2));
    //pix_new = clip((int) pix_new, 0, 255);
    mrxf[row][col].Y = (uint8_t) pix_new;
}

TRIPLEYCbCr **MedianFilter::apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius, double sigma) {
    TRIPLEYCbCr **mrxf = new TRIPLEYCbCr *[h];
    for (int i = 0; i < h; i++) {
        mrxf[i] = new TRIPLEYCbCr[w];
        memset(mrxf[i], 0, sizeof(TRIPLEYCbCr) * w);
    }
    for (int i = radius; i < h - radius; i++) {
        for (int j = radius; j < w - radius; j++) {
            median_filter(&mrxf[0], mrx, i, j, radius, sigma);
        }
    }
    return mrxf;
}


//----------------------------
//------- LaplasFilter -------
//----------------------------

void LaplasFilter::laplas_filter(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int radius) {
    double pix_new = 0;
    for (int i = 0; i < (radius * 2 + 1); i++) {
        for (int j = 0; j < (radius * 2 + 1); j++) {
            pix_new += this->weights[i][j] * mrx[row + ((-1) * radius + i)][col + ((-1) * radius + j)].Y;
        }
    }
    pix_new = round(pix_new);
    pix_new = clip((int) pix_new + ((this->adding != SYN) ? this->adding : mrx[row][col].Y), 0, 255);
    mrxf[row][col].Y = (uint8_t) pix_new;
}

TRIPLEYCbCr **LaplasFilter::apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius, double sigma) {
    TRIPLEYCbCr **mrxf = new TRIPLEYCbCr *[h];
    for (int i = 0; i < h; i++) {
        mrxf[i] = new TRIPLEYCbCr[w];
        memset(mrxf[i], 0, sizeof(TRIPLEYCbCr) * w);
    }
    for (int i = radius; i < h - radius; i++) {
        for (int j = radius; j < w - radius; j++) {
            laplas_filter(&mrxf[0], mrx, i, j, radius);
        }
    }
    return mrxf;
}

LaplasFilter::LaplasFilter(int weight[3][3], int adding) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            this->weights[i][j] = weight[i][j];
        }
    }
    this->adding = adding;
}

void LaplasFilter::setAdding(int adding) {
    this->adding = adding;
}

void LaplasFilter::setAlpha(double alpha) {
    this->weights[1][1] = alpha + 4;
}

//----------------------------
//------- SobelFilter --------
//----------------------------
void SobelFilter::sobel_filter(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int thr) {
    int vertWeights[3][3] = {{-1, 0, 1},
                             {-2, 0, 2},
                             {-1, 0, 1}};
    int horWeights[3][3] = {{1,  2,  1},
                            {0,  0,  0},
                            {-1, -2, -1}};
    int radius = 1;
    int horResp = 0;
    int vertResp = 0;
    double pix_new = 0;
    for (int i = 0; i < (radius * 2 + 1); i++) {
        for (int j = 0; j < (radius * 2 + 1); j++) {
            vertResp += vertWeights[i][j] * mrx[row + ((-1) * radius + i)][col + ((-1) * radius + j)].Y;
            horResp += horWeights[i][j] * mrx[row + ((-1) * radius + i)][col + ((-1) * radius + j)].Y;
        }
    }
    double theta = atan2(vertResp, horResp);
    printf("%d : %d = %f\n", vertResp, horResp, theta);
    if ((theta > 0) && (theta < M_PI / 2)) {
        mrxMap[row][col].red = 255;
        mrxMap[row][col].green = mrxMap[row][col].blue = 0;
    }
    if ((theta > M_PI / 2) && (theta < M_PI)) {
        mrxMap[row][col].red = mrxMap[row][col].green = mrxMap[row][col].blue = 255;
    }
    if ((theta < 0) && (theta > ((-1) * (M_PI / 2)))) {
        mrxMap[row][col].green = 255;
        mrxMap[row][col].red = mrxMap[row][col].blue = 0;
    }
    if ((theta < ((-1) * (M_PI / 2))) && (theta > ((-1) * M_PI))) {
        mrxMap[row][col].blue = 255;
        mrxMap[row][col].green = mrxMap[row][col].red = 0;
    }
    double x = sqrt(pow(horResp, 2) + pow(vertResp, 2));
    pix_new = (x > thr) ? 255 : 0;
    mrxf[row][col].Y = (uint8_t) pix_new;
}

TRIPLEYCbCr **SobelFilter::apply_filter(TRIPLEYCbCr **mrx, BITMAPFILEHEADER bmFile,
                                        BITMAPINFOHEADER bmInfo, int thr) {
    int h = bmInfo.biHeight;
    int w = bmInfo.biWidth;
    int radius = 1;
    TRIPLEYCbCr **mrxf = new TRIPLEYCbCr *[h];
    mrxMap = new TRIPLERGB *[h];
    for (int i = 0; i < h; i++) {
        mrxf[i] = new TRIPLEYCbCr[w];
        mrxMap[i] = new TRIPLERGB[w];
        memset(mrxf[i], 0, sizeof(TRIPLEYCbCr) * w);
        memset(mrxMap[i], 0, sizeof(TRIPLERGB) * w);
    }
    for (int i = radius; i < h - radius; i++) {
        for (int j = radius; j < w - radius; j++) {
            sobel_filter(&mrxf[0], mrx, i, j, thr);
        }
    }
    saveBMPFile(bmFile, bmInfo, mrxMap, "lab2/contour/sobelMap.bmp");
    return mrxf;
}

int SobelFilter::searchThr(TRIPLEYCbCr **mrx, int min, int max, int step, BITMAPFILEHEADER bmFile,
                           BITMAPINFOHEADER bmInfo, const char *filepath) {
    TRIPLEYCbCr **mrxSobelFilter;
    std::string command = "mkdir -p ";
    command.append(filepath);
    system(command.c_str());
    std::string filename = filepath;
    for (int i = min; i < max; i += step) {
        mrxSobelFilter = apply_filter(mrx, bmFile, bmInfo, i);
        save_component_to_files((TRIPLEBYTES **) mrxSobelFilter, bmFile, bmInfo, COMPONENT_Y,
                                (filename + "thr_" + std::to_string(i) + ".bmp").c_str());
    }
    return 0;
}
