//
// Created by sfaxi19 on 21.12.17.
//
#include "bmp.h"
#include "filter.h"

void saveToFile(int **mrx, const int height, const int width, const char *filename) {
    FILE *file = fopen(filename, "w");
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fprintf(file, "%3d,", mrx[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

int **getDifferenceArray(TRIPLEBYTES **mrx, int component, int height, int width, int rule) {
    int **diffArray = new int *[height - 1];
    int min = 256;
    int max = -256;
    for (int i = 1; i < height; i++) {
        diffArray[i - 1] = new int[width - 1];
        for (int j = 1; j < width; j++) {
            double f = 0;
            switch (rule) {
                case DIFF_RULE_1:
                    f = getComponent(mrx, i, j - 1, component);
                    break;
                case DIFF_RULE_2:
                    f = getComponent(mrx, i - 1, j, component);
                    break;
                case DIFF_RULE_3:
                    f = getComponent(mrx, i - 1, j - 1, component);
                    break;
                case DIFF_RULE_4:
                    f = round((getComponent(mrx, i, j - 1, component) +
                               getComponent(mrx, i - 1, j, component) +
                               getComponent(mrx, i - 1, j - 1, component)) / 3);
                    break;
                default:
                    break;
            }
            f = round(f);
            diffArray[i - 1][j - 1] = getComponent(mrx, i, j, component) - (int) f;
            if (diffArray[i - 1][j - 1] < min) min = diffArray[i - 1][j - 1];
            if (diffArray[i - 1][j - 1] > max) max = diffArray[i - 1][j - 1];
        }
    }
    std::string str = "";
    str.append("rule").append(std::to_string(rule)).append(".txt");
    saveToFile(diffArray, height - 1, width - 1, str.c_str());
    //printf("min = %d\tmax = %d\n", min, max);
    return diffArray;
}

int *getHistFreqFromDiff(int **mrx, int h, int w, const char *fpath, const char *fname) {
    int *freq = new int[BYTE_SIZE * 2 - 1];
    memset(freq, 0, sizeof(int) * BYTE_SIZE * 2 - 1);
    std::string command = "mkdir -p ";
    command.append(fpath);
    system(command.c_str());
    std::string filename = fpath;
    filename.append(fname);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            //printf("255 + %d = %d\n", mrx[i][j], 255 + mrx[i][j]);
            freq[255 + mrx[i][j]]++;
        }
    }
    saveHistCSVFile(freq, -255, BYTE_SIZE * 2 - 1, filename.c_str());
    return freq;
}

void checkDifferenceAndEntropy(TRIPLEBYTES **mrx, int height, int width, int component, const char *name) {
    printf("-------------------------------------\n%s:\n", name);
    int **diffRedRule1 = getDifferenceArray(mrx, component, height, width, DIFF_RULE_1);
    int **diffRedRule2 = getDifferenceArray(mrx, component, height, width, DIFF_RULE_2);
    int **diffRedRule3 = getDifferenceArray(mrx, component, height, width, DIFF_RULE_3);
    int **diffRedRule4 = getDifferenceArray(mrx, component, height, width, DIFF_RULE_4);

    std::string filepath = "histograms/Diffs/";
    filepath.append(name).append("/");
    std::string command = "mkdir -p ";
    system((command + filepath).c_str());
    filepath.append(name);
    std::string filename = name;
    int *diffFreqRule1 = getHistFreqFromDiff(diffRedRule1, height - 1, width - 1, (filepath + "1/").c_str(),
                                             (filename + "1.csv").c_str());
    int *diffFreqRule2 = getHistFreqFromDiff(diffRedRule2, height - 1, width - 1, (filepath + "2/").c_str(),
                                             (filename + "1.csv").c_str());
    int *diffFreqRule3 = getHistFreqFromDiff(diffRedRule3, height - 1, width - 1, (filepath + "3/").c_str(),
                                             (filename + "1.csv").c_str());
    int *diffFreqRule4 = getHistFreqFromDiff(diffRedRule4, height - 1, width - 1, (filepath + "4/").c_str(),
                                             (filename + "1.csv").c_str());
    double entropyRule1 = getEntropy(diffFreqRule1, height * width);
    double entropyRule2 = getEntropy(diffFreqRule2, height * width);
    double entropyRule3 = getEntropy(diffFreqRule3, height * width);
    double entropyRule4 = getEntropy(diffFreqRule4, height * width);

    printf("EntropyRule1 = %f\n", entropyRule1);
    printf("EntropyRule2 = %f\n", entropyRule2);
    printf("EntropyRule3 = %f\n", entropyRule3);
    printf("EntropyRule4 = %f\n", entropyRule4);
}

void bmp_part2(const char *filename) {
    BITMAPFILEHEADER bmFileDef;
    BITMAPINFOHEADER bmInfoDef;
    TRIPLERGB **mrxRGB = loadBMPFile(&bmFileDef, &bmInfoDef, filename);
    int defHeight = bmInfoDef.biHeight;
    int defWidth = bmInfoDef.biWidth;
    TRIPLEYCbCr **mrxYCbCr = (TRIPLEYCbCr **) RGB2YCbCr(mrxRGB, defHeight, defWidth);
    //printHeader(bmFileDef, bmInfoDef);
    checkDifferenceAndEntropy((TRIPLEBYTES **) mrxRGB, defHeight, defWidth, COMPONENT_RED, "histDiffRed");
    checkDifferenceAndEntropy((TRIPLEBYTES **) mrxRGB, defHeight, defWidth, COMPONENT_GREEN, "histDiffGreen");
    checkDifferenceAndEntropy((TRIPLEBYTES **) mrxRGB, defHeight, defWidth, COMPONENT_BLUE, "histDiffBlue");
    checkDifferenceAndEntropy((TRIPLEBYTES **) mrxYCbCr, defHeight, defWidth, COMPONENT_Y, "histDiffY");
    checkDifferenceAndEntropy((TRIPLEBYTES **) mrxYCbCr, defHeight, defWidth, COMPONENT_CB, "histDiffCb");
    checkDifferenceAndEntropy((TRIPLEBYTES **) mrxYCbCr, defHeight, defWidth, COMPONENT_CR, "histDiffCr");
    system("csv_charts -h histograms/Diffs/histDiffRed/histDiffRed1/ & exit");
    system("csv_charts -h histograms/Diffs/histDiffCb/histDiffCb1/ & exit");
    //system("csv_charts -h histograms/Diffs/histDiffRed2/ & exit");
    //system("csv_charts -h histograms/Diffs/histDiffRed3/ & exit");
    //system("csv_charts -h histograms/Diffs/histDiffRed4/ & exit");
}

void leftRotateBMP(TRIPLERGB **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo,
                   const char *filename, int rotate) {
    int height = bmInfo.biHeight;
    int width = bmInfo.biWidth;
    int newHeight = ((rotate % 180) != 0) ? bmInfo.biHeight = width : height;
    int newWidth = ((rotate % 180) != 0) ? bmInfo.biWidth = height : width;

    TRIPLERGB **newMrx = new TRIPLERGB *[newHeight];

    for (int i = 0; i < newHeight; i++) {
        newMrx[i] = new TRIPLERGB[newWidth];
        for (int j = 0; j < newWidth; j++) {
            switch (rotate) {
                case 90:
                    newMrx[i][j] = mrx[j][width - i - 1];
                    break;
                case 180:
                    newMrx[i][j] = mrx[height - i - 1][width - j - 1];
                    break;
                case 270:
                    newMrx[i][j] = mrx[height - j - 1][i];
                    break;
                default:
                    break;
            }
        }
    }
    saveBMPFile(bmFile, bmInfo, newMrx, filename);
    std::string command = "display ";
    system((command + filename).c_str());
}

void mirrorBMP(TRIPLERGB **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo,
               const char *filename, int mirror) {
    int height = bmInfo.biHeight;
    int width = bmInfo.biWidth;
    TRIPLERGB **newMrx = new TRIPLERGB *[height];
    for (int i = 0; i < height; i++) {
        newMrx[i] = new TRIPLERGB[width];
        for (int j = 0; j < width; j++) {
            switch (mirror) {
                case VERTICAL:
                    newMrx[i][j] = mrx[i][width - j - 1];
                    break;
                case HORIZONTAL:
                    newMrx[i][j] = mrx[height - i - 1][j];
                    break;
                default:
                    break;
            }
        }
    }
    saveBMPFile(bmFile, bmInfo, newMrx, filename);
    std::string command = "display ";
    system((command + filename).c_str());
}

void individual(const char *filepath) {
    BITMAPFILEHEADER bmFile;
    BITMAPINFOHEADER bmInfo;
    TRIPLERGB **mrx = loadBMPFile(&bmFile, &bmInfo, filepath);
    leftRotateBMP(mrx, bmFile, bmInfo, "rotate270.bmp", 180);
    mirrorBMP(mrx, bmFile, bmInfo, "mirrorHorizontal.bmp", VERTICAL);
}


int *getRandom(int size, double sigma) {
    system("mkdir -p noizeAdditive");
    FILE *file = fopen("noizeAdditive/noize.csv", "w");
    int *randArray = new int[size];
    int *freq = new int[BYTE_SIZE * 2 - 1];

    double z1;
    double z2;
    memset(freq, 0, sizeof(int) * BYTE_SIZE * 2 - 1);

    for (int i = 0; i < size; i += 2) {
        do {
            z1 = (double) rand() / RAND_MAX;
            z2 = (double) rand() / RAND_MAX;
        } while ((z1 == 0) || (z2 == 0));
        double x1 = sqrt((-2) * log(z1)) * sin(2 * M_PI * z2);
        double x2 = sqrt((-2) * log(z1)) * cos(2 * M_PI * z2);
        randArray[i] = (int) round((x1 * sigma));//clip(255 * x1, 0, 255);
        randArray[i + 1] = (int) round((x2 * sigma));//clip(255 * x2, 0, 255);
        freq[255 + randArray[i]]++;
        freq[255 + randArray[i + 1]]++;
    }
    for (int i = 0; i < (BYTE_SIZE * 2 - 1); i++) {
        fprintf(file, "%d,", (-255 + i));
    }
    fprintf(file, "\n");
    for (int i = 0; i < (BYTE_SIZE * 2 - 1); i++) {
        //cout << "[" << i << "] = " << freq[i] << endl;
        fprintf(file, "%d,", freq[i]);
    }
    fclose(file);
    return randArray;
}

TRIPLEBYTES **gause_noise(TRIPLEBYTES **mrx, int h, int w, double sigma, int component) {
    int *randNoize = getRandom(h * w, sigma);
    int id = 0;
    TRIPLEBYTES **gauseNoize = new TRIPLEBYTES *[h];
    for (int i = 0; i < h; i++) {
        gauseNoize[i] = new TRIPLEBYTES[w];
        for (int j = 0; j < w; j++) {
            switch (component) {
                case COMPONENT_A:
                    gauseNoize[i][j].A = clip(mrx[i][j].A + randNoize[id], 0, 255);
                    break;
                case COMPONENT_B:
                    gauseNoize[i][j].B = clip(mrx[i][j].B + randNoize[id], 0, 255);
                    break;
                case COMPONENT_C:
                    gauseNoize[i][j].C = clip(mrx[i][j].C + randNoize[id], 0, 255);
                    break;
                default:
                    break;
            }
            id++;
        }
    }
    delete[]randNoize;
    return gauseNoize;
}

unsigned char getImpulseValue(double p, double pa, double pb, unsigned char value, int *freq) {
    unsigned char newValue = 0;
    if (p < pa) {
        newValue = 0;
        freq[0]++;
    } else if ((p >= pa) && (p < (pa + pb))) {
        newValue = 255;
        freq[1]++;
    } else {
        newValue = value;
        freq[2]++;
    }
    return newValue;
}

TRIPLEBYTES **impulse_noise(TRIPLEBYTES **mrx, int h, int w, int component, double pa, double pb) {
    TRIPLEBYTES **impulseNoize = new TRIPLEBYTES *[h];
    int *freq = new int[3];
    memset(freq, 0, sizeof(freq[0]) * 3);
    for (int i = 0; i < h; i++) {
        impulseNoize[i] = new TRIPLEBYTES[w];
        for (int j = 0; j < w; j++) {
            double p = (double) rand() / RAND_MAX;
            switch (component) {
                case COMPONENT_A:
                    impulseNoize[i][j].A = getImpulseValue(p, pa, pb, mrx[i][j].A, &freq[0]);
                    break;
                case COMPONENT_B:
                    impulseNoize[i][j].B = getImpulseValue(p, pa, pb, mrx[i][j].B, &freq[0]);
                    break;
                case COMPONENT_C:
                    impulseNoize[i][j].C = getImpulseValue(p, pa, pb, mrx[i][j].C, &freq[0]);
                    break;
                default:
                    break;
            }
        }
    }
    //printf("freq[0]: %d\nfreq[255]: %d\nfreq[value]: %d\n", freq[0], freq[1], freq[2]);
    system("mkdir -p noizeImpuls/");
    FILE *file = fopen("noizeImpuls/noize.csv", "w");
    fprintf(file, "0,255,1,\n");
    fprintf(file, "%f,%f,%f,", (double) freq[0] / (h * w), (double) freq[1] / (h * w), (double) freq[2] / (h * w));
    fclose(file);
    return impulseNoize;
}

TRIPLEYCbCr **
impulseNoizeForPercent(TRIPLEYCbCr **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo, double percent,
                       const char *filepath) {
    percent = percent / 100;
    double pa = percent / 2;
    double pb = percent - pa;
    TRIPLEYCbCr **mrxImpulseNoize = (TRIPLEYCbCr **) impulse_noise((TRIPLEBYTES **) mrx, bmInfo.biHeight,
                                                                   bmInfo.biWidth, COMPONENT_Y, pa, pb);
    double psnr = getPSNR((TRIPLEBYTES **) mrx, (TRIPLEBYTES **) mrxImpulseNoize, 0, 0, bmInfo.biHeight, bmInfo.biWidth,
                          COMPONENT_Y);
    printf("Impulse noize: %4.0f%\tPSNR[original ; noize] = %6.3f\n", percent * 100, psnr);
    std::string path = filepath;
    system(("mkdir -p " + path).c_str());
    save_component_to_files((TRIPLEBYTES **) mrxImpulseNoize, bmFile, bmInfo, COMPONENT_Y,
                            (path + "/noize_" + std::to_string((int) (percent * 100)) + ".bmp").c_str());
    return mrxImpulseNoize;
}

void testAlphaForLaplass(TRIPLEYCbCr **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo, const char *filepath) {
    int weights[3][3] = {{0,  -1, 0},
                         {-1, 4,  -1},
                         {0,  -1, 0}};
    LaplasFilter laplasFilter(weights);
    std::string path = filepath;
    system(("mkdir -p " + path).c_str());
    int h = bmInfo.biHeight;
    int w = bmInfo.biWidth;
    for (double alpha = 1; alpha < 1.6; alpha += 0.1) {
        laplasFilter.setAlpha(alpha);
        TRIPLEYCbCr **mrxLaplasFilter = laplasFilter.apply_filter(mrx, h, w);
        save_component_to_files((TRIPLEBYTES **) mrxLaplasFilter, bmFile, bmInfo, COMPONENT_Y,
                                (path + "alpha_" + std::to_string(alpha) + ".bmp").c_str());
        std::string filepath_csv = filepath;
        filepath_csv.append("/hists/");
        std::string filename = "alpha_";
        filename.append(std::to_string(alpha)).append(".csv");
        getHistFreqFromTriple((TRIPLERGB **) mrxLaplasFilter, h, w, COMPONENT_Y, filepath_csv.c_str(),
                              filename.c_str());
    }
}


void clear_memory(TRIPLEYCbCr **mrx, int h) {
    for (int i = 0; i < h; i++)
        delete[] mrx[i];
    delete[] mrx;
}

void psnrCharts(TRIPLEYCbCr **mrxYCbCr, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo) {
    int min = 1;
    int max = 50;
    int step = 1;
    int h = bmInfo.biHeight;
    int w = bmInfo.biWidth;
    FILE *file = fopen("lab2/noize/chartPSNRGause/psnr.csv", "w");
    TRIPLEYCbCr **mrxGauseNoize;
    for (int sigma = min; sigma < max; sigma += step) {
        fprintf(file, "%d,", sigma);
    }
    fprintf(file, "\n");
    for (int sigma = min; sigma < max; sigma += step) {
        mrxGauseNoize = (TRIPLEYCbCr **) gause_noise((TRIPLEBYTES **) mrxYCbCr, bmInfo.biHeight,
                                                     bmInfo.biWidth, sigma, COMPONENT_Y);
        //printf("get noize\n");
        double psnr = getPSNR((TRIPLEBYTES **) mrxYCbCr, (TRIPLEBYTES **) mrxGauseNoize, 0, 0, h, w,
                              COMPONENT_Y);
        printf("sigma: %d\tPSNR[gauseNoize ; origin ] - %f\n", sigma, psnr);
        fprintf(file, "%4.2f,", psnr);
        clear_memory(mrxGauseNoize, h);
    }
    fclose(file);

    FILE *file2 = fopen("lab2/noize/chartPSNRImpulse/psnr.csv", "w");
    TRIPLEYCbCr **mrxImpulseNoize;
    for (double prob = 0.1; prob <= 1; prob += 0.1) {
        fprintf(file2, "%4.2f,", prob);
    }
    fprintf(file2, "\n");
    for (double prob = 0.1; prob <= 1; prob += 0.1) {
        mrxImpulseNoize = (TRIPLEYCbCr **) impulse_noise((TRIPLEBYTES **) mrxYCbCr, bmInfo.biHeight,
                                                         bmInfo.biWidth, COMPONENT_Y, prob / 2, prob / 2);
        //printf("get noize\n");
        double psnr = getPSNR((TRIPLEBYTES **) mrxYCbCr, (TRIPLEBYTES **) mrxImpulseNoize, 0, 0, h, w,
                              COMPONENT_Y);
        printf("prob: %f\tPSNR[gauseNoize ; origin ] - %f\n", prob, psnr);
        fprintf(file2, "%4.2f,", psnr);
        clear_memory(mrxImpulseNoize, h);
    }
    fclose(file2);
}


uint8_t interpolation(int x1, int y1, int x2, int y2, int value) {
    int x0 = 0;
    int y0 = 0;
    int x3 = 255;
    int y3 = 255;
    if (value < x1) {
        return clip(((value - x0) * (y1 - y0)) / (x1 - x0) + y0, 0, 255);
    } else if ((value >= x1) && (value < x2)) {
        return clip(((value - x1) * (y2 - y1)) / (x2 - x1) + y1, 0, 255);
    } else {
        return clip(((value - x2) * (y3 - y2)) / (x3 - x2) + y2, 0, 255);
    }
}

TRIPLEYCbCr **referencePoints(TRIPLEYCbCr **mrx, int h, int w, int x1, int y1, int x2, int y2) {
    int chart[256];
    TRIPLEYCbCr **mrxRefPoints = new TRIPLEYCbCr *[h];
    for (int i = 0; i < h; i++) {
        mrxRefPoints[i] = new TRIPLEYCbCr[w];
        for (int j = 0; j < w; j++) {
            mrxRefPoints[i][j].Y = interpolation(x1, y1, x2, y2, mrx[i][j].Y);
        }
    }
    for (int i = 0; i < 256; i++) {
        chart[i] = interpolation(x1, y1, x2, y2, i);
    }
    saveHistCSVFile(chart, 0, 256, "lab2/transformation/checkRefPoints.csv");
    return mrxRefPoints;
}

TRIPLEYCbCr **gammaTransformation(TRIPLEYCbCr **mrx, int h, int w, double gamma) {
    int chart[256];
    TRIPLEYCbCr **mrxGamma = new TRIPLEYCbCr *[h];
    for (int i = 0; i < h; i++) {
        mrxGamma[i] = new TRIPLEYCbCr[w];
        for (int j = 0; j < w; j++) {
            double floatValue = (double) mrx[i][j].Y / 255;
            mrxGamma[i][j].Y = (uint8_t) round(pow(floatValue, gamma) * 255);
        }
    }
    for (int i = 0; i < 256; i++) {
        double floatValue = (double) i / 255;
        chart[i] = (uint8_t) round(pow(floatValue, gamma) * 255);

    }
    std::string filename = "lab2/transformation/gammaCharts/";
    system(("mkdir -p " + filename).c_str());
    saveHistCSVFile(chart, 0, 256, (filename + "gamma_" + std::to_string(gamma) + ".csv").c_str());
    return mrxGamma;
}

void gammaTest(TRIPLEYCbCr **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo,
               double start, double stop, double step) {
    system("mkdir -p transformation/gamma/");
    std::string filename = "transformation/gamma/gamma_";
    for (double gamma = start; gamma < stop; gamma += step) {
        TRIPLEYCbCr **mrxGamma = gammaTransformation(mrx, bmInfo.biHeight, bmInfo.biWidth, gamma);
        save_component_to_files((TRIPLEBYTES **) mrxGamma, bmFile, bmInfo, COMPONENT_Y,
                                (filename + std::to_string(gamma) + ".bmp").c_str());
    }

}

TRIPLEYCbCr **gradationalTransformation(TRIPLEYCbCr **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo) {
    int *n = new int[256];
    int h = bmInfo.biHeight;
    int w = bmInfo.biWidth;
    memset(n, 0, sizeof(int) * 256);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            n[mrx[i][j].Y]++;
        }
    }

    uint8_t *s = new uint8_t[256];
    for (int i = 0; i < 256; i++) {
        int sum = 0;
        for (int j = 0; j < i; j++) {
            sum += n[j];
        }
        s[i] = (uint8_t) (sum * ((double) 255 / (h * w)));
    }
    TRIPLEYCbCr **mrxGrad = new TRIPLEYCbCr *[h];
    for (int i = 0; i < h; i++) {
        mrxGrad[i] = new TRIPLEYCbCr[w];
        for (int j = 0; j < w; j++) {
            mrxGrad[i][j].Y = s[mrx[i][j].Y];
        }
    }
    return mrxGrad;
}

