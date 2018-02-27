/*
 * dip_bmp.cpp
 *
 *  Created on: 28 февр. 2017 г.
 *      Author: sfaxi19
 */
#include "bmp.h"

void printHeader(const BITMAPFILEHEADER bmFile, const BITMAPINFOHEADER bmInfo) {
    printf("Bitmap file:\n");
    char type[2] = {(char) (bmFile.bfType & 0xff), (char) ((bmFile.bfType >> 8) & 0xff)};
    printf("Type: \t\t%c%c\n", type[0], type[1]);
    printf("Size: \t\t%d bytes\n", bmFile.bfSize);
    printf("Reserved 1: \t%d\n", bmFile.bfReserved1);
    printf("Reserved 2: \t%d\n", bmFile.bfReserved2);
    printf("OffBits: \t%d\n", bmFile.bfOffBits);

    printf("\nBitmap info:\n");
    printf("Size: \t%d\n", bmInfo.biSize);
    printf("Width: \t%d\n", bmInfo.biWidth);
    printf("Height:\t%d\n", bmInfo.biHeight);
    printf("Planes:\t%d\n", bmInfo.biPlanes);
    printf("BitCount\t%d\n", bmInfo.biBitCount);
    printf("Compression:\t%d\n", bmInfo.biCompression);
    printf("SizeImage:\t%d\n", bmInfo.biSizeImage);
    printf("XPelsPerMeter:\t%d\n", bmInfo.biXPelsPerMeter);
    printf("YPelsPerMeter:\t%d\n", bmInfo.biYPelsPerMeter);
    printf("ClrUsed: \t%d\n", bmInfo.biClrUsed);
    printf("ClrImportant: \t%d\n", bmInfo.biClrImportant);
}

void printMatrix(TRIPLERGB **matrix, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::cout << (int) matrix[i][j].red << " - "
                      << (int) matrix[i][1].green << " - "
                      << (int) matrix[i][1].blue << "\n";
        }
    }
}

void saveHistCSVFile(int *mrx, const int start, const int width, const char *filename) {
    FILE *csvFile = fopen(filename, "w");
    for (int j = start; j < start + width; j++) {
        fprintf(csvFile, "%d,", j);
    }
    fprintf(csvFile, "\n");
    for (int j = 0; j < width; j++) {
        fprintf(csvFile, "%d,", mrx[j]);
    }
    fclose(csvFile);
}

void saveXYCSVFile(double **mrx, const int height, const int width, std::string format) {
    std::string filename;
    for (int i = 0; i < height; i++) {
        filename.clear();
        //system("mkdir -p charts/chartsRGB");
        //system("mkdir -p charts/chartsYCbCr");
        filename.append("charts/")
                .append("charts").append(format).append("/");
        std::string command = "mkdir -p ";
        command.append(filename);
        system(command.c_str());
        filename.append("Chart").append(format)
                .append("_H(")
                .append(std::to_string((-10) + i * 5))
                .append(")");
        filename.append(".csv");
        //cout << filename << "\n";
        //cout << "filepath = " << filepath << "\n";
        FILE *csvFile = fopen(filename.c_str(), "w");
        //cout << "check2\n";
        for (int j = 0; j < width; j++) {
            fprintf(csvFile, "%d,", j);
        }
        fprintf(csvFile, "\n");
        for (int j = 0; j < width; j++) {
            fprintf(csvFile, "%4.3f,", mrx[i][j]);
        }
        fclose(csvFile);
    }
}

