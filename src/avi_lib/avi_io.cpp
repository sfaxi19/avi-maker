//
// Created by sfaxi19 on 26.09.17.
//

#include "avi_rw.h"

void print_bitmap_info(BITMAPINFOHEADER &bmInfo) {
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

void print_list(List &list) {
    print_fourcc(list.listID);
    printf(" - %d", list.listSize);
    printf(" - ");
    print_fourcc(list.listType);
    printf("\n");
}

void print_chunk(Chunk &chunk) {
    print_fourcc(chunk.ckID);
    printf(" - %d",chunk.ckSize);
    printf("\n");
}

void print_avi_header(AVIHeader &avi_header) {
    printf("\nAVI Main Header:\n");
    printf("dwMicroSecPerFrame:   \t%d\n", avi_header.dwMicroSecPerFrame);
    printf("dwMaxBytesPerSec:     \t%d\n", avi_header.dwMaxBytesPerSec);
    printf("dwPaddingGranularity: \t%d\n", avi_header.dwPaddingGranularity);
    printf("dwFlags:              \t%d\n", avi_header.dwFlags);
    printf("dwTotalFrames:        \t%d\n", avi_header.dwTotalFrames);
    printf("dwInitialFrames:      \t%d\n", avi_header.dwInitialFrames);
    printf("dwStreams:            \t%d\n", avi_header.dwStreams);
    printf("dwSuggestedBufferSize:\t%d\n", avi_header.dwSuggestedBufferSize);
    printf("dwWidth:              \t%d\n", avi_header.dwWidth);
    printf("dwHeight:             \t%d\n", avi_header.dwHeight);
    printf("time_scale:           \t%d\n", avi_header.time_scale);
    printf("playback_data_rate:   \t%d\n", avi_header.playback_data_rate);
    printf("starting_time:        \t%d\n", avi_header.starting_time);
    printf("data_length:          \t%d\n", avi_header.data_length);
}

void print_fourcc(uint32_t fourcc) {
    printf("%c%c%c%c", fourcc, fourcc >> 8, fourcc >> 16, fourcc >> 24);
}

void print_stream_header(StreamHeader &streamHeader) {
    printf("\nStream Header:\n");
    printf("fccType:               ");
    print_fourcc(streamHeader.fccType);
    printf("\n");
    printf("fccHandler:            ");
    print_fourcc(streamHeader.fccHandler);
    printf("\n");
    printf("dwFlags:               %d\n", streamHeader.dwFlags);
    printf("dwPriority:            %d\n", streamHeader.dwPriority);
    printf("dwInitialFrames:       %d\n", streamHeader.dwInitialFrames);
    printf("dwScale:               %d\n", streamHeader.dwScale);
    printf("dwRate:                %d\n", streamHeader.dwRate);
    printf("dwStart:               %d\n", streamHeader.dwStart);
    printf("dwLength:              %d\n", streamHeader.dwLength);
    printf("dwSuggestedBufferSize: %d\n", streamHeader.dwSuggestedBufferSize);
    printf("dwQuality:             %d\n", streamHeader.dwQuality);
    printf("dwSampleSize:          %d\n", streamHeader.dwSampleSize);
    printf("rcFrame:               %d.%d.%d.%d\n",
           streamHeader.rcFrame.x1, streamHeader.rcFrame.y1,
           streamHeader.rcFrame.x2, streamHeader.rcFrame.y2);
}

void print_wave(WAVEFORMAT &wave) {
    printf("\nWave Info\n");
    printf("nAvgBytesPerSec:\t%d\n", wave.nAvgBytesPerSec);
    printf("nBlockAlign:\t%d\n", wave.nBlockAlign);
    printf("nChannels:\t%d\n", wave.nChannels);
    printf("nSamplesPerSec:\t%d\n", wave.nSamplesPerSec);
    printf("wBitsPerSample:\t%d\n", wave.wBitsPerSample);
    printf("wFormatTag:\t%d\n", wave.wFormatTag);
}