void getBinFromGradation(TRIPLEYCbCr **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo) {
    system("mkdir -p lab2/bin/");
    //FILE file = fopen("lab2/bin/")
    for (int thr = 16; thr <= 240; thr += 10) {
        TRIPLEYCbCr **mrxBin = referencePoints(mrx, bmInfo.biHeight, bmInfo.biHeight, thr, 0, thr, 255);
        std::string filename = "lab2/bin/thr_";
        filename.append(std::to_string(thr)).append(".bmp");
        save_component_to_files((TRIPLEBYTES **) mrxBin, bmFile, bmInfo, COMPONENT_Y, filename.c_str());
    }
}


void example3(const char *filepath) {
    system("mkdir -p lab2/transformation/");
    BITMAPFILEHEADER bmFile;
    BITMAPINFOHEADER bmInfo;
    TRIPLERGB **mrxRGB = loadBMPFile(&bmFile, &bmInfo, filepath);
    TRIPLERGB **mrxHightRGB = loadBMPFile(&bmFile, &bmInfo, filepath);
    TRIPLERGB **mrxLowRGB = loadBMPFile(&bmFile, &bmInfo, filepath);
    TRIPLERGB **mrxOriginRGB = loadBMPFile(&bmFile, &bmInfo, filepath);

    printHeader(bmFile, bmInfo);
    int defHeight = bmInfo.biHeight;
    int defWidth = bmInfo.biWidth;
    TRIPLEYCbCr **mrxYCbCr = RGB2YCbCr(mrxRGB, defHeight, defWidth);
    TRIPLEYCbCr **mrxHight = RGB2YCbCr(mrxHightRGB, defHeight, defWidth);
    TRIPLEYCbCr **mrxLow = RGB2YCbCr(mrxLowRGB, defHeight, defWidth);
    TRIPLEYCbCr **mrxOrigin = RGB2YCbCr(mrxOriginRGB, defHeight, defWidth);

    TRIPLEYCbCr **mrxRefPoints = referencePoints(mrxYCbCr, defHeight, defWidth, 127, 50, 200, 200);
    gammaTest(mrxYCbCr, bmFile, bmInfo, 0, 1, 0.1);
    gammaTest(mrxYCbCr, bmFile, bmInfo, 2, 25, 1);
    TRIPLEYCbCr **mrxGamma = gammaTransformation(mrxYCbCr, bmInfo.biHeight, bmInfo.biWidth, 3);
    TRIPLEYCbCr **mrxGrad = gradationalTransformation(mrxGamma, bmFile, bmInfo);
    getHistFreqFromTriple((TRIPLERGB **) mrxYCbCr, defHeight, defWidth, COMPONENT_Y,
                          "lab2/transformation/gamma_hists/", "origin.csv");
    getHistFreqFromTriple((TRIPLERGB **) mrxGamma, defHeight, defWidth, COMPONENT_Y,
                          "lab2/transformation/gamma_hists/", "gamma_3.csv");

    save_component_to_files((TRIPLEBYTES **) mrxYCbCr, bmFile, bmInfo, COMPONENT_Y, "lab2/transformation/origin.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxRefPoints, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/transformation/ref_points.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxGrad, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/transformation/grad_transform.bmp");

    getHistFreqFromTriple((TRIPLERGB **) mrxYCbCr, defHeight, defWidth, COMPONENT_Y,
                          "lab2/transformation/ref_points_hists/", "origin.csv");
    getHistFreqFromTriple((TRIPLERGB **) mrxRefPoints, defHeight, defWidth, COMPONENT_Y,
                          "lab2/transformation/ref_points_hists/", "refPoints.csv");
    getHistFreqFromTriple((TRIPLERGB **) mrxGrad, defHeight, defWidth, COMPONENT_Y,
                          "lab2/transformation/grad_transform/", "transform.csv");
    getBinFromGradation(mrxYCbCr, bmFile, bmInfo);
}