void saveBMPFile(const BITMAPFILEHEADER bmFile, const BITMAPINFOHEADER bmInfo,
                 TRIPLERGB **matrix, const char *filepath) {
    FILE *file = fopen(filepath, "w");
    fwrite(&bmFile, BITMAP_FILE_SIZE, 1, file);
    fwrite(&bmInfo, BITMAP_INFO_SIZE, 1, file);
    size_t dummy_count = (4 - ((bmInfo.biWidth * 3) % 4)) % 4;
    u_char *dummy = new u_char[dummy_count];
    for (int i = 0; i < dummy_count; i++) {
        dummy[i] = 0x00;
    }
    //memset(dummy,0,dummy_count);
    for (uint i = 0; i < bmInfo.biHeight; i++) {
        fwrite(&matrix[i][0], sizeof(TRIPLERGB) * bmInfo.biWidth, 1, file);
        fwrite(&dummy[0], dummy_count, 1, file);
    }
    delete[] dummy;
    fclose(file);
}

TRIPLERGB **loadBMPFile(BITMAPFILEHEADER *bmFile, BITMAPINFOHEADER *bmInfo, const char *filepath) {
    FILE *file = fopen(filepath, "r");
    fread(bmFile, BITMAP_FILE_SIZE, 1, file);
    fread(bmInfo, BITMAP_INFO_SIZE, 1, file);
    size_t dummy_count = (4 - (bmInfo->biWidth * 3) % 4) % 4;
    u_char *dummy = new u_char[dummy_count];
    TRIPLERGB **mrx = new TRIPLERGB *[bmInfo->biHeight];
    for (size_t i = 0; i < bmInfo->biHeight; i++) {
        mrx[i] = new TRIPLERGB[bmInfo->biWidth];
        fread(&mrx[i][0], (sizeof(TRIPLERGB) * bmInfo->biWidth), 1, file);
        fread(&dummy[0], dummy_count, 1, file);
    }
    delete[]dummy;
    fclose(file);
    return mrx;
}

u_char getComponent(TRIPLEBYTES **mrx, int i, int j, int component) {
    switch (component) {
        case COMPONENT_A:
            return mrx[i][j].A;
        case COMPONENT_B:
            return mrx[i][j].B;
        case COMPONENT_C:
            return mrx[i][j].C;
        default:
            break;
    }
    return 0;
}

void setComponent(TRIPLEBYTES **mrx, int i, int j, uint8_t value, int component) {
    switch (component) {
        case COMPONENT_A:
            mrx[i][j].A = value;
        case COMPONENT_B:
            mrx[i][j].B = value;
        case COMPONENT_C:
            mrx[i][j].C = value;
        default:
            break;
    }
}

double **getChartsForComponent(TRIPLERGB **mrx, int h, int w, int component) {
    double **graph = new double *[5];
    const int chart_3d_stop_width = w / 4;
    for (int i = 0; i < 5; i++) {
        graph[i] = new double[chart_3d_stop_width / CHART_3D_STEP_WIDTH];
        for (int j = 0; j < chart_3d_stop_width / CHART_3D_STEP_WIDTH; j++) {
            graph[i][j] = getCorrelationCoefficient3d(mrx, h, w, component,
                                                      CHART_3D_START_HEIGHT + (i * CHART_3D_STEP_HEIGHT),
                                                      j * CHART_3D_STEP_WIDTH);
            printf("%8.3f", graph[i][j]);
        }
        std::cout << "\n";
    }
    return graph;
}

