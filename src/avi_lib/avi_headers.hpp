//
// Created by sfaxi19 on 21.12.17.
//

#ifndef AVI_HEADERS_HPP
#define AVI_HEADERS_HPP
struct AVIHeader {
    unsigned int dwMicroSecPerFrame;
    unsigned int dwMaxBytesPerSec;
    unsigned int dwPaddingGranularity;
    unsigned int dwFlags;
    unsigned int dwTotalFrames;
    unsigned int dwInitialFrames;
    unsigned int dwStreams;
    unsigned int dwSuggestedBufferSize;
    unsigned int dwWidth;
    unsigned int dwHeight;
    unsigned int time_scale;
    unsigned int playback_data_rate;
    unsigned int starting_time;
    unsigned int data_length;
};

struct WAVEFORMAT {
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
};

struct RECT {
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
};

struct StreamHeader {
    uint32_t fccType = 0;
    uint32_t fccHandler = 0;
    uint32_t dwFlags = 0;
    uint32_t dwPriority = 0;
    uint32_t dwInitialFrames = 0;
    uint32_t dwScale = 0;
    uint32_t dwRate = 0;
    uint32_t dwStart = 0;
    uint32_t dwLength = 0;
    uint32_t dwSuggestedBufferSize = 0;
    uint32_t dwQuality = 0;
    uint32_t dwSampleSize = 0;
    RECT rcFrame = {};
};

struct Chunk {
    uint32_t ckID = 0;
    uint32_t ckSize = 0;
};

struct List {
    uint32_t listID = 0;
    uint32_t listSize = 0;
    uint32_t listType = 0;
};

const uint32_t REC__TYPE = 0x20636572;
const uint32_t HDRL_TYPE = 0x6c726468;
const uint32_t MOVI_TYPE = 0x69766f6d;
const uint32_t STRL_TYPE = 0x6c727473;
const uint32_t AVI__TYPE = 0x20495641;
const uint32_t LIST = 0x5453494c;
const uint32_t RIFF = 0x46464952;
const uint32_t AVIH = 0x68697661;
const uint32_t STRH = 0x68727473;
const uint32_t STRF = 0x66727473;
const uint32_t VIDS = 0x73646976;
const uint32_t AUDS = 0x73647561;
const uint32_t JUNK = 0x4b4e554a;
const uint32_t VEDT = 0x74646576;
const uint32_t IDX1 = 0x31786469;

const uint32_t STRD = 0x01010101;
const uint32_t STRN = 0x01010101;

const uint32_t DB00 = 0x62643030;
const uint32_t WB01 = 0x62773130;

const uint32_t XX00 = 0x00003030;
const uint32_t XX01 = 0x00003130;

const uint16_t WB = 0x6277;
const uint16_t DB = 0x6264;
#endif //AVI_HEADERS_HPP
