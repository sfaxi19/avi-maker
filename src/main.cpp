#include <iostream>
#include "AVIMaker/AVIMaker.h"
#include "motion_compensation/motion_compensation.h"
#include "bmp_lib/bmp.h"
#include "h264/h264.hpp"

void createFrames(VideoStream *videoStream, size_t x, size_t y, uint32_t h, uint32_t w) {
    BITMAPINFOHEADER bmInfo = videoStream->bmInfo();
    int defHeight = bmInfo.biHeight;
    int defWidth = bmInfo.biWidth;
    if (h + x > defHeight) h = (defHeight - x) - 1;
    if (w + y > defWidth) w = (defWidth - y) - 1;
    for (int id_frame = 0; id_frame < videoStream->frames.size(); id_frame++) {
        TRIPLERGB **frame = videoStream->frames[id_frame];
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                frame[h - i][j].red = frame[(defHeight - x - 1) - i][y + j].red;
                frame[h - i][j].green = frame[(defHeight - x - 1) - i][y + j].green;
                frame[h - i][j].blue = frame[(defHeight - x - 1) - i][y + j].blue;
            }
        }
    }
    bmInfo.biHeight = h;
    bmInfo.biWidth = w;
    bmInfo.biSizeImage = h * w * 3;
    videoStream->setBITMAPINFOHEADER(bmInfo);
}

double correletion(TRIPLERGB **mrx, TRIPLERGB **mrx2, int h, int w, int COMPONENT) {
    double Exp = getExpection(mrx, h, w, COMPONENT);
    double MSDR = getMeanSquareDeviation(mrx, h, w, COMPONENT, Exp);
    double r = getCorrelationCoefficient(mrx, mrx2, h, w, COMPONENT, COMPONENT, Exp, Exp);
    return r;
}

void correletion_func(const char *filepath1, const char *filepath2, int component) {
    AVIMaker avi_maker;
    FILE *file_graph = fopen(filepath2, "w");
    avi_maker.readAVIFile(filepath1);
    size_t frames = avi_maker.videoStreams[0]->frames.size();
    size_t step = 1;
    for (int i = 0; i < frames - step; i += step) {
        fprintf(file_graph, "%d,", i);
    }
    fprintf(file_graph, "\n");
    for (int i = 0; i < frames - step; i += step) {
        double r = correletion(avi_maker.videoStreams[0]->frames[i],
                               avi_maker.videoStreams[0]->frames[i + step],
                               avi_maker.videoStreams[0]->bmInfo().biHeight,
                               avi_maker.videoStreams[0]->bmInfo().biWidth,
                               component);
        fprintf(file_graph, "%f,", r);
    }
    fclose(file_graph);
}



void subtract_block(TRIPLEYCbCr **base, TRIPLEYCbCr **target, TRIPLEYCbCr **out, mc::block block, mc::vect v) {
    for (int i = 0; i < block.height; i++) {
        for (int j = 0; j < block.width; j++) {
            out[block.y + i][block.x + j].Y = clip(target[block.y + i][block.x + j].Y -
                                                   base[block.y + v.y + i][block.x + v.x + j].Y + 128, 0, 255);
        }
    }
}