void checkCorrelationProperties(TRIPLERGB **mrx, int h, int w, int format) {
    std::string a, b, c;
    switch (format) {
        case FORMAT_RGB:
            a = 'R';
            b = 'G';
            c = 'B';
            break;
        case FORMAT_YCBCR:
            a = 'Y';
            b = "Cb";
            c = "Cr";
            break;
    }
    std::cout << "-------------------------------------\n";
    std::cout << "Correlation properties for " << a << b << c << ":\n\n";
    double ExpR = getExpection(mrx, h, w, COMPONENT_RED);
    double ExpG = getExpection(mrx, h, w, COMPONENT_GREEN);
    double ExpB = getExpection(mrx, h, w, COMPONENT_BLUE);
    double MSDR = getMeanSquareDeviation(mrx, h, w, COMPONENT_RED, ExpR);
    double MSDG = getMeanSquareDeviation(mrx, h, w, COMPONENT_GREEN, ExpG);
    double MSDB = getMeanSquareDeviation(mrx, h, w, COMPONENT_BLUE, ExpB);
    double rRR = getCorrelationCoefficient(mrx, mrx, h, w, COMPONENT_RED, COMPONENT_RED, ExpR, ExpR);
    double rRG = getCorrelationCoefficient(mrx, mrx, h, w, COMPONENT_RED, COMPONENT_GREEN, ExpR, ExpG);
    double rRB = getCorrelationCoefficient(mrx, mrx, h, w, COMPONENT_RED, COMPONENT_BLUE, ExpR, ExpB);
    double rBG = getCorrelationCoefficient(mrx, mrx, h, w, COMPONENT_BLUE, COMPONENT_GREEN, ExpB, ExpG);
    std::cout << "E[" << a << "] = " << ExpR << "\t" << "MSD[" << a << "] = " << MSDR << "\n";
    std::cout << "E[" << b << "] = " << ExpG << "\t" << "MSD[" << b << "] = " << MSDG << "\n";
    std::cout << "E[" << c << "] = " << ExpB << "\t" << "MSD[" << c << "] = " << MSDB << "\n";
    std::cout << "r[" << a << ", " << a << "] = " << rRR << "\n";
    std::cout << "r[" << a << ", " << b << "] = " << rRG << "\n";
    std::cout << "r[" << a << ", " << c << "] = " << rRB << "\n";
    std::cout << "r[" << c << ", " << b << "] = " << rBG << "\n";
    const int chart_3d_stop_width = (w / 4) / CHART_3D_STEP_WIDTH;
    std::cout << "Charts " << a << b << c << a << "\n";
    double **chartsR = getChartsForComponent(mrx, h, w, COMPONENT_RED);
    std::cout << "Charts " << a << b << c << b << "\n";
    double **chartsG = getChartsForComponent(mrx, h, w, COMPONENT_GREEN);
    std::cout << "Charts " << a << b << c << c << "\n";
    double **chartsB = getChartsForComponent(mrx, h, w, COMPONENT_BLUE);
    std::string formatStrR = a + b + c + "_" + a;
    std::string formatStrG = a + b + c + "_" + b;
    std::string formatStrB = a + b + c + "_" + c;
    std::cout << "Save charts " << a << b << c << a << "\n";
    saveXYCSVFile(chartsR, CHART_3D_COUNT_HEIGHT, chart_3d_stop_width, formatStrR);
    std::cout << "Save charts " << a << b << c << b << "\n";
    saveXYCSVFile(chartsG, CHART_3D_COUNT_HEIGHT, chart_3d_stop_width, formatStrG);
    std::cout << "Save charts " << a << b << c << c << "\n";
    saveXYCSVFile(chartsB, CHART_3D_COUNT_HEIGHT, chart_3d_stop_width, formatStrB);
    for (int i = 0; i < 5; i++) {
        delete[] chartsR[i];
        delete[] chartsG[i];
        delete[] chartsB[i];
    }
    delete[] chartsR;
    delete[] chartsG;
    delete[] chartsB;
}

TRIPLERGB **recovery(BITMAPFILEHEADER bmFile,
                     BITMAPINFOHEADER bmInfo,
                     TRIPLERGB **mrx, int k, const char *filepath) {
    u_int h = bmInfo.biHeight;
    u_int w = bmInfo.biWidth;
    bmInfo.biWidth = bmInfo.biWidth * k;
    bmInfo.biHeight = bmInfo.biHeight * k;
    bmInfo.biSizeImage = bmInfo.biWidth * bmInfo.biHeight * 3;
    //cout << "Recovery... New size = " << m_bmInfo.biSizeImage << "\n";
    TRIPLERGB **recMrx = new TRIPLERGB *[bmInfo.biHeight];
    for (int i = 0; i < bmInfo.biHeight; i++) {
        recMrx[i] = new TRIPLERGB[bmInfo.biWidth];
    }
    for (size_t i = 0; i < h; i++) {
        for (size_t j = 0; j < w; j++) {
            size_t idSubH = i * k;
            //size_t count = 0;
            while (idSubH < (k * (i + 1))) {
                size_t idSubW = j * k;
                while (idSubW < (k * (j + 1))) {
                    recMrx[idSubH][idSubW].red = mrx[i][j].red;
                    recMrx[idSubH][idSubW].green = mrx[i][j].green;
                    recMrx[idSubH][idSubW].blue = mrx[i][j].blue;
                    idSubW++;
                }
                idSubH++;
            }
        }
    }
    saveBMPFile(bmFile, bmInfo, recMrx, filepath);
    return recMrx;
}

