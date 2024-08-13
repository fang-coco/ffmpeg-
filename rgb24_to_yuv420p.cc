#include <stdio.h>
#include <stdlib.h>
/*
    rgb像素格式转yuv420p像素格式，公式：
        Y = 0.299R + 0.587G + 0.114B
        U = -0.169R - 0.331G + 0.500B + 128
        V = 0.500R - 0.419G - 0.081B + 128
    note:
        rgb240p像素格式： 8:8:8 (Packed)
        yuv420p像素格式： 8:2:2/4:2:0 (Planar)
*/

/*
Packed
定义：在“packed”格式中，所有通道的数据都是紧密排列在一起的，通常按照像素顺序存储。例如，在RGB图像中，每个像素的R、G、B值连续存储在一起，形成一个像素单元，随后是下一个像素的RGB值，依此类推。

示例：
假设有一个 3x3 的 RGB 图像，其数据布局如下：
R1 G1 B1 R2 G2 B2 R3 G3 B3
R4 G4 B4 R5 G5 B5 R6 G6 B6
R7 G7 B7 R8 G8 B8 R9 G9 B9
在“packed”格式中，这些数据将被存储为：
R1 G1 B1 R2 G2 B2 R3 G3 B3 R4 G4 B4 R5 G5 B5 R6 G6 B6 R7 G7 B7 R8 G8 B8 R9 G9 B9


Planar
定义：在“planar”格式中，数据按通道分开存储，每个通道的数据形成一个独立的平面。例如，在RGB图像中，所有的R值、G值和B值分别存储在三个不同的数组或平面中。

示例：
使用上面的 3x3 RGB 图像作为示例，其数据布局如下：
R1 G1 B1 R2 G2 B2 R3 G3 B3
R4 G4 B4 R5 G5 B5 R6 G6 B6
R7 G7 B7 R8 G8 B8 R9 G9 B9
在“planar”格式中，这些数据将被存储为三个独立的平面：
R1 R2 R3 R4 R5 R6 R7 R8 R9
G1 G2 G3 G4 G5 G6 G7 G8 G9
B1 B2 B3 B4 B5 B6 B7 B8 B9

总结
Packed 格式通常用于大多数图像处理库和应用程序中，因为它允许快速访问单个像素的所有通道值，并且便于进行常见的图像操作。
Planar 格式则更适合于某些特定的应用场景，例如视频处理和高性能计算，其中可能需要单独处理每个通道的数据。

应用场景
Packed 格式适用于：
大多数图形库（如OpenGL、DirectX）。
通用图像处理任务（如滤波、缩放）。
Planar 格式适用于：
视频编码/解码，特别是当需要独立处理每个颜色平面时。
高性能计算，特别是在并行处理环境中，可以针对每个通道独立优化。
在实际应用中，选择哪种格式取决于具体的需求和上下文。例如，如果你正在开发一个图形渲染引擎，可能会倾向于使用“packed”格式；而如果你正在编写一个视频编码器，那么“planar”格式可能更为合适。
*/

#define RGB_TO_YUV_Y(r, g, b) (0.299 * r + 0.587 * g + 0.114 * b)
#define RGB_TO_YUV_U(r, g, b) (-0.169 * r - 0.331 * g + 0.500 * b + 128)
#define RGB_TO_YUV_V(r, g, b) (0.500 * r - 0.419 * g - 0.081 * b + 128)
#define CLIP_VALUE(x) (((x) < 0) ? 0 : (((x) > 255) ? 255 : (x)))
int simple_rgb_to_yuv420p(const char *rgb, const char *yuv, int width, int height)
{

    FILE *fp_yuv = fopen(yuv, "wb+");
    if (fp_yuv == NULL)
    {
        printf("open yuv file failed!\n");
        return -1;
    }

    FILE *fp_rgb = fopen(rgb, "rb+");
    if (fp_rgb == NULL)
    {
        printf("open rgb file failed!\n");
        return -1;
    }

    int bits = width * height;

    unsigned char *rgb_buf = (unsigned char *)malloc(bits * 3);
    unsigned char *yuv_buf = (unsigned char *)malloc(bits * 3 / 2);

    if (rgb_buf == NULL || yuv_buf == NULL)
    {
        printf("malloc failed!\n");
        return -1;
    }

    fread(rgb_buf, 1, bits * 3, fp_rgb);

    unsigned char *y_buf = yuv_buf;
    unsigned char *u_buf = yuv_buf + bits;
    unsigned char *v_buf = yuv_buf + bits * 5 / 4;
    unsigned char r, g, b;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            int index = (h * width + w) * 3;
            r = rgb_buf[index * 3];
            g = rgb_buf[index * 3 + 1];
            b = rgb_buf[index * 3 + 2];
            *(y_buf++) = CLIP_VALUE(RGB_TO_YUV_Y(r, g, b));
            if (h % 2 == 0 && w % 2 == 0)
            {
                *(u_buf++) = CLIP_VALUE(RGB_TO_YUV_U(r, g, b));
            }
            else if (h % 2 == 0)
            {
                *(v_buf++) = CLIP_VALUE(RGB_TO_YUV_V(r, g, b));
            }
        }
    }

    fwrite(yuv_buf, 1, bits * 3 / 2, fp_yuv);
    fclose(fp_yuv);
    fclose(fp_rgb);
    free(rgb_buf);
    free(yuv_buf);
    return 0;
}

int main()
{
    simple_rgb_to_yuv420p("../res/longmao_rgb24.rgb", "../res/longmao_yuv420pB.yuv", 1680, 1050);
}