#include <stdio.h>
#include <stdlib.h>
/*
    mbp文件格式:
        BITMAPFILEHEADER // 14字节
        BITMAPINFOHEADER // 40字节
        RGB像素格式
    note:
        BMP采用的是小端存储方式，故需要把rgb24格式中的r和b顺序调换
        如果直接将rgb像素数据写入bmp文件， 那么bmp图片会变成原图片的镜像，这也是由于bmp的存储方式而造成的
*/
// 这里必须控制结构体字节不对齐，不然解析不出bmp文件
#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER
{
    unsigned short int bfType;        /* 位图文件的类型， 必须为"BM" */
    unsigned int bfSize;         /* 文件大小， 以字节为单位 */
    unsigned short int bfReserved1; /* 保留字， 必须为0 */
    unsigned short int bfReserved2; /* ... */
    unsigned int bfOffBits;      /* 文件头到数据的偏移量， 以字节为单位 */
} BITMAPFILEHEADER ;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER
{
    int biSize;                  /* 本结构所占用的字节数, 字节为单位 */
    int biWidth;                  /* 位图的宽度，以像素为单位 */
    int biHeight;                  /* 位图的高度，以像素为单位 */
    short int biplPlanes;          /* 位图的平面数， 必须为1 */
    short int biBitCount;          /* 颜色深度， 每个像素所需要的位数 */
    int biCompression;      /* 位图的压缩方式 */
    int biSizeImage;              /* 位图的大小， 以字节为单位 */
    int biXPelsPerMeter;  /* 位图水平分辨率，每米像素数 */
    int biYPelsPerMeter;  /* 位图垂直分辨率，每米像素数 */
    int biClrUsed;                /* 位图实际使用的颜色表中的颜色数 */
    int biClrImportant;          /* 位图显示过程中重要的颜色数 */
} BITMAPINFOHEADER;

int simple_rgb24_to_bmp(const char *rgb24_file, const char *bmp_file, int width, int height)
{

    FILE *fp_rgb24 = fopen(rgb24_file, "rb");    
    if (fp_rgb24 == NULL)
    {
        printf("open rgb24 file failed\n");
        return -1;
    }

    FILE *fp_bmp = fopen(bmp_file, "wb");
    if (fp_bmp == NULL)
    {
        printf("open bmp file failed\n");
        return -1;
    }

    int bits = width * height;
    unsigned char *p_rgb24 = (unsigned char *)malloc(bits * 3);
    if (p_rgb24 == NULL)
    {
        printf("malloc failed\n");
        return -1;
    }
    fread(p_rgb24, 1, bits * 3, fp_rgb24);

    BITMAPFILEHEADER bfh = {0};
    BITMAPINFOHEADER bih = {0};
    int header_size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    // printf("size: %ld\t%ld\n", sizeof(BITMAPFILEHEADER) , sizeof(BITMAPINFOHEADER)); //size: 14        40
    bfh.bfOffBits = header_size;
    bfh.bfSize = header_size + bits * 3;
    bfh.bfType = 0x4d42; // "BM"

    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biBitCount = 24;
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biSizeImage = bits * 3;
    bih.biCompression = 0;
    bih.biplPlanes = 1;
    // bih.biXPelsPerMeter = 0;
    // bih.biYPelsPerMeter = 0;

    fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp_bmp);
    fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp_bmp);

    // 交换r和b
    for (int i = 0; i < bits * 3; i += 3) {
        unsigned char tmp = p_rgb24[i];
        p_rgb24[i] = p_rgb24[i + 2];
        p_rgb24[i + 2] = tmp;
    }

    /* 因为bmp存储方式为小端存储，所以需要颠倒像素数据, 不然是镜像的 */
    // 颠倒像素数据
    for (int y = 0; y < height / 2; ++y) {
        unsigned char *row_start = p_rgb24 + y * width * 3;
        unsigned char *row_end = p_rgb24 + (height - 1 - y) * width * 3;
        for (int x = 0; x < width * 3; ++x) {
            unsigned char tmp = row_start[x];
            row_start[x] = row_end[x];
            row_end[x] = tmp;
        }
    }

    fwrite(p_rgb24, 1, bits * 3, fp_bmp);
    fclose(fp_rgb24);
    fclose(fp_bmp);
    return 0;
}

int main() {
    simple_rgb24_to_bmp("../res/longmao_rgb24.rgb", "../res/longmao.bmp", 1680, 1050);
}

