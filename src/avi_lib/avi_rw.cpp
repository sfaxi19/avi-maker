//
// Created by sfaxi19 on 26.09.17.
//
#include "avi_rw.h"
#include "../AVIMaker/AVIMaker.h"

void saveBMPAVIFile(const BITMAPFILEHEADER &bmFile, const BITMAPINFOHEADER &bmInfo,
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

TRIPLERGB **loadBMPAVIFile(BITMAPINFOHEADER &bmInfo, FILE *file) {
    //printf("bmFile: %d\nm_bmInfo: %d\n", m_bmInfo.biHeight, m_bmInfo.biWidth);
    //size_t dummy_count = (4 - (m_bmInfo.biWidth * 3) % 4) % 4;
    //u_char *dummy = new u_char[dummy_count];
    TRIPLERGB **mrx = new TRIPLERGB *[bmInfo.biHeight];
    for (size_t i = 0; i < bmInfo.biHeight; i++) {
        mrx[i] = new TRIPLERGB[bmInfo.biWidth];
        fread(&mrx[i][0], (sizeof(TRIPLERGB) * bmInfo.biWidth), 1, file);
        //fread(&dummy[0], dummy_count, 1, file);
    }
    //delete[]dummy;
    return mrx;
}


void read_chunk(FILE *file, Chunk &chunk, uint32_t node, uint32_t &bytes) {
    if (node == 0) {
        fread(&(chunk.ckID), sizeof(chunk.ckID), 1, file);
        bytes -= sizeof(chunk.ckID);
    } else {
        chunk.ckID = node;
    }
    fread(&(chunk.ckSize), sizeof(chunk.ckSize), 1, file);
    bytes -= sizeof(chunk.ckSize);
}

void read_list(FILE *file, List &list, uint32_t node, uint32_t &bytes) {
    if (node == 0) {
        fread(&(list.listID), sizeof(list.listID), 1, file);
        bytes -= sizeof(list.listID);
    } else {
        list.listID = node;
    }
    fread(&(list.listSize), sizeof(list.listSize), 1, file);
    fread(&(list.listType), sizeof(list.listType), 1, file);
    bytes -= sizeof(list.listSize);
    bytes -= sizeof(list.listType);
}

void skip(FILE *file, int size, uint32_t &bytes) {
    fseek(file, size, SEEK_CUR);
    bytes -= size;
}

void back_move(FILE *file, int size) {
    fseek(file, SEEK_CUR - size, SEEK_SET);
}

uint32_t read_node(FILE *file, uint32_t &bytes) {
    uint32_t node;
    fread(&node, sizeof(node), 1, file);
    if (&bytes != nullptr) bytes -= sizeof(node);
    return node;
}


//====================================================================
//                     AVI RIFF format read functions
//====================================================================
void read_hdrl(FILE *file, AVIMaker *aviMaker) {
    Chunk chunk;
    uint32_t fake = 0;
    read_chunk(file, chunk, 0, fake);
    //TRACE print_chunk(chunk);
    fread(&(aviMaker->aviHeader), sizeof(AVIHeader), 1, file);
    //print_avi_header(this->aviHeader);
    for (size_t stream_id = 0; stream_id < aviMaker->aviHeader.dwStreams; stream_id++) {
        read_strl(file, stream_id, aviMaker);
    }
}

void read_strl(FILE *file, size_t stream_id, AVIMaker *aviMaker) {
    uint32_t node = 0;
    List list;
    Chunk chunk;
    uint32_t type = 0;
    uint32_t bytes = 0;
    read_list(file, list, 0, bytes);
    //TRACE print_list(list);
    bytes = list.listSize - LIST_TYPE_SIZE;
    StreamHeader *streamHeader;
    while (bytes > 0) {
        node = read_node(file, bytes);
        switch (node) {
            case STRH: {
                read_chunk(file, chunk, node, bytes);
                //TRACE print_chunk(chunk);
                streamHeader = new StreamHeader;
                fread(streamHeader, sizeof(StreamHeader), 1, file);
                bytes -= chunk.ckSize;
                type = streamHeader->fccType;
                if (type == VIDS) {
                    VideoStream *videoStream = new VideoStream(stream_id, *streamHeader);
                    aviMaker->videoStreams.push_back(videoStream);
                    //print_stream_header(*this->videoStreams.back()->streamHeader);
                } else if (type == AUDS) {
                    AudioStream *audioStream = new AudioStream;
                    audioStream->streamID = stream_id;
                    audioStream->streamHeader = streamHeader;
                    aviMaker->audioStreams.push_back(audioStream);
                }
                //print_stream_header(this->streamHeader[str_id]);
                break;
            }
            case STRF:
                read_chunk(file, chunk, node, bytes);
                //TRACE print_chunk(chunk);
                bytes -= chunk.ckSize;
                //skip(file, chunk.ckSize);
                if (type == VIDS) {
                    BITMAPINFOHEADER bmInfo;
                    fread(&bmInfo, BITMAP_INFO_SIZE, 1, file);
                    aviMaker->videoStreams.back()->setBITMAPINFOHEADER(bmInfo);
                    //print_bitmap_info(this->m_bmInfo);
                } else if (type == AUDS) {
                    WAVEFORMAT *wave = new WAVEFORMAT;
                    fread(wave, sizeof(WAVEFORMAT), 1, file);
                    aviMaker->audioStreams.back()->wave = wave;
                    //print_wave(this->wave);
                }
                break;
            case JUNK:
                read_chunk(file, chunk, node, bytes);
                //TRACE print_chunk(chunk);
                skip(file, chunk.ckSize, bytes);
                break;
            default:
                //TRACE printf("unknow fourcc strl - ");
                //TRACE print_fourcc(node);
                //TRACE printf("- 0x%x\n", node);
                break;
        };
    }
}

void read_movi(FILE *file, uint32_t bytes, AVIMaker *aviMaker) {
    List list;
    Chunk chunk;
    uint32_t node;
    while (bytes > 0) {
        node = read_node(file, bytes);
        if (read_data(file, node, bytes, aviMaker)) continue;
        switch (node) {
            case LIST:
                //TRACE printf("\n");
                read_list(file, list, node, bytes);
                //TRACE printf("movi\n");
                //TRACE print_list(list);
                //skip(file, list.listSize - LIST_TYPE_SIZE, bytes);
                read_rec(file, list.listSize - LIST_TYPE_SIZE, aviMaker);
                bytes -= list.listSize - LIST_TYPE_SIZE;
                break;
            case VEDT:
                read_chunk(file, chunk, node, bytes);
                //print_chunk(chunk);
                skip(file, chunk.ckSize, bytes);
                break;
            case JUNK:
                read_chunk(file, chunk, node, bytes);
                //TRACE print_chunk(chunk);
                skip(file, chunk.ckSize, bytes);
                break;
            default:
                //TRACE printf("unknow fourcc movi - ");
                //TRACE print_fourcc(node);
                //TRACE printf("- 0x%x\n", node);
                break;
        };
        //TRACE printf("bytes: %d\n", bytes);
        //printf("move round: ");
        //std::cin >> x;
    }
}

bool read_data(FILE *file, uint32_t node, uint32_t &bytes, AVIMaker *aviMaker) {
    uint16_t chunk_data_type = getDataChunkType(node);
    if (chunk_data_type == 0) return false;
    //uint32_t bytesRead = 0;
    Chunk chunk;
    //printf("0x%x    0x%x    0x%x\n", node, node >> 8, (node >> 8) & 0xff);
    uint32_t chunk_data_id = ((node >> 8) & 0xff) - 0x30; // только до 10 потоков (00, 01, ..., 09)
    read_chunk(file, chunk, node, bytes);
    //bytesRead += CHUNK_SIZE;
    //TRACE print_chunk(chunk);
    //printf("chunk_id: %d\n", chunk_data_id);
    //printf("chunk_type: 0x%x\n", chunk_data_type);
    int x;
    //std::cin >> x;
    switch (chunk_data_type) {
        case WB: {
            //printf("audio stream\n");
            AudioStream *aStream = aviMaker->getAudioStreamByID(chunk_data_id);
            if (aStream == nullptr) return false;
            uint8_t *sample = (uint8_t *) malloc(chunk.ckSize);
            fread(sample, chunk.ckSize, 1, file);
            bytes -= chunk.ckSize;
            aStream->samples.push_back(sample);
            break;
        }
        case DB: {
            //printf("video stream\n");
            VideoStream *vStream = aviMaker->getVideoStreamByID(chunk_data_id);
            if (vStream == nullptr) return false;
            TRIPLERGB **rgb = (TRIPLERGB **) malloc(chunk.ckSize);
            //TRACE printf("point: 0x%x\n", rgb);
            BITMAPINFOHEADER bmInfo = aviMaker->videoStreams[chunk_data_id]->bmInfo();
            for (size_t i = 0; i < bmInfo.biHeight; i++) {
                rgb[i] = new TRIPLERGB[bmInfo.biWidth];
                fread(&rgb[i][0], (sizeof(TRIPLERGB) * bmInfo.biWidth), 1, file);
                //fread(&dummy[0], dummy_count, 1, file);
            }
            bytes -= chunk.ckSize;
            vStream->frames.push_back(rgb);
            break;
        }
        default:
            //TRACE printf("unknown id type: 0x%x", chunk_data_type);
            break;
    };
    //std::cin >> x;
    return true;
}

void read_rec(FILE *file, uint32_t bytes, AVIMaker *aviMaker) {
    uint32_t node;
    Chunk chunk;
    while (bytes > 0) {
        node = read_node(file, bytes);
        if (read_data(file, node, bytes, aviMaker)) continue;
        switch (node) {
            case VEDT:
                //bytes -= CHUNK_SIZE;
                read_chunk(file, chunk, node, bytes);
                //TRACE print_chunk(chunk);
                skip(file, chunk.ckSize, bytes);
                //bytes -= chunk.ckSize;
                break;
            case JUNK:
                //bytes -= CHUNK_SIZE;
                read_chunk(file, chunk, node, bytes);
                //TRACE print_chunk(chunk);
                skip(file, chunk.ckSize, bytes);
                //bytes -= chunk.ckSize;
                break;
            default:
                int x;
                //std::cin >> x;
                //TRACE printf("unknow fourcc rec - ");
                //TRACE print_fourcc(node);
                //TRACE printf("- 0x%x\n", node);
                break;
        };
        //printf("bytes: %d\n", bytes);
        //std::cin >> x;
    }
}

uint16_t getDataChunkType(uint32_t node) {
    uint16_t chunkIDtype = uint16_t(node >> 16);
    if ((chunkIDtype != WB) && (chunkIDtype != DB)) {
        //TRACE printf("getDataChunkType faled. node = 0x%x, type = 0x%x\n", node, chunkIDtype);
        return 0;
    }
    return chunkIDtype;
}


//====================================================================
//                     AVI RIFF format write functions
//====================================================================
uint32_t saveHDRL(FILE *file, AVIMaker *aviMaker) {
    fpos_t cur_pos;
    fpos_t pHDRLsize;
    aviMaker->aviHeader.dwStreams = 1;
    fwrite(&LIST, sizeof(LIST), 1, file);
    fgetpos(file, &pHDRLsize);
    //TRACE printf("pHDRLsize: %d\n", pHDRLsize);
    fwrite(&SIZE, sizeof(SIZE), 1, file);
    fwrite(&HDRL_TYPE, sizeof(HDRL_TYPE), 1, file);

    fwrite(&AVIH, sizeof(AVIH), 1, file);
    uint32_t tmp = sizeof(AVIHeader);
    fwrite(&tmp, 4, 1, file);
    fwrite(&aviMaker->aviHeader, sizeof(AVIHeader), 1, file);
    std::vector<VideoStream *>::iterator it = aviMaker->videoStreams.begin();
    for (; it != aviMaker->videoStreams.end(); it++) {
        saveVideoSTRL(file, *it);
    }
    fgetpos(file, &cur_pos);
    __off_t sizeHDRL = cur_pos.__pos - (pHDRLsize.__pos + 4);
    fsetpos(file, &pHDRLsize);
    fwrite(&sizeHDRL, 4, 1, file);
    fsetpos(file, &cur_pos);
}

uint32_t saveVideoSTRL(FILE *file, VideoStream *videoStream) {
    fpos_t pSTRLsize;
    fpos_t cur_pos;
    uint32_t tmp = 0;
    fwrite(&LIST, sizeof(LIST), 1, file);
    fgetpos(file, &pSTRLsize);
    fwrite(&SIZE, sizeof(SIZE), 1, file);
    fwrite(&STRL_TYPE, sizeof(STRL_TYPE), 1, file);

    fwrite(&STRH, sizeof(STRH), 1, file);
    tmp = sizeof(StreamHeader);
    fwrite(&tmp, 4, 1, file);
    //TRACE print_stream_header(videoStream->streamHeader);
    StreamHeader streamHeader = videoStream->streamHeader();
    fwrite(&streamHeader, sizeof(StreamHeader), 1, file);

    fwrite(&STRF, sizeof(STRF), 1, file);
    tmp = BITMAP_INFO_SIZE;
    fwrite(&tmp, 4, 1, file);
    BITMAPINFOHEADER bmInfo = videoStream->bmInfo();
    fwrite(&bmInfo, tmp, 1, file);
    fgetpos(file, &cur_pos);
    __off_t sizeSTRL = cur_pos.__pos - (pSTRLsize.__pos + 4);
    fsetpos(file, &pSTRLsize);
    fwrite(&sizeSTRL, 4, 1, file);
    fsetpos(file, &cur_pos);
    return 0;
}

uint32_t saveVideoREC(FILE *file, TRIPLERGB **rgb, const BITMAPINFOHEADER bmInfo) {
    fpos_t pRECsize;
    fpos_t cur_pos;
    uint32_t tmp = 0;
    fwrite(&LIST, sizeof(LIST), 1, file);
    fgetpos(file, &pRECsize);
    fwrite(&SIZE, sizeof(SIZE), 1, file);
    fwrite(&REC__TYPE, sizeof(REC__TYPE), 1, file);

    fwrite(&DB00, sizeof(DB00), 1, file);
    tmp = bmInfo.biSizeImage;
    fwrite(&tmp, 4, 1, file);
    for (uint i = 0; i < bmInfo.biHeight; i++) {
        fwrite(&rgb[i][0], sizeof(TRIPLERGB) * bmInfo.biWidth, 1, file);
    }
    fgetpos(file, &cur_pos);
    __off_t sizeREC = cur_pos.__pos - (pRECsize.__pos + 4);
    fsetpos(file, &pRECsize);
    fwrite(&sizeREC, 4, 1, file);
    fsetpos(file, &cur_pos);
    return 0;
}

uint32_t saveMOVI(FILE *file, AVIMaker *aviMaker) {
    fpos_t pMOVIsize;
    fpos_t cur_pos;
    fwrite(&LIST, sizeof(LIST), 1, file);
    fgetpos(file, &pMOVIsize);
    fwrite(&SIZE, sizeof(SIZE), 1, file);
    fwrite(&MOVI_TYPE, sizeof(MOVI_TYPE), 1, file);

    auto it = aviMaker->videoStreams[0]->frames.begin();
    for (; it != aviMaker->videoStreams[0]->frames.end(); it++) {
        saveVideoREC(file, *it, aviMaker->videoStreams[0]->bmInfo());
    }
    fgetpos(file, &cur_pos);
    __off_t sizeMOVI = cur_pos.__pos - (pMOVIsize.__pos + 4);
    fsetpos(file, &pMOVIsize);
    fwrite(&sizeMOVI, 4, 1, file);
    fsetpos(file, &cur_pos);
    return 0;
}