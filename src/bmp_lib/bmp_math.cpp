//
// Created by sfaxi19 on 05.03.17.
//


#include "bmp.h"

double getCorrelationCoefficient(TRIPLERGB **mrx, TRIPLERGB **mrx2, int h, int w, int componentA, int componentB,
                                 double ExpA, double ExpB) {
    double count = 0;
    double count1 = 0;
    double count2 = 0;
    double countSqr1 = 0;
    double countSqr2 = 0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            count1 = (double) getComponent((TRIPLEBYTES **) mrx, i, j, componentA) - ExpA;
            count2 = (double) getComponent((TRIPLEBYTES **) mrx2, i, j, componentB) - ExpB;
            count += count1 * count2;
            countSqr1 += pow(count1, 2);
            countSqr2 += pow(count2, 2);
        }
    }
    return count / sqrt(countSqr1 * countSqr2);
}

double getCorrelationCoefficient3d(TRIPLERGB **mrx, int h, int w, int component, int y, int x) {
    double Exp1;
    double Exp2;
    if (y < 0) {
        Exp1 = getExpection(mrx, h, w - x, component, abs(y), 0);
        Exp2 = getExpection(mrx, h - abs(y), w, component, 0, x);
    } else {
        Exp1 = getExpection(mrx, h - y, w - x, component);
        Exp2 = getExpection(mrx, h, w, component, y, x);
    }
    double count = 0;
    double count1 = 0;
    double count2 = 0;
    double countSqr1 = 0;
    double countSqr2 = 0;
    for (int i = 0; i < h - abs(y); i++) {
        for (int j = 0; j < w - x; j++) {
            if (y < 0) {
                count1 = (double) getComponent((TRIPLEBYTES **) mrx, abs(y) + i, j, component) - Exp1;
                count2 = (double) getComponent((TRIPLEBYTES **) mrx, i, x + j, component) - Exp2;
            } else {
                count1 = (double) getComponent((TRIPLEBYTES **) mrx, i, j, component) - Exp1;
                count2 = (double) getComponent((TRIPLEBYTES **) mrx, y + i, x + j, component) - Exp2;
            }
            count += count1 * count2;
            countSqr1 += pow(count1, 2);
            countSqr2 += pow(count2, 2);
        }
    }
    return count / sqrt(countSqr1 * countSqr2);
}

double getStandartDeviation(TRIPLERGB **mrx, int h, int w, int component, double E) {
    double count = 0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            count += pow((double) getComponent((TRIPLEBYTES **) mrx, i, j, component) - E, 2);
        }
    }
    return count / (w * h);
}


double getMeanSquareDeviation(TRIPLERGB **mrx, int h, int w, int component, double E) {
    double count = 0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            count += pow((double) getComponent((TRIPLEBYTES **) mrx, i, j, component) - E, 2);
        }
    }
    return sqrt(count / (w * h));
}

double getExpection(TRIPLERGB **mrx, int h, int w, int component, int y, int x) {
    double count = 0;
    for (int i = y; i < h; i++) {
        for (int j = x; j < w; j++) {
            count += getComponent((TRIPLEBYTES **) mrx, i, j, component);
        }
    }
    return (count / ((h - y) * (w - x)));
}

double getPSNR(TRIPLEBYTES **mrx1, TRIPLEBYTES **mrx2, int y, int x, int h, int w, int component) {
    double count = 0;
    for (int i = y; i < h; i++) {
        for (int j = x; j < w; j++) {
            count += pow(getComponent(mrx1, i, j, component) -
                         getComponent(mrx2, i, j, component), 2);
        }
    }
    return 10 * log10(((w - x) * (h - y) * pow(255, 2)) / count);
}