//
// Created by sfaxi19 on 26.09.17.
//

#ifndef MULTITECH_AVIMAKER_H
#define MULTITECH_AVIMAKER_H

#include "../avi_lib/avi_rw.h"
#include "vector"

#define TRACE if(isTrace)


class AVIMaker {
private:
    bool isTrace = false;
public:
    AVIHeader aviHeader;
    std::vector<VideoStream *> videoStreams;
    std::vector<AudioStream *> audioStreams;

    AVIMaker(AVIHeader aviHeader, VideoStream *videoStream);

    AVIMaker() {};

    AVIMaker(const char *filepath) {
        int res = this->readAVIFile(filepath);
        if (res != 0) {
            perror(filepath);
//            exit(-2);
        }
    }

    VideoStream *video() {
        return videoStreams[0];
    }

    virtual ~AVIMaker();

    void setTrace(bool value) {
        isTrace = value;
    }

    int readAVIFile(const char *filepath);

    int saveAVIFile(const char *filepath);

    void saveVideoStreamToBMP(const char *filepath);

    void saveFrameToBMP(const char *filename, int frameID);

    void reverseVideoStream(size_t stream_id);

    void addFrames(size_t stream_id, std::vector<TRIPLERGB **> add_frame);

    void addFrame(size_t stream_id, TRIPLERGB **frame);

    void addCopyFrame(size_t stream_id, TRIPLERGB **frame);

    VideoStream *getVideoStreamByID(uint32_t stream_id);

    AudioStream *getAudioStreamByID(uint32_t stream_id);
};


#endif //MULTITECH_AVIMAKER_H
