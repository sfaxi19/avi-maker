//
// Created by sfaxi19 on 26.09.17.
//

#ifndef AVI_RW_H
#define AVI_RW_H

#include <vector>
#include "iostream"
#include "avi_headers.hpp"
#include "../bmp_lib/bmp_headers.hpp"

class AVIMaker;

const uint32_t CHUNK_SIZE = 8;
const uint32_t LIST_SIZE = 12;
const uint32_t LIST_TYPE_SIZE = 4;
const uint32_t SIZE = 0x00000000;


struct VideoStream {
    size_t streamID{0};
    StreamHeader streamHeader;
    BITMAPINFOHEADER bmInfo;
    std::vector<TRIPLERGB **> frames;

    // for support prev. version
    VideoStream() {};

    VideoStream(const BITMAPINFOHEADER &info, const StreamHeader &sHeader) {
        bmInfo = info;
        streamHeader = sHeader;
    }

    size_t getLength() {
        return frames.size();
    }

    TRIPLERGB **getFrame(size_t id) {
        return frames[id];
    }

    size_t height() { return bmInfo.biHeight; };

    size_t width() { return bmInfo.biWidth; };

};

struct AudioStream {
    size_t streamID;
    StreamHeader *streamHeader;
    WAVEFORMAT *wave;
    std::vector<uint8_t *> samples;
};

/*
 * implementation in avi_io.cpp file
 */
void print_bitmap_info(BITMAPINFOHEADER &bmInfo);

void print_list(List &list);

void print_chunk(Chunk &chunk);

void print_avi_header(AVIHeader &avi_header);

void print_fourcc(uint32_t fourcc);

void print_stream_header(StreamHeader &streamHeader);

void print_wave(WAVEFORMAT &wave);

void saveBMPAVIFile(const BITMAPFILEHEADER &bmFile, const BITMAPINFOHEADER &bmInfo,
                    TRIPLERGB **matrix, const char *filepath);

TRIPLERGB **loadBMPFile(BITMAPINFOHEADER &bmInfo, FILE *file);

/*  reading from file   */
void read_chunk(FILE *file, Chunk &chunk, uint32_t node, uint32_t &bytes);

void read_list(FILE *file, List &list, uint32_t node, uint32_t &bytes);

void skip(FILE *file, int size, uint32_t &bytes);

uint32_t read_node(FILE *file, uint32_t &bytes);

//====================================================================
//                     AVI RIFF format r/w functions
//====================================================================
void read_hdrl(FILE *file, AVIMaker *aviMaker);

void read_movi(FILE *file, uint32_t bytes, AVIMaker *aviMaker);

void read_strl(FILE *file, size_t, AVIMaker *aviMaker);

void read_rec(FILE *file, uint32_t bytes, AVIMaker *aviMaker);

bool read_data(FILE *file, uint32_t node, uint32_t &bytes, AVIMaker *aviMaker);

uint16_t getDataChunkType(uint32_t node);

uint32_t saveHDRL(FILE *file, AVIMaker *aviMaker);

uint32_t saveMOVI(FILE *file, AVIMaker *aviMaker);

uint32_t saveVideoREC(FILE *file, TRIPLERGB **rgb, BITMAPINFOHEADER *bmInfo);

uint32_t saveVideoSTRL(FILE *file, VideoStream *videoStream);

#endif //AVI_RW_H