TRIPLERGB **decimationMean(BITMAPFILEHEADER bmFile,
                           BITMAPINFOHEADER bmInfo,
                           TRIPLERGB **mrx, int k, const char *filepath) {
    u_int h;
    u_int w;
    bmInfo.biWidth = w = bmInfo.biWidth / k;
    bmInfo.biHeight = h = bmInfo.biHeight / k;
    bmInfo.biSizeImage = bmInfo.biWidth * bmInfo.biHeight * 3;
    //cout << "Deimation... New size = " << m_bmInfo.biSizeImage << "\n";
    TRIPLERGB **decMrx = new TRIPLERGB *[h];
    for (size_t i = 0; i < h; i++) {
        decMrx[i] = new TRIPLERGB[w];
        for (size_t j = 0; j < w; j++) {
            size_t idSubH = i * k;
            size_t count = 0;
            while (idSubH < (k * (i + 1))) {
                size_t idSubW = j * k;
                while (idSubW < (k * (j + 1))) {
                    count += mrx[idSubH][idSubW].red;
                    idSubW++;
                }
                idSubH++;
            }
            u_char tmp = (u_char) round(count / (k * k));
            decMrx[i][j].red = tmp;
            decMrx[i][j].green = tmp;
            decMrx[i][j].blue = tmp;
        }
    }
    saveBMPFile(bmFile, bmInfo, decMrx, filepath);
    return decMrx;
}

TRIPLERGB **decimationEjection(BITMAPFILEHEADER bmFile,
                               BITMAPINFOHEADER bmInfo,
                               TRIPLERGB **mrx, int k, const char *filepath) {
    unsigned int originH = bmInfo.biHeight;
    unsigned int originW = bmInfo.biWidth;
    bmInfo.biWidth = originW / k;
    bmInfo.biHeight = originH / k;
    bmInfo.biSizeImage = bmInfo.biWidth * bmInfo.biHeight * 3;
    //cout << "Decimation... New size = " << m_bmInfo.biSizeImage << "\n";
    TRIPLERGB **decMrx = new TRIPLERGB *[bmInfo.biHeight];
    int idH = 0;
    for (size_t i = 0; i < originH; i++) {
        if ((i % k) == (k - 1)) {
            decMrx[idH] = new TRIPLERGB[bmInfo.biWidth];
            int idW = 0;
            for (size_t j = 0; j < originW; j++) {
                if ((j % k) == (k - 1)) {
                    decMrx[idH][idW].red = mrx[i][j].red;
                    decMrx[idH][idW].green = mrx[i][j].green;
                    decMrx[idH][idW].blue = mrx[i][j].blue;
                    idW++;
                }
            }
            idH++;
        }
    }
    saveBMPFile(bmFile, bmInfo, decMrx, filepath);
    return decMrx;
}


