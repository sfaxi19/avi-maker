//
// Created by sfaxi19 on 26.04.17.
//

#ifndef BMP_FILTER_H
#define BMP_FILTER_H
#define SYN (-1)

#include "bmp.h"

class Filter {
public:
    Filter();

    virtual TRIPLEYCbCr **apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius, double sigma = 0);


    int searchRadius(TRIPLEYCbCr **mrx, TRIPLEYCbCr **mrxNoize, int min, int max, BITMAPFILEHEADER bmFile,
                     BITMAPINFOHEADER bmInfo, const char *searchName, double sigma = 0);

    virtual ~Filter();
};

class MovingAveradgeFilter : public Filter {
private:
    void moving_averadge(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int radius);

public:
    TRIPLEYCbCr **apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius, double sigma = 0) override;
};

class GauseFilter : public Filter {
private:
    void gause_filter(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int radius, int **weights);

public:
    TRIPLEYCbCr **apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius, double sigma) override;

    double searchSigma(TRIPLEYCbCr **mrx, TRIPLEYCbCr **mrxNoize, double min, double max, BITMAPFILEHEADER bmFile,
                       BITMAPINFOHEADER bmInfo, const char *filepath, int radius);
};

class SobelFilter {
private:
    TRIPLERGB **mrxMap;

    void sobel_filter(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int thr);

public:
    TRIPLEYCbCr **
    apply_filter(TRIPLEYCbCr **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo, int thr);

    int searchThr(TRIPLEYCbCr **mrx, int min, int max, int step, BITMAPFILEHEADER bmFile,
                  BITMAPINFOHEADER bmInfo, const char *filepath);
};

class MedianFilter : public Filter {
private:
    void median_filter(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int radius, double sigma);

public:
    TRIPLEYCbCr **apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius, double sigma = 0) override;
};

class LaplasFilter : public Filter {

private:
    double weights[3][3];
    int adding = 0;

    void laplas_filter(TRIPLEYCbCr **mrxf, TRIPLEYCbCr **mrx, int row, int col, int radius);

public:
    LaplasFilter(int weight[][3], int adding = 0);

    TRIPLEYCbCr **apply_filter(TRIPLEYCbCr **mrx, int h, int w, int radius = 1, double sigma = 0) override;

    void setAdding(int adding);

    void setAlpha(double alpha);

};

#endif //BMP_FILTER_H