void example2(const char *filepath) {
    system("mkdir -p lab2/noize/images/");
    system("mkdir -p lab2/contour/");
    BITMAPFILEHEADER bmFile;
    BITMAPINFOHEADER bmInfo;
    TRIPLERGB **mrxRGB = loadBMPFile(&bmFile, &bmInfo, filepath);
    system("mkdir -p lab2/noize/chartPSNRGause/");
    system("mkdir -p lab2/noize/chartPSNRImpulse/");

    printHeader(bmFile, bmInfo);
    int defHeight = bmInfo.biHeight;
    int defWidth = bmInfo.biWidth;
    TRIPLEYCbCr **mrxYCbCr = (TRIPLEYCbCr **) RGB2YCbCr(mrxRGB, defHeight, defWidth);
    save_component_to_files((TRIPLEBYTES **) mrxYCbCr, bmFile, bmInfo, COMPONENT_Y, "lab2/noize/images/origin.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxYCbCr, bmFile, bmInfo, COMPONENT_Y, "lab2/contour/origin.bmp");
    psnrCharts(mrxYCbCr, bmFile, bmInfo);

    TRIPLEYCbCr **mrxGauseNoize = (TRIPLEYCbCr **) gause_noise((TRIPLEBYTES **) mrxYCbCr, bmInfo.biHeight,
                                                               bmInfo.biWidth, 30, COMPONENT_Y);

    //percent
    TRIPLEYCbCr **mrxImpulseNoize = (TRIPLEYCbCr **) impulse_noise((TRIPLEBYTES **) mrxYCbCr, bmInfo.biHeight,
                                                                   bmInfo.biWidth, COMPONENT_Y, 0.01, 0.01);
    TRIPLEYCbCr **impulseNoize5 = impulseNoizeForPercent(mrxYCbCr, bmFile, bmInfo, 5, "lab2/noize/imagesImpulseNoize/");
    TRIPLEYCbCr **impulseNoize10 = impulseNoizeForPercent(mrxYCbCr, bmFile, bmInfo, 10,
                                                          "lab2/noize/imagesImpulseNoize/");
    TRIPLEYCbCr **impulseNoize25 = impulseNoizeForPercent(mrxYCbCr, bmFile, bmInfo, 25,
                                                          "lab2/noize/imagesImpulseNoize/");
    TRIPLEYCbCr **impulseNoize50 = impulseNoizeForPercent(mrxYCbCr, bmFile, bmInfo, 50,
                                                          "lab2/noize/imagesImpulseNoize/");

    MovingAveradgeFilter movingAveradgeFilter;
    GauseFilter gauseFilter;
    MedianFilter medianFilter;
    int weights[3][3] = {{0,  -1, 0},
                         {-1, 4,  -1},
                         {0,  -1, 0}};
    LaplasFilter laplasFilter(weights);

    int radius = movingAveradgeFilter.searchRadius(mrxYCbCr, mrxGauseNoize, 0, 5, bmFile, bmInfo,
                                                   "lab2/noize/movingRadiusSearch");
    TRIPLEYCbCr **mrxGauseNoizeRecov1 = movingAveradgeFilter.apply_filter(mrxGauseNoize, defHeight, defWidth, radius);

    double sigma = gauseFilter.searchSigma(mrxYCbCr, mrxGauseNoize, 0.1, 2, bmFile, bmInfo, "lab2/noize/searchSigma",
                                           radius);
    TRIPLEYCbCr **mrxGauseNoizeRecov2 = gauseFilter.apply_filter(mrxGauseNoize, defHeight, defWidth, radius, sigma);

    int radiusMedianImpulse = medianFilter.searchRadius(mrxYCbCr, impulseNoize5, 0, 4, bmFile, bmInfo,
                                                        "lab2/noize/medianRadiusImpulseSearch5");

    radiusMedianImpulse = medianFilter.searchRadius(mrxYCbCr, impulseNoize10, 0, 4, bmFile, bmInfo,
                                                    "lab2/noize/medianRadiusImpulseSearch10");

    radiusMedianImpulse = medianFilter.searchRadius(mrxYCbCr, impulseNoize25, 0, 4, bmFile, bmInfo,
                                                    "lab2/noize/medianRadiusImpulseSearch25");

    radiusMedianImpulse = medianFilter.searchRadius(mrxYCbCr, impulseNoize50, 0, 4, bmFile, bmInfo,
                                                    "lab2/noize/medianRadiusImpulseSearch50");
    TRIPLEYCbCr **mrxGauseNoizeRecov3 = medianFilter.apply_filter(mrxGauseNoize, defHeight, defWidth, 2);

    TRIPLEYCbCr **mrxLaplasFilter = laplasFilter.apply_filter(mrxYCbCr, defHeight, defWidth);

    laplasFilter.setAdding(128);

    TRIPLEYCbCr **mrxLaplasFilter_add_128 = laplasFilter.apply_filter(mrxYCbCr, defHeight, defWidth);

    laplasFilter.setAdding(SYN);

    TRIPLEYCbCr **mrxLaplasFilter_add_syn = laplasFilter.apply_filter(mrxYCbCr, defHeight, defWidth);

    testAlphaForLaplass(mrxYCbCr, bmFile, bmInfo, "lab2/contour/testLaplassAlpha/");

    SobelFilter sobelFilter;
    sobelFilter.searchThr(mrxYCbCr, 50, 250, 10, bmFile, bmInfo, "lab2/contour/searchSobelThr/");
    TRIPLEYCbCr **mrxSobelFilter = sobelFilter.apply_filter(mrxYCbCr, bmFile, bmInfo, 127);


    save_component_to_files((TRIPLEBYTES **) mrxSobelFilter, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/contour/sobel_thr_127.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxGauseNoize, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/noize/images/imageGauseNoize.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxImpulseNoize, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/noize/images/imageImpulseNoize.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxGauseNoizeRecov1, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/noize/images/filterGauseNoize1.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxGauseNoizeRecov2, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/noize/images/filterGauseNoize2.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxGauseNoizeRecov3, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/noize/images/filterGauseNoize3.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxLaplasFilter, bmFile, bmInfo, COMPONENT_Y, "lab2/contour/laplas.bmp");

    save_component_to_files((TRIPLEBYTES **) mrxLaplasFilter_add_128, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/contour/laplas_128.bmp");
    save_component_to_files((TRIPLEBYTES **) mrxLaplasFilter_add_syn, bmFile, bmInfo, COMPONENT_Y,
                            "lab2/contour/laplas_syn.bmp");
}

void example1(const char *filename, const char *file_Y, const char *file_Cb, const char *file_Cr,
              const char *file_rgb_recov) {
    BITMAPFILEHEADER bmFileDef;
    BITMAPINFOHEADER bmInfoDef;
    std::cout << "reading file...\n";
    system("mkdir -p images");
    system("mkdir -p histograms");
    //Read bmp file
    TRIPLERGB **mrxRGB = loadBMPFile(&bmFileDef, &bmInfoDef, filename);
    std::cout << "-------------------------------------\n";
    printHeader(bmFileDef, bmInfoDef);
    std::cout << "-------------------------------------\n";
    size_t defHeight = bmInfoDef.biHeight;
    size_t defWidth = bmInfoDef.biWidth;
    TRIPLERGB **mrxYCbCr = (TRIPLERGB **) RGB2YCbCr(mrxRGB, defHeight, defWidth);

    save_components_to_files(mrxRGB, bmFileDef, bmInfoDef, FORMAT_RGB,
                             "file_Red.bmp", "file_Green.bmp", "file_Blue.bmp");
    save_components_to_files(mrxYCbCr, bmFileDef, bmInfoDef, FORMAT_YCBCR,
                             "file_Y.bmp", "file_Cb.bmp", "file_Cr.bmp");

    TRIPLERGB **mrxY = loadBMPFile(&bmFileDef, &bmInfoDef, file_Y);
    TRIPLERGB **mrxCb = loadBMPFile(&bmFileDef, &bmInfoDef, file_Cb);
    TRIPLERGB **mrxCr = loadBMPFile(&bmFileDef, &bmInfoDef, file_Cr);
    TRIPLERGB **mrxRGBconv = YCbCr2RGB(mrxY, mrxCb, mrxCr, defHeight, defWidth);
    saveBMPFile(bmFileDef, bmInfoDef, mrxRGBconv, file_rgb_recov);

    double PSNR_RGB_RGBr_red = getPSNR((TRIPLEBYTES **) mrxRGB, (TRIPLEBYTES **) mrxRGBconv, 0, 0, defHeight, defWidth,
                                       COMPONENT_RED);
    double PSNR_RGB_RGBr_green = getPSNR((TRIPLEBYTES **) mrxRGB, (TRIPLEBYTES **) mrxRGBconv, 0, 0, defHeight,
                                         defWidth, COMPONENT_GREEN);
    double PSNR_RGB_RGBr_blue = getPSNR((TRIPLEBYTES **) mrxRGB, (TRIPLEBYTES **) mrxRGBconv, 0, 0, defHeight, defWidth,
                                        COMPONENT_BLUE);
    checkCorrelationProperties(mrxRGB, defHeight, defWidth, FORMAT_RGB);
    checkCorrelationProperties(mrxYCbCr, defHeight, defWidth, FORMAT_YCBCR);
    std::cout << "-------------------------------------\n";
    std::cout << "PSNR(red)[RGB, RGBr] = " << PSNR_RGB_RGBr_red << "\n";
    std::cout << "PSNR(green)[RGB, RGBr] = " << PSNR_RGB_RGBr_green << "\n";
    std::cout << "PSNR(blue)[RGB, RGBr] = " << PSNR_RGB_RGBr_blue << "\n";


    std::cout << "-------------------------------------\n";
    std::cout << "\nDecimation \"mean\" x2:\n";
    decimationAndRecovering(mrxCb, bmFileDef, bmInfoDef, DECIMATION_MEAN, 2,
                            "images/decMeanCb2.bmp", "images/recMeanCb2.bmp");
    decimationAndRecovering(mrxCr, bmFileDef, bmInfoDef, DECIMATION_MEAN, 2,
                            "images/decMeanCr2.bmp", "images/recMeanCr2.bmp");
    check_PSNR_for_recov(mrxRGB, mrxY, mrxCb, mrxCr, "images/recMeanCb2.bmp", "images/recMeanCr2.bmp",
                         "images/recoveryMeanRGB2.bmp");
    std::cout << "\nDecimation \"mean\" x4:\n";
    decimationAndRecovering(mrxCb, bmFileDef, bmInfoDef, DECIMATION_MEAN, 4,
                            "images/decMeanCb4.bmp", "images/recMeanCb4.bmp");
    decimationAndRecovering(mrxCr, bmFileDef, bmInfoDef, DECIMATION_MEAN, 4,
                            "images/decMeanCr4.bmp", "images/recMeanCr4.bmp");

    check_PSNR_for_recov(mrxRGB, mrxY, mrxCb, mrxCr, "images/recMeanCb4.bmp", "images/recMeanCr4.bmp",
                         "images/recoveryMeanRGB4.bmp");

    std::cout << "\nDecimation \"ejection\" x2:\n";
    decimationAndRecovering(mrxCb, bmFileDef, bmInfoDef, DECIMATION_EJECT, 2,
                            "images/decEjectCb2.bmp", "images/recEjectCb2.bmp");
    decimationAndRecovering(mrxCr, bmFileDef, bmInfoDef, DECIMATION_EJECT, 2,
                            "images/decEjectCr2.bmp", "images/recEjectCr2.bmp");
    check_PSNR_for_recov(mrxRGB, mrxY, mrxCb, mrxCr, "images/recEjectCb2.bmp", "images/recEjectCr2.bmp",
                         "images/recoveryEjectRGB2.bmp");
    std::cout << "\nDecimation \"ejection\" x4:\n";
    decimationAndRecovering(mrxCb, bmFileDef, bmInfoDef, DECIMATION_EJECT, 4,
                            "images/decEjectCb4.bmp", "images/recEjectCb4.bmp");
    decimationAndRecovering(mrxCr, bmFileDef, bmInfoDef, DECIMATION_EJECT, 4,
                            "images/decEjectCr4.bmp", "images/recEjectCr4.bmp");

    check_PSNR_for_recov(mrxRGB, mrxY, mrxCb, mrxCr, "images/recEjectCb4.bmp", "images/recEjectCr4.bmp",
                         "images/recoveryEjectRGB4.bmp");
    std::cout << "-------------------------------------\n";

    int *freq;
    freq = getHistFreqFromTriple(mrxRGB, defHeight, defWidth, COMPONENT_RED, "histograms/Part1/histRed/",
                                 "histRed.csv");
    double entropyRed = getEntropy(freq, defHeight * defWidth);
    delete[] freq;
    freq = getHistFreqFromTriple(mrxRGB, defHeight, defWidth, COMPONENT_GREEN, "histograms/Part1/histGreen/",
                                 "histGreen.csv");
    double entropyGreen = getEntropy(freq, defHeight * defWidth);
    delete[] freq;
    freq = getHistFreqFromTriple(mrxRGB, defHeight, defWidth, COMPONENT_BLUE, "histograms/Part1/histBlue/",
                                 "histBlue.csv");
    double entropyBlue = getEntropy(freq, defHeight * defWidth);
    delete[] freq;
    freq = getHistFreqFromTriple(mrxYCbCr, defHeight, defWidth, COMPONENT_RED, "histograms/Part1/histY/", "histY.csv");
    double entropyY = getEntropy(freq, defHeight * defWidth);
    delete[] freq;
    freq = getHistFreqFromTriple(mrxYCbCr, defHeight, defWidth, COMPONENT_GREEN, "histograms/Part1/histCb/",
                                 "histCb.csv");
    double entropyCb = getEntropy(freq, defHeight * defWidth);
    delete[] freq;
    freq = getHistFreqFromTriple(mrxYCbCr, defHeight, defWidth, COMPONENT_BLUE, "histograms/Part1/histCr/",
                                 "histCr.csv");
    double entropyCr = getEntropy(freq, defHeight * defWidth);
    delete[] freq;
    std::cout << "Entropy_Red = " << entropyRed << "\n";
    std::cout << "Entropy_Green = " << entropyGreen << "\n";
    std::cout << "Entropy_Blue = " << entropyBlue << "\n";
    std::cout << "Entropy_Y = " << entropyY << "\n";
    std::cout << "Entropy_Cb = " << entropyCb << "\n";
    std::cout << "Entropy_Cr = " << entropyCr << "\n";
    printf("-------------------------------------\n");
    printf("-------------------------------------\n");
    printf("-------------------------------------\n");
    usleep(2 * 1000 * 1000);
    //system("csv_charts -h histograms/Part1/histRed/ & exit");
    //system("csv_charts -h histograms/Part1/histGreen/ & exit");
    //system("csv_charts -h histograms/Part1/histBlue/ & exit");
    //system("csv_charts -h histograms/Part1/histY/ & exit");
    //system("csv_charts -h histograms/Part1/histCb/ & exit");
    //system("csv_charts -h histograms/Part1/histCr/ & exit");
    for (int i = 0; i < defHeight; i++) {
        delete[] mrxRGB[i];
        delete[] mrxYCbCr[i];
        delete[] mrxY[i];
        delete[] mrxCb[i];
        delete[] mrxCr[i];
        delete[] mrxRGBconv[i];
    }
    delete[] mrxRGB;
    delete[] mrxYCbCr;
    delete[] mrxY;
    delete[] mrxCb;
    delete[] mrxCr;
    delete[] mrxRGBconv;
}