TRIPLERGB **YCbCr2RGB(TRIPLERGB **mrxY, TRIPLERGB **mrxCb, TRIPLERGB **mrxCr, int h, int w) {
    TRIPLERGB **mrxRGBrec = new TRIPLERGB *[h];
    for (size_t i = 0; i < h; i++) {
        mrxRGBrec[i] = new TRIPLERGB[w];
        for (size_t j = 0; j < w; j++) {
            u_char Y = mrxY[i][j].red;
            u_char Cb = mrxCb[i][j].red;
            u_char Cr = mrxCr[i][j].red;
            int R = (int) round(Y + 1.402 * (Cr - 128));
            int G = (int) round(Y - 0.714 * (Cr - 128) - 0.334 * (Cb - 128));
            int B = (int) round(Y + 1.772 * (Cb - 128));
            R = (R > 255) ? 255 : ((R < 0) ? 0 : R);
            G = (G > 255) ? 255 : ((G < 0) ? 0 : G);
            B = (B > 255) ? 255 : ((B < 0) ? 0 : B);
            mrxRGBrec[i][j].red = (u_char) R;
            mrxRGBrec[i][j].green = (u_char) G;
            mrxRGBrec[i][j].blue = (u_char) B;
        }
    }
    return mrxRGBrec;
}

TRIPLERGB **Y2RGB(TRIPLEYCbCr **mrx, size_t h, size_t w) {
    TRIPLERGB **rgb = new TRIPLERGB *[h];
    for (size_t i = 0; i < h; i++) {
        rgb[i] = new TRIPLERGB[w];
        for (size_t j = 0; j < w; j++) {
            rgb[i][j].blue = rgb[i][j].red = rgb[i][j].green = mrx[i][j].Y;
        }
    }
    return rgb;
}

TRIPLEYCbCr RGB2YCbCrPix(TRIPLERGB &pix) {
    TRIPLEYCbCr pixYCbCr{};
    double Y = round(0.299 * pix.red + 0.587 * pix.green + 0.114 * pix.blue);
    double Cb = round(0.5643 * (pix.blue - Y) + 128);
    double Cr = round(0.7132 * (pix.red - Y) + 128);
    Y = (Y > 255) ? 255 : ((Y < 0) ? 0 : Y);
    Cb = (Cb > 255) ? 255 : ((Cb < 0) ? 0 : Cb);
    Cr = (Cr > 255) ? 255 : ((Cr < 0) ? 0 : Cr);
    pixYCbCr.Y = (u_char) Y;
    pixYCbCr.Cb = (u_char) Cb;
    pixYCbCr.Cr = (u_char) Cr;
    return pixYCbCr;
}

TRIPLEYCbCr **RGB2YCbCr(TRIPLERGB **mrxRGB, size_t h, size_t w) {
    TRIPLEYCbCr **mrxYCbCr = new TRIPLEYCbCr *[h];
    for (size_t i = 0; i < h; i++) {
        mrxYCbCr[i] = new TRIPLEYCbCr[w];
        for (size_t j = 0; j < w; j++) {
            int Y = (int) round(0.299 * mrxRGB[i][j].red
                                + 0.587 * mrxRGB[i][j].green + 0.114 * mrxRGB[i][j].blue);
            int Cb = (int) round(0.5643 * (mrxRGB[i][j].blue - Y) + 128);
            int Cr = (int) round(0.7132 * (mrxRGB[i][j].red - Y) + 128);
            Y = (Y > 255) ? 255 : ((Y < 0) ? 0 : Y);
            Cb = (Cb > 255) ? 255 : ((Cb < 0) ? 0 : Cb);
            Cr = (Cr > 255) ? 255 : ((Cr < 0) ? 0 : Cr);
            mrxYCbCr[i][j].Y = (u_char) Y;
            mrxYCbCr[i][j].Cb = (u_char) Cb;
            mrxYCbCr[i][j].Cr = (u_char) Cr;
        }
    }
    return mrxYCbCr;
}