void createDifferenceVideo(const char *filepath) {
    AVIMaker avi_in_file(filepath);
    VideoStream *in_video = avi_in_file.video();
    size_t defHeight = in_video->height();
    size_t defWidth = in_video->width();
    AVIMaker avi_out_file(avi_in_file.aviHeader,
                          new VideoStream(in_video->bmInfo(), in_video->streamHeader()));
    mc::block block(0, 0, 16, 16);
    auto **frameSimpDiff = new TRIPLEYCbCr *[defHeight];
    for (size_t i = 0; i < in_video->height(); i++) {
        frameSimpDiff[i] = new TRIPLEYCbCr[defWidth];
    }
    printf("%10s%20s%20s%20s%20s%20s%20s\n", "Frame ID", "DiffDeviation", "TargetDeviation", "EntropyDiff",
           "EntropySimpleDiff", "EntropyTarget", "Type");
    avi_out_file.addCopyFrame(0, in_video->getFrame(0));
    for (size_t frame_id = 1; frame_id < in_video->getLength(); frame_id++) {
        TRIPLEYCbCr **frameBase = RGB2YCbCr(in_video->getFrame(frame_id - 1), defHeight, defWidth);
        TRIPLEYCbCr **frameTarget = RGB2YCbCr(in_video->getFrame(frame_id), defHeight, defWidth);
        auto **frameOut = new TRIPLEYCbCr *[defHeight];
        for (size_t i = 0; i < defHeight; i++) {
            frameOut[i] = new TRIPLEYCbCr[defWidth];
        }
        size_t n_blocks_in_height = (size_t) ceil((double) in_video->height() / block.height);
        size_t n_blocks_in_width = (size_t) ceil((double) in_video->width() / block.width);
        size_t width_end_blocks = in_video->width() % block.width;
        if (width_end_blocks == 0) width_end_blocks = block.width;
        size_t height_end_blocks = in_video->height() % block.height;
        if (height_end_blocks == 0) height_end_blocks = block.height;
        for (size_t i = 0; i < n_blocks_in_height; i++) {
            for (size_t j = 0; j < n_blocks_in_width; j++) {
                mc::block cur_block = block;
                cur_block.x = j * block.width;
                cur_block.y = i * block.height;
                if (i == n_blocks_in_height - 1) cur_block.height = height_end_blocks;
                if (j == n_blocks_in_width - 1) cur_block.width = width_end_blocks;
                mc::vect v_res = mc::logarithmicSearch(frameBase, frameTarget, defHeight, defWidth, cur_block);
                subtract_block(frameBase, frameTarget, frameOut, cur_block, v_res);
            }
        }
        subtract_block(frameBase, frameTarget, frameSimpDiff, mc::block(0, 0, defHeight, defWidth), mc::vect(0, 0));

        int *freq;
        freq = getHistFreqFromTriple((TRIPLERGB **) frameOut, defHeight, defWidth, COMPONENT_Y,
                                     "histograms/MC/diff/",
                                     "frameOut.csv");
        double entropyDiff = getEntropy(freq, defHeight * defWidth);
        delete[] freq;
        freq = getHistFreqFromTriple((TRIPLERGB **) frameTarget, defHeight, defWidth, COMPONENT_Y,
                                     "histograms/MC/orig/",
                                     "frameTarget.csv");
        double entropyTarget = getEntropy(freq, defHeight * defWidth);
        delete[] freq;
        freq = getHistFreqFromTriple((TRIPLERGB **) frameSimpDiff, defHeight, defWidth, COMPONENT_Y,
                                     "histograms/MC/simp_diff/",
                                     "frameSimpDiff.csv");
        double entropySimpDiff = getEntropy(freq, defHeight * defWidth);
        delete[] freq;
        if (entropyDiff < 5.0) {
            TRIPLERGB **rgbOutFrame = Y2RGB(frameOut, defHeight, defWidth);
            avi_out_file.addFrame(0, rgbOutFrame);
        } else {
            TRIPLERGB **rgbOutFrame = Y2RGB(frameTarget, defHeight, defWidth);
            avi_out_file.addFrame(0, rgbOutFrame);
        }
        double targetExp = getExpection((TRIPLERGB **) frameTarget, defHeight, defWidth, COMPONENT_Y);
        double targetDev = getMeanSquareDeviation((TRIPLERGB **) frameTarget, defHeight, defWidth, COMPONENT_Y,
                                                  targetExp);
        double diffExp = getExpection((TRIPLERGB **) frameOut, defHeight, defWidth, COMPONENT_Y);
        double diffDev = getMeanSquareDeviation((TRIPLERGB **) frameOut, defHeight, defWidth, COMPONENT_Y,
                                                diffExp);
        printf("%10lu%20f%20f%20f%20f%20f%20s\n",
               frame_id,
               diffDev, targetDev, entropyDiff,
               entropySimpDiff, entropyTarget, (entropyDiff >= 5.0) ? "I/P" : " B ");
    }
    for (size_t i = 0; i < defHeight; i++) {
        delete[] frameSimpDiff[i];
    }
    //print_bitmap_info(avi_out_file.video()->m_bmInfo);
    avi_out_file.saveVideoStreamToBMP("cm_files");
    avi_out_file.saveAVIFile("diff.avi");
}

void test1(const char *filepath1, const char *filepath2, const char *filepath3) {
    system("mkdir -p bmp_files/");
    AVIMaker avi_maker;
    AVIMaker avi_maker2;
    avi_maker.readAVIFile(filepath1);
    avi_maker2.readAVIFile(filepath2);
    avi_maker.saveVideoStreamToBMP("bmp_files/");
    correletion_func(filepath1, "charts/red/graph1.csv", COMPONENT_RED);
    correletion_func(filepath2, "charts/red/graph2.csv", COMPONENT_RED);
    correletion_func(filepath3, "charts/red/graphDictor.csv", COMPONENT_RED);

    correletion_func(filepath1, "charts/green/graph1.csv", COMPONENT_GREEN);
    correletion_func(filepath2, "charts/green/graph2.csv", COMPONENT_GREEN);
    correletion_func(filepath3, "charts/green/graphDictor.csv", COMPONENT_GREEN);

    correletion_func(filepath1, "charts/blue/graph1.csv", COMPONENT_BLUE);
    correletion_func(filepath2, "charts/blue/graph2.csv", COMPONENT_BLUE);
    correletion_func(filepath3, "charts/blue/graphDictor.csv", COMPONENT_BLUE);

    avi_maker.saveAVIFile("test_save.avi");
    //Read and write video after reverse
    AVIMaker avi_maker3;
    avi_maker3.readAVIFile(filepath2);
    avi_maker3.reverseVideoStream(0);
    avi_maker3.saveAVIFile("reverse.avi");
}

void h264_test(const char *filepath) {
    AVIMaker aviMaker(filepath);
    uint8_t *bytes = 0;
    size_t len = 0;
    //print_bitmap_info(aviMaker.video()->bmInfo());
    printf("w_blocks_count: %f\nh_blocks_count: %f\n", (float) aviMaker.video()->width() / 4,
           (float) aviMaker.video()->height() / 4);
    avi_to_h264(&bytes, len, aviMaker);
}

int main() {
    //test1("resources/lr1_1.AVI", "resources/lr1_2.AVI");
    //createDifferenceVideo("../resources/lr1_2.AVI");
    h264_test("../resources/lr1_1.AVI");
    std::cout << "finish\n" << std::endl;
    return 0;
}