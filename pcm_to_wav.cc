#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct WAVE_HEADER
{
    char fccID[4];       /* 'RIFF' */
    unsigned int cbSize; /* 整个文件大小 - 8 */
    char fccType[4];     /* 'WAVE' */
} WAVE_HEADER;

typedef struct WAVE_FMT
{
    char fccID[4];                  /* 'fmt ' */
    unsigned int dwSize;            /* 16 */
    unsigned short wFormatTag;      /* 编码方式， 1: PCM */
    unsigned short wChannels;       /* 通道数， 1: 单声道， 2: 双声道 */
    unsigned int dwSamplesPerSec;   /* 采样率 */
    unsigned int dwAvgBytesPerSec;  /* 采样率 * 通道数 * 采样位数 / 8 */
    unsigned short wBlockAlign;     /* 通道数 * 采样位数 / 8 */
    unsigned short uiBitsPerSample; /* 采样位数 */
} WAVE_FMT;

typedef struct WAVE_DATA
{
    char fccID[4];       /* 'data' */
    unsigned int cbSize; /* 音频数据的大小 */
} WAVE_DATA;

// pcm数据转wav数据
int simple_pcms16le_to_wav(const char *pcm_file, const char *wav_file, int sample_rate, int channels)
{
    FILE *fp_pcm = fopen(pcm_file, "rb+");
    FILE *fp_wav = fopen(wav_file, "wb+");
    if (fp_pcm == NULL || fp_wav == NULL)
    {
        return -1;
    }

    WAVE_HEADER wave_header = {0};
    WAVE_FMT wave_fmt = {0};
    WAVE_DATA wave_data = {0};

    memcpy(wave_header.fccID, "RIFF", 4);
    memcpy(wave_header.fccType, "WAVE", 4);
    memcpy(wave_fmt.fccID, "fmt ", 4); // 记住这里是四个字节 后面有一个空格
    memcpy(wave_data.fccID, "data", 4);

    wave_header.cbSize = 0;
    fseek(fp_wav, sizeof(WAVE_HEADER), SEEK_CUR);

    wave_fmt.dwSize = 16;
    wave_fmt.wFormatTag = 1;
    wave_fmt.wChannels = channels;
    wave_fmt.dwSamplesPerSec = sample_rate;
    wave_fmt.uiBitsPerSample = 16;
    wave_fmt.dwAvgBytesPerSec = wave_fmt.dwSamplesPerSec * wave_fmt.wChannels * wave_fmt.uiBitsPerSample / 8;
    wave_fmt.wBlockAlign = wave_fmt.wChannels * wave_fmt.uiBitsPerSample / 8;

    fwrite(&wave_fmt, sizeof(WAVE_FMT), 1, fp_wav);

    wave_data.cbSize = 0;
    fseek(fp_wav, sizeof(WAVE_DATA), SEEK_CUR);

    unsigned int data_size = 0;
    unsigned short data = 0;

    fread(&data, sizeof(unsigned short), 1, fp_pcm);
    while (!feof(fp_pcm))
    {
        data_size += sizeof(unsigned short);
        fwrite(&data, sizeof(unsigned short), 1, fp_wav);
        fread(&data, sizeof(unsigned short), 1, fp_pcm);
    }

    wave_data.cbSize = data_size;
    wave_header.cbSize = wave_data.cbSize + sizeof(WAVE_FMT) + sizeof(WAVE_DATA) + sizeof(WAVE_HEADER) - 8;
    // wave_header.cbSize = wave_data.cbSize + 44;

    rewind(fp_wav);
    fwrite(&wave_header, sizeof(WAVE_HEADER), 1, fp_wav);
    fseek(fp_wav, sizeof(WAVE_FMT), SEEK_CUR);
    fwrite(&wave_data, sizeof(WAVE_DATA), 1, fp_wav);

    fclose(fp_pcm);
    fclose(fp_wav);
    return 0;
}

int main()
{
    simple_pcms16le_to_wav("../res/motion.pcm", "../res/motion_pcm_to_wav.wav", 44100, 2);
    return 0;
}