void save_component_to_files(TRIPLEBYTES **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo, int component,
                             const char *filename) {
    int height = bmInfo.biHeight;
    int width = bmInfo.biWidth;
    TRIPLERGB **mrxComponent = new TRIPLERGB *[height];
    for (int i = 0; i < height; i++) {
        mrxComponent[i] = new TRIPLERGB[width];
        for (int j = 0; j < width; j++) {
            mrxComponent[i][j].red = mrxComponent[i][j].green = mrxComponent[i][j].blue = getComponent(mrx, i, j,
                                                                                                       component);
        }
    }
    saveBMPFile(bmFile, bmInfo, mrxComponent, filename);
    for (int i = 0; i < height; i++) {
        delete[] mrxComponent[i];
    }
    delete[] mrxComponent;
}

void save_components_to_files(TRIPLERGB **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo, int format,
                              const char *filepath1, const char *filepath2, const char *filepath3) {
    int height = bmInfo.biHeight;
    int width = bmInfo.biWidth;
    TRIPLERGB **mrxA = new TRIPLERGB *[height];
    TRIPLERGB **mrxB = new TRIPLERGB *[height];
    TRIPLERGB **mrxC = new TRIPLERGB *[height];
    for (size_t i = 0; i < height; i++) {
        mrxA[i] = new TRIPLERGB[width];
        mrxB[i] = new TRIPLERGB[width];
        mrxC[i] = new TRIPLERGB[width];
        for (size_t j = 0; j < width; j++) {
            switch (format) {
                case FORMAT_RGB:
                    mrxA[i][j].red = mrx[i][j].red;
                    mrxB[i][j].green = mrx[i][j].green;
                    mrxC[i][j].blue = mrx[i][j].blue;
                    break;
                case FORMAT_YCBCR:
                    mrxA[i][j].red = mrxA[i][j].green = mrxA[i][j].blue = mrx[i][j].red;
                    mrxB[i][j].red = mrxB[i][j].green = mrxB[i][j].blue = mrx[i][j].green;
                    mrxC[i][j].red = mrxC[i][j].green = mrxC[i][j].blue = mrx[i][j].blue;
                    break;
                default:
                    break;
            }
        }
    }
    switch (format) {
        case FORMAT_RGB:
            saveBMPFile(bmFile, bmInfo, mrxA, filepath1);
            saveBMPFile(bmFile, bmInfo, mrxB, filepath2);
            saveBMPFile(bmFile, bmInfo, mrxC, filepath3);
            break;
        case FORMAT_YCBCR:
            saveBMPFile(bmFile, bmInfo, mrxA, filepath1);
            saveBMPFile(bmFile, bmInfo, mrxB, filepath2);
            saveBMPFile(bmFile, bmInfo, mrxC, filepath3);
            break;
        default:
            break;
    }
    for (int i = 0; i < height; i++) {
        delete[] mrxA[i];
        delete[] mrxB[i];
        delete[] mrxC[i];
    }
    delete[] mrxA;
    delete[] mrxB;
    delete[] mrxC;
}

TRIPLERGB **decimationAndRecovering(TRIPLERGB **mrx, BITMAPFILEHEADER bmFile, BITMAPINFOHEADER bmInfo,
                                    int decimation, int k, const char *filenameDecimate, const char *filenameRecover) {
    switch (decimation) {
        case DECIMATION_EJECT:
            decimationEjection(bmFile, bmInfo, mrx, k, filenameDecimate);
            break;
        case DECIMATION_MEAN:
            decimationMean(bmFile, bmInfo, mrx, k, filenameDecimate);
            break;
        default:
            break;
    }

    BITMAPFILEHEADER bmFileDecimat;
    BITMAPINFOHEADER bmInfoDecimat;
    TRIPLERGB **mrxDecimat = loadBMPFile(&bmFileDecimat, &bmInfoDecimat, filenameDecimate);
    TRIPLERGB **mrxRecov = recovery(bmFileDecimat, bmInfoDecimat, mrxDecimat, k, filenameRecover);

    //cout << "-------------------------------------\n";
    return mrxRecov;
}

