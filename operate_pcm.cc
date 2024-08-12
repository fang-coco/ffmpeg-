#include <stdio.h>
#include <stdlib.h>

// 分离左右声道
int simplest_pcms16le_split(char * pcm_file, char *pcml, char *pcmr) {
    FILE *fp = fopen(pcm_file, "rb+");
    FILE *fpl = fopen(pcml, "wb+");
    FILE *fpr = fopen(pcmr, "wb+");

    if (fp == NULL || fpl == NULL || fpr == NULL) {
        return -1;
    }

    unsigned char* buf = (unsigned char*)malloc(4);
    if (buf == NULL) {
        return -1;
    }

    while (!feof(fp)) {
        fread(buf, 1, 4, fp);
        fwrite(buf, 1, 2, fpl);
        fwrite(buf + 2, 1, 2, fpr);
    }
    fclose(fp);
    fclose(fpl);
    fclose(fpr);
    free(buf);
    return 0;
}

// 左声道减半
int simplest_pcms16le_half(char * pcm_file, char *pcm_half) {
    FILE *fp = fopen(pcm_file, "rb+");
    FILE *fph = fopen(pcm_half, "wb+");

    if (fp == NULL || fph == NULL) {
        return -1;
    }

    unsigned char* buf = (unsigned char*)malloc(4);
    if(buf == NULL) {
        return -1;
    }

    while (!feof(fp)) {
        fread(buf, 1, 4, fp);
        short *buf_tmp = (short *) buf;
        *buf_tmp = *buf_tmp / 2;
        fwrite(buf, 1, 2, fph);
        fwrite(buf + 2, 1, 2, fph);
    }   
    fclose(fp);
    fclose(fph);
    free(buf);
    return 0;
}

// 将速度提升一倍
int simplest_pcms16le_speed(char * pcm_file, char *pcm_speed) {
    FILE *fp = fopen(pcm_file, "rb+");
    FILE *fps = fopen(pcm_speed, "wb+");

    if (fp == NULL || fps == NULL) {
        return -1;
    }

    unsigned char* buf = (unsigned char*)malloc(4);
    if(buf == NULL) {
        return -1;
    }
    int flag = 0;
    while (!feof(fp)) {
        fread(buf, 1, 4, fp);
        if (flag % 2 == 0) {
            fwrite(buf, 1, 4, fps);
        }
        flag++;
    }
    fclose(fp);
    fclose(fps);
    free(buf);
    return 0;
}

// 转换为8bit
int simplest_pcms16le_to_8bit(char * pcm_file, char *pcm_8bit) {
    FILE *fp = fopen(pcm_file, "rb+");
    FILE *fp8 = fopen(pcm_8bit, "wb+");

    if (fp == NULL || fp8 == NULL) {
        return -1;
    }

    unsigned char* buf = (unsigned char*)malloc(4);

    if (buf == NULL) {
        return -1;
    }

    while (!feof(fp)) {
        fread(buf, 1, 4, fp);
        short *buf_tmp = (short *) buf;
        unsigned char buf_tmp_8 = (*buf_tmp) >> 8 + 128;
        fwrite(&buf_tmp_8, 1, 1, fp8);
        buf_tmp_8 = (*buf_tmp + 2) >> 8 + 128;
        fwrite(&buf_tmp_8, 1, 1, fp8);
    }
    fclose(fp);
    fclose(fp8);
    free(buf);
    return 0;
}
int main() {
    // simplest_pcms16le_split("../res/motion.pcm", "../res/motion_l.pcm", "../res/motion_r.pcm");
    // simplest_pcms16le_half("../res/motion.pcm", "../res/motion_half.pcm");
    // simplest_pcms16le_speed("../res/motion.pcm", "../res/motion_speed.pcm");
    simplest_pcms16le_to_8bit("../res/motion.pcm", "../res/motion_8bit.pcm");
}