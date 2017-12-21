#ifndef BMP_H
#define BMP_H

#include <zconf.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "bmp_headers.hpp"


const int COMPONENT_RED = 0;
const int COMPONENT_GREEN = 1;
const int COMPONENT_BLUE = 2;

const int COMPONENT_Y = 0;
const int COMPONENT_CB = 1;
const int COMPONENT_CR = 2;

const int COMPONENT_A = 0;
const int COMPONENT_B = 1;
const int COMPONENT_C = 2;

const int DIFF_RULE_1 = 0;
const int DIFF_RULE_2 = 1;
const int DIFF_RULE_3 = 2;
const int DIFF_RULE_4 = 3;

const int CHART_3D_START_HEIGHT = -10;
const int CHART_3D_COUNT_HEIGHT = 5;
const int CHART_3D_STEP_HEIGHT = 5;
const int CHART_3D_START_WIDTH = 0;
const int CHART_3D_STEP_WIDTH = 8;
const int FORMAT_RGB = 0;
const int FORMAT_YCBCR = 1;
const int BYTE_SIZE = 256;

const int VERTICAL = 0;
const int HORIZONTAL = 1;

const int DECIMATION_EJECT = 0;
const int DECIMATION_MEAN = 1;

void printHeader(BITMAPFILEHEADER, BITMAPINFOHEADER);

uint8_t clip(int value, uint8_t min, uint8_t max);

void printMatrix(TRIPLERGB **mrx, int h, int w);

void saveBMPFile(BITMAPFILEHEADER, BITMAPINFOHEADER, TRIPLERGB **, const char *filepath);

void save_component_to_files(TRIPLEBYTES **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo, int component,
                             const char *filename);

void save_components_to_files(TRIPLERGB **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo, int format,
                              const char *filepath1, const char *filepath2, const char *filepath3);

void saveXYCSVFile(double **mrx, const int height, const int width, std::string format);

void saveHistCSVFile(int *mrx, const int start, const int width, const char *filename);

u_char getComponent(TRIPLEBYTES **mrx, int i, int j, int component);

//Mathematics functions
double getCorrelationCoefficient(TRIPLERGB **mrx, TRIPLERGB **mrx2, int h, int w, int componentA, int componentB,
                                 double ExpA, double ExpB);

//double getCorrelationCoefficient(TRIPLERGB **mrx1, TRIPLERGB **mrx2, int h, int w, int component1, int component2);
double getCorrelationCoefficient3d(TRIPLERGB **mrx, int h, int w, int component, int y = 0, int x = 0);

double getMeanSquareDeviation(TRIPLERGB **mrx, int h, int w, int component, double E);

double getStandartDeviation(TRIPLERGB **mrx, int h, int w, int component, double E);

double getExpection(TRIPLERGB **mrx, int h, int w, int component, int y = 0, int x = 0);

double getPSNR(TRIPLEBYTES **mrx1, TRIPLEBYTES **mrx2, int y, int x, int h, int w, int component);

TRIPLERGB **loadBMPFile(BITMAPFILEHEADER *bmFile, BITMAPINFOHEADER *bmInfo, const char *filepath);

double **getChartsForComponent(TRIPLERGB **mrx, int h, int w, int component);

void checkCorrelationProperties(TRIPLERGB **mrx, int h, int w, int format = FORMAT_RGB);

TRIPLERGB **recovery(BITMAPFILEHEADER bmFile,
                     BITMAPINFOHEADER bmInfo,
                     TRIPLERGB **mrx, int k, const char *filepath);

TRIPLERGB **decimationMean(BITMAPFILEHEADER bmFile,
                           BITMAPINFOHEADER bmInfo,
                           TRIPLERGB **mrx, int k, const char *filepath);

TRIPLERGB **decimationEjection(BITMAPFILEHEADER bmFile,
                               BITMAPINFOHEADER bmInfo,
                               TRIPLERGB **mrx, int k, const char *filepath);

TRIPLERGB **YCbCr2RGB(TRIPLERGB **mrxY, TRIPLERGB **mrxCb, TRIPLERGB **mrxCr, int h, int w);

TRIPLEYCbCr **RGB2YCbCr(TRIPLERGB **mrxRGB, size_t h, size_t w);

TRIPLERGB **Y2RGB(TRIPLEYCbCr **mrx, size_t h, size_t w);

TRIPLERGB **decimationAndRecovering(TRIPLERGB **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo,
                                    int decimation, int k, const char *filenameDecimate, const char *filenameRecover);

void check_PSNR_for_recov(TRIPLERGB **mrxRGB, TRIPLERGB **mrxY, TRIPLERGB **mrxCb, TRIPLERGB **mrxCr,
                          const char *filenameCb, const char *filenameCr, const char *filenameRGB);

int *getHistFreqFromTriple(TRIPLERGB **mrx, int h, int w, int component, const char *fpath, const char *fname);

double getEntropy(const int freq[], int N);


#endif //BMP_H
