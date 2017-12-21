//
// Created by sfaxi19 on 21.12.17.
//

#ifndef BMP_HEADERS_HPP
#define BMP_HEADERS_HPP


const int BITMAP_FILE_SIZE = 14;
const int BITMAP_INFO_SIZE = 40;

struct TRIPLERGB {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
};

struct TRIPLEBYTES {
    unsigned char C;
    unsigned char B;
    unsigned char A;
};

struct TRIPLEYCbCr {
    unsigned char Cr;
    unsigned char Cb;
    unsigned char Y;
};

struct BITMAPFILEHEADER {
    unsigned short int bfType;
    unsigned int bfSize;
    unsigned short int bfReserved1;
    unsigned short int bfReserved2;
    unsigned int bfOffBits;
};

struct BITMAPINFOHEADER {
    unsigned int biSize;
    unsigned int biWidth;
    unsigned int biHeight;
    unsigned short int biPlanes;
    unsigned short int biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    unsigned int biXPelsPerMeter;
    unsigned int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
};

#endif //BMP_HEADERS_HPP
