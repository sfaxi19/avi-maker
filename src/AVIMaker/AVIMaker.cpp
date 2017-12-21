//
// Created by sfaxi19 on 26.09.17.
//
#include "AVIMaker.h"

int AVIMaker::saveAVIFile(const char *filepath) {
    if (this->videoStreams[0]->frames.size() == 0) return -1;
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        printf("File not found [%s]\n", filepath);
        return 1;
    }
    fpos_t pRIFFsize;
    fpos_t cur_pos;
    fwrite(&RIFF, sizeof(RIFF), 1, file);
    fgetpos(file, &pRIFFsize);
    TRACE printf("pRIFFsize: %d\n", pRIFFsize);
    fwrite(&SIZE, sizeof(SIZE), 1, file);
    fwrite(&AVI__TYPE, sizeof(AVI__TYPE), 1, file);
    saveHDRL(file, this);
    saveMOVI(file, this);
    fgetpos(file, &cur_pos);
    fsetpos(file, &pRIFFsize);
    __off_t sizeRIFF = cur_pos.__pos - (pRIFFsize.__pos + 4);
    TRACE printf("RIFF size: %ld", sizeRIFF);
    fwrite(&sizeRIFF, 4, 1, file);
    fsetpos(file, &cur_pos);
    fclose(file);
    return 0;
}

int AVIMaker::readAVIFile(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        TRACE printf("file error<%s>\n", filepath);
        return -1;
    }
    Chunk chunk;
    uint32_t node;
    static int frame_count = 0;
    uint32_t fake;
    while (!feof(file)) {
        node = read_node(file, fake);
        TRACE printf("...readNode\n");
        fpos_t pos;
        fgetpos(file, &pos);
        TRACE print_fourcc(node);
        TRACE printf("\npos: %ld\n", pos.__pos);
        switch (node) {
            case RIFF: {
                List list;
                list.listID = 0;
                list.listType = 0;
                list.listSize = 0;
                read_list(file, list, node, fake);
                TRACE print_list(list);
                break;//17235284
            }
            case LIST: {
                List list;
                list.listID = 0;
                list.listType = 0;
                list.listSize = 0;
                read_list(file, list, node, fake);
                TRACE printf("main\n");
                TRACE print_list(list);
                if (list.listType == HDRL_TYPE) {
                    read_hdrl(file, this);
                }
                if (list.listType == MOVI_TYPE) {
                    read_movi(file, list.listSize - LIST_TYPE_SIZE, this);
                }
                //skip(file, list.listSize);
                break;
            }
            case VEDT:
                read_chunk(file, chunk, node, fake);
                //print_chunk(chunk);
                skip(file, chunk.ckSize, fake);
                break;
            case JUNK:
                read_chunk(file, chunk, node, fake);
                TRACE print_chunk(chunk);
                skip(file, chunk.ckSize, fake);
                break;
            case IDX1:
                read_chunk(file, chunk, node, fake);
                skip(file, chunk.ckSize, fake);
                //print_fourcc(node);
                TRACE print_chunk(chunk);
                break;
            default:
                TRACE printf("unknow fourcc main - ");
                TRACE print_fourcc(node);
                TRACE printf("- 0x%x\n", node);
                break;
        };
        //int x = 0;
        //std::cin >> x;
    }
    fclose(file);
}

void AVIMaker::saveVideoStreamToBMP(const char *filepath) {
    if (this->videoStreams[0]->frames.size() == 0) return;
    system("mkdir -p cm_files");
    BITMAPFILEHEADER bmFile;
    bmFile.bfType = (((uint16_t) 'M') << 8) | ((uint16_t) 'B');
    bmFile.bfSize = 12;//bmInfo.biSizeImage;// + BITMAP_INFO_SIZE;// + BITMAP_FILE_SIZE;
    bmFile.bfReserved1 = 0;
    bmFile.bfReserved2 = 54;
    bmFile.bfOffBits = 0;
    std::string filename = filepath;
    filename.append("/frame_");
    system("rm cm_files/*");
    int id = 0;
    std::vector<TRIPLERGB **>::iterator it = this->videoStreams[0]->frames.begin();
    for (; it != this->videoStreams[0]->frames.end(); it++) {
        saveBMPAVIFile(bmFile, this->videoStreams[0]->bmInfo, *it, (filename + std::to_string(id)).c_str());
        id++;
    }
}


