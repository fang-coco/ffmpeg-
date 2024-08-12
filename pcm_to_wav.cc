
typedef struct WAVE_HEADER
{
    char fccID[4]; /* 'RIFF' */
    unsigned int cbSize; /* 整个文件大小 - 8 */
    char fccType[4]; /* 'WAVE' */
}WAVE_HEADER;

typedef struct WAVE_FMT
{
    char fccID[4]; /* 'fmt ' */
    unsigned int dwSize;    /* 16 */
    unsigned short wFormatTag; /* 编码方式， 1: PCM */
    unsigned short wChannels; /* 通道数， 1: 单声道， 2: 双声道 */
    unsigned int dwSamplesPerSec; /* 采样率 */
    unsigned int dwAvgBytesPerSec; /* 采样率 * 通道数 * 采样位数 / 8 */
    unsigned short wBlockAlign; /* 通道数 * 采样位数 / 8 */
    unsigned short uiBitsPerSample; /* 采样位数 */
}WAVE_FMT;

typedef struct WAVE_DATA
{
    char fccID[4]; /* 'data' */
    unsigned int cbSize; /* 音频数据的大小 */
}WAVE_DATA;