void check_PSNR_for_recov(TRIPLERGB **mrxRGB, TRIPLERGB **mrxY, TRIPLERGB **mrxCb, TRIPLERGB **mrxCr,
                          const char *filenameCb, const char *filenameCr, const char *filenameRGB) {
    BITMAPFILEHEADER bmFile;
    BITMAPINFOHEADER bmInfo;
    TRIPLERGB **mrxRecovCb = loadBMPFile(&bmFile, &bmInfo, filenameCb);
    TRIPLERGB **mrxRecovCr = loadBMPFile(&bmFile, &bmInfo, filenameCr);
    int height = bmInfo.biHeight;
    int width = bmInfo.biWidth;
    TRIPLERGB **mrxRGBRecov = YCbCr2RGB(mrxY, mrxRecovCb, mrxRecovCr, height, width);
    saveBMPFile(bmFile, bmInfo, mrxRGBRecov, filenameRGB);

    double PSNR_Cb_Cbr = getPSNR((TRIPLEBYTES **) mrxCb, (TRIPLEBYTES **) mrxRecovCb, 0, 0, height, width,
                                 COMPONENT_RED);
    double PSNR_Cr_Crr = getPSNR((TRIPLEBYTES **) mrxCr, (TRIPLEBYTES **) mrxRecovCr, 0, 0, height, width,
                                 COMPONENT_RED);
    double PSNR_R_Rr = getPSNR((TRIPLEBYTES **) mrxRGB, (TRIPLEBYTES **) mrxRGBRecov, 0, 0, height, width,
                               COMPONENT_RED);
    double PSNR_G_Gr = getPSNR((TRIPLEBYTES **) mrxRGB, (TRIPLEBYTES **) mrxRGBRecov, 0, 0, height, width,
                               COMPONENT_GREEN);
    double PSNR_B_Br = getPSNR((TRIPLEBYTES **) mrxRGB, (TRIPLEBYTES **) mrxRGBRecov, 0, 0, height, width,
                               COMPONENT_BLUE);
    std::cout << "PSNR[Cb, Cb'] = " << PSNR_Cb_Cbr << "\n";
    std::cout << "PSNR[Cr, Cr'] = " << PSNR_Cr_Crr << "\n";
    std::cout << "PSNR[R, R'] = " << PSNR_R_Rr << "\n";
    std::cout << "PSNR[G, G'] = " << PSNR_G_Gr << "\n";
    std::cout << "PSNR[B, B'] = " << PSNR_B_Br << "\n";
    for (int i = 0; i < height; i++) {
        delete[] mrxRecovCb[i];
        delete[] mrxRecovCr[i];
        delete[] mrxRGBRecov[i];
    }
    delete[] mrxRecovCb;
    delete[] mrxRecovCr;
    delete[] mrxRGBRecov;
}

int *getHistFreqFromTriple(TRIPLERGB **mrx, int h, int w, int component, const char *fpath, const char *fname) {
    int *freq = new int[BYTE_SIZE];
    memset(freq, 0, sizeof(int) * BYTE_SIZE);
    std::string command = "mkdir -p ";
    command.append(fpath);
    system(command.c_str());
    std::string filename = fpath;
    filename.append(fname);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            switch (component) {
                case COMPONENT_RED:
                    freq[mrx[i][j].red]++;
                    break;
                case COMPONENT_GREEN:
                    freq[mrx[i][j].green]++;
                    break;
                case COMPONENT_BLUE:
                    freq[mrx[i][j].blue]++;
                    break;
                default:
                    break;
            }

        }
    }
    saveHistCSVFile(freq, 0, BYTE_SIZE, filename.c_str());
    return freq;
}

double getEntropy(const int freq[], int N) {
    double count = 0;
    double p = 0;
    for (int i = 0; i < BYTE_SIZE; i++) {
        p = ((double) freq[i] / N);
        if (p == 0)continue;
        count += p * log2(p);
    }
    return count * (-1);
}

uint8_t clip(int value, uint8_t min, uint8_t max) {
    //value = round(value);
    return (value > max) ? max : ((value < min) ? min : ((uint8_t) value));
}