AVIMaker::AVIMaker(const AVIHeader aviHeader, VideoStream *videoStream) : aviHeader(aviHeader) {
    this->videoStreams.push_back(videoStream);
}

AVIMaker::~AVIMaker() {
    for (size_t video_id = 0; video_id < videoStreams.size(); video_id++) {
        int height = videoStreams[video_id]->bmInfo.biHeight;
        for (size_t frame_id = 0; frame_id < videoStreams[video_id]->frames.size(); frame_id++) {
            TRIPLERGB **tmp = videoStreams[video_id]->frames[frame_id];
            for (int i = 0; i < height; i++) {
                delete[] tmp[i];
            }
            delete[] tmp;
        }
    }

    for (size_t audio_id = 0; audio_id < audioStreams.size(); audio_id++) {
        for (size_t sample_id = 0; sample_id < audioStreams[audio_id]->samples.size(); sample_id++) {
            delete[] audioStreams[audio_id]->samples[sample_id];
        }
        delete audioStreams[audio_id]->streamHeader;
        delete audioStreams[audio_id]->wave;
    }
}

void AVIMaker::reverseVideoStream(size_t stream_id) {
    uint64_t frames = this->videoStreams[stream_id]->frames.size();
    if (frames == 0) return;
    TRIPLERGB **tmp;
    for (int i = 0; i < frames / 2; i++) {
        tmp = this->videoStreams[stream_id]->frames[i];
        this->videoStreams[stream_id]->frames[i] = this->videoStreams[0]->frames[frames - i - 1];
        this->videoStreams[stream_id]->frames[frames - i - 1] = tmp;
    }
}

void AVIMaker::addFrames(size_t stream_id, std::vector<TRIPLERGB **> add_frame) {
    VideoStream *videoStream = this->videoStreams[stream_id];
    for (size_t id = 0; id < add_frame.size(); id++) {
        TRIPLERGB **rgb = new TRIPLERGB *[videoStream->bmInfo.biHeight];
        for (int i = 0; i < videoStream->bmInfo.biHeight; i++) {
            rgb[i] = new TRIPLERGB[videoStream->bmInfo.biWidth];
            for (int j = 0; j < videoStream->bmInfo.biWidth; j++) {
                rgb[i][j].red = add_frame[id][i][j].red;
                rgb[i][j].green = add_frame[id][i][j].green;
                rgb[i][j].blue = add_frame[id][i][j].blue;
            }
        }
        videoStream->frames.push_back(rgb);
    }
}

void AVIMaker::addFrame(size_t stream_id, TRIPLERGB **frame) {
    VideoStream *videoStream = this->videoStreams[stream_id];
    videoStream->frames.push_back(frame);
}

void AVIMaker::addCopyFrame(size_t stream_id, TRIPLERGB **frame) {
    auto **rgb = new TRIPLERGB *[video()->height()];
    for (int i = 0; i < video()->height(); i++) {
        rgb[i] = new TRIPLERGB[video()->width()];
        for (int j = 0; j < video()->width(); j++) {
            rgb[i][j].red = frame[i][j].red;
            rgb[i][j].blue = frame[i][j].blue;
            rgb[i][j].green = frame[i][j].green;
        }
    }
    addFrame(0, rgb);
}

VideoStream *AVIMaker::getVideoStreamByID(uint32_t stream_id) {
    for (int i = 0; i < this->videoStreams.size(); i++) {
        if (this->videoStreams[i]->streamID == stream_id) return this->videoStreams[i];
    }
    TRACE printf("getVideoStreamByID failed. id = %d", stream_id);
    return nullptr;
}

AudioStream *AVIMaker::getAudioStreamByID(uint32_t stream_id) {
    for (int i = 0; i < this->audioStreams.size(); i++) {
        if (this->audioStreams[i]->streamID == stream_id) return this->audioStreams[i];
    }
    TRACE printf("getAudioStreamByID failed. id = %d", stream_id);
    return nullptr;
}