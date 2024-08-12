#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>

// 分离y,u,v数据 yuv420p
int simplest_yuv420p_split(char *yuv_file, char *y_file, char *u_file, char *v_file, int width, int height) {
    FILE *fp_yuv = fopen(yuv_file, "rb+");
    FILE *fp_y = fopen(y_file, "wb+");
    FILE *fp_u = fopen(u_file, "wb+");
    FILE *fp_v = fopen(v_file, "wb+");
    if (fp_yuv == NULL || fp_y == NULL || fp_u == NULL || fp_v == NULL) {
        printf("open file error!\n");
        return -1;
    }

    int bits = width * height;
    unsigned char *yuv = (unsigned char *) malloc(bits * 3 / 2);
    int n = fread(yuv, 1, bits * 3 / 2, fp_yuv);
    fwrite(yuv, 1, bits, fp_y);
    fwrite(yuv + bits, 1, bits / 4, fp_u);
    fwrite(yuv + bits + bits / 4, 1, bits / 4, fp_v);

    fclose(fp_yuv);
    fclose(fp_y);
    fclose(fp_u);
    fclose(fp_v);
    free(yuv);
    return 0;
}
// 灰度图
int simplest_yuv420p_gray(char *yuv_file, char *yuv_gray_file, int width, int height) {
    FILE *fp_yuv = fopen(yuv_file, "rb+");
    FILE *fp_yuv_gray = fopen(yuv_gray_file, "wb+");
    if (fp_yuv == NULL || fp_yuv_gray == NULL) {
        printf("open file error!\n");
        return -1;
    }

    int bits = width * height;
    unsigned char *yuv = (unsigned char *) malloc(bits * 3 / 2);
    int n = fread(yuv, 1, bits * 3 / 2, fp_yuv);
    memset(yuv + bits, 128, bits / 2); // 将U V置为128, 灰度图
    fwrite(yuv, 1, bits * 3 / 2, fp_yuv_gray);

    fclose(fp_yuv);
    fclose(fp_yuv_gray);
    free(yuv);
    return 0;
}
// 亮度减半
int simplest_yuv420p_half(char *yuv_file, char *yuv_gray_file, int width, int height) {
    FILE *fp_yuv = fopen(yuv_file, "rb+");
    FILE *fp_yuv_gray = fopen(yuv_gray_file, "wb+");
    if (fp_yuv == NULL || fp_yuv_gray == NULL) {
        printf("open file error!\n");
        return -1;
    }

    int bits = width * height;
    unsigned char *yuv = (unsigned char *) malloc(bits * 3 / 2);
    int n = fread(yuv, 1, bits * 3 / 2, fp_yuv);

    for (int i = 0; i < bits; i++) {
        unsigned char y = yuv[i];
        y = y / 2;
        yuv[i] = y;
    }
    fwrite(yuv, 1, bits * 3 / 2, fp_yuv_gray);

    fclose(fp_yuv);
    fclose(fp_yuv_gray);
    free(yuv);
    return 0;
}

// 添加边框
int simplest_yuv420p_add_border(char *yuv_file, char *yuv_border_file, int width, int height, int border) {
    FILE *fp_yuv = fopen(yuv_file, "rb+");
    FILE *fg_yuv_border = fopen(yuv_border_file, "wb+");
    if (fp_yuv == NULL || fg_yuv_border == NULL) {
        printf("open file error!\n");
        return -1;
    }

    int bits = width * height;
    unsigned char *yuv = (unsigned char *) malloc(bits * 3 / 2);
    if (yuv == NULL) {
        printf("malloc error!\n");
        return -1;
    }
    int n = fread(yuv, 1, bits * 3 / 2, fp_yuv);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i < border || i >= height - border || j < border || j >= width - border) {
                // yuv[i * width + j] = 0; //黑框
                yuv[i * width + j] = 255; //白框
            }
        }
    }
    fwrite(yuv, 1, bits * 3 / 2, fg_yuv_border);
    free(yuv);
    fclose(fp_yuv);
    fclose(fg_yuv_border);
    return 0;
}

// 灰阶测试图
int simplest_yuv420p_graybar(char *yuv_graybar_file, int width, int height, int ymin, int ymax, int barnum) {
    int bits = width * height;
    unsigned char *data_y = (unsigned char *) malloc(bits);
    unsigned char *data_u = (unsigned char *) malloc(bits / 4);
    unsigned char *data_v = (unsigned char *) malloc(bits / 4);

    FILE *fp_yuv_graybar = fopen(yuv_graybar_file, "wb+");
    if (fp_yuv_graybar == NULL || data_y == NULL || data_u == NULL || data_v == NULL) {
        printf("open file error!\n");
        return -1;
    }

    int bar_width = width / barnum;
    int lum_inc = (float)(ymax - ymin) / (barnum - 1);

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            int t = i / bar_width;
            data_y[j * width + i] = ymin + (char)t * lum_inc;
        }
    }

    for (int j = 0; j < height / 2; ++j) {
        for (int i = 0; i < width / 2; ++i) {
            int t = i / bar_width;
            data_u[j * width / 2 + i] = 128;
        }
    }

    for (int j = 0; j < height / 2; ++j) {
        for (int i = 0; i < width / 2; ++i) {
            int t = i / bar_width;
            data_v[j * width / 2 + i] = 128;
        }
    }

    fwrite(data_y, 1, bits, fp_yuv_graybar);
    fwrite(data_u, 1, bits / 4, fp_yuv_graybar);
    fwrite(data_v, 1, bits / 4, fp_yuv_graybar);
    fclose(fp_yuv_graybar);
    free(data_y);
    free(data_u);
    free(data_v);
    return 0;
}

// 计算像素数据的PSNR
int simplest_yuv420p_psnr(char *yuv_file1, char *yuv_file2, int width, int height) {
    FILE *fp_yuv1 = fopen(yuv_file1, "rb+");
    FILE *fp_yuv2 = fopen(yuv_file2, "rb+");
    if (fp_yuv1 == NULL || fp_yuv2 == NULL) {
        printf("open file error!\n");
    }

    int bits = width * height;
    unsigned char *pic1 = (unsigned char *) malloc(bits);
    unsigned char *pic2 = (unsigned char *) malloc(bits);

    if (pic1 == NULL || pic2 == NULL) {
        printf("malloc error!\n");
        return -1;
    }

    fread(pic1, 1, bits, fp_yuv1);
    fread(pic2, 1, bits, fp_yuv2);

    double mse_sum, mse, psnr;
    mse_sum = mse = psnr = 0;
    for (int i = 0; i < bits; i++) {
        mse_sum += pow(pic1[i] - pic2[i], 2);
    }

    mse = mse_sum / (double) bits;

    psnr = 10 * log10(255 * 255 / mse);

    printf("PSNR: %f\n", psnr);

    free(pic1);
    free(pic2);
    fclose(fp_yuv1);
    fclose(fp_yuv2);
    return 0;
}
// 分离y,u,v数据 yuv444p
int simplest_yuv444p_split(char *yuv_file, char *y_file, char *u_file, char *v_file, int width, int height) {
    FILE *fp_yuv = fopen(yuv_file, "rb+");
    FILE *fp_y = fopen(y_file, "wb+");
    FILE *fp_u = fopen(u_file, "wb+");
    FILE *fp_v = fopen(v_file, "wb+");
    if (fp_yuv == NULL || fp_y == NULL || fp_u == NULL || fp_v == NULL) {
        printf("open file error!\n");
        return -1;
    }

    int bits = width * height;
    unsigned char *yuv = (unsigned char *) malloc(bits * 3);
    int n = fread(yuv, 1, bits * 3, fp_yuv);
    fwrite(yuv, 1, bits, fp_y);
    fwrite(yuv + bits, 1, bits , fp_u);
    fwrite(yuv + bits + bits, 1, bits, fp_v);

    fclose(fp_yuv);
    fclose(fp_y);
    fclose(fp_u);
    fclose(fp_v);
    free(yuv);
    return 0;
}

// 分离rgb24的r, g, b数据
int simplest_rgb24_split(char *rgb_file, char *r_file, char *g_file, char *b_file, int width, int height) {
    FILE *fp_rgb = fopen(rgb_file, "rb+");
    FILE *fp_r = fopen(r_file, "wb+");
    FILE *fp_g = fopen(g_file, "wb+");
    FILE *fp_b =fopen(b_file, "wb+");
    if (fp_rgb == NULL || fp_r == NULL || fp_g == NULL || fp_b == NULL) {
        printf("open file error!\n");
        return -1;
    }
    
    int bits = width * height;

    unsigned char *rgb = (unsigned char *) malloc(bits * 3);
    if (rgb == NULL) {
        printf("malloc error!\n");
        return -1;
    }
    fread(rgb, 1, bits * 3, fp_rgb);

    for (int i = 0; i < bits * 3; i += 3) {
        fwrite(rgb + i, 1, 1, fp_r);        //R
        fwrite(rgb + i + 1, 1, 1, fp_g);    //G
        fwrite(rgb + i + 2, 1, 1, fp_b);    //B
    }

    free(rgb);
    fclose(fp_rgb);
    fclose(fp_r);
    fclose(fp_g);
    fclose(fp_b);
    return 0;
}

// 生成RGB24格式的彩条测试图
int simplest_rgb24_colorbar(char *out_file, int width, int height) {
    FILE *fp_out = fopen(out_file, "wb+");
    if (fp_out == NULL) {
        printf("open file error!\n");
        return -1;
    }

    int barwidth = width / 8;
    int bits = width * height;
    unsigned char *data = (unsigned char *) malloc(bits * 3);

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            int t = w / barwidth;
            switch (t)
            {
            case 0: {
                data[(h * width + w) * 3] = 255;
                data[(h * width + w) * 3 + 1] = 255;
                data[(h * width + w) * 3 + 2] = 255;
                break;
            }
            case 1: {
                data[(h * width + w) * 3] = 255;
                data[(h * width + w) * 3 + 1] = 255;
                data[(h * width + w) * 3 + 2] = 0;
                break;
            }
            
            case 2: {
                data[(h * width + w) * 3] = 0;
                data[(h * width + w) * 3 + 1] = 255;
                data[(h * width + w) * 3 + 2] = 255;
                break;
            }
            case 3: {
                data[(h * width + w) * 3] = 0;
                data[(h * width + w) * 3 + 1] = 255;
                data[(h * width + w) * 3 + 2] = 0;
                break;
            }
            case 4: {
                data[(h * width + w) * 3] = 255;
                data[(h * width + w) * 3 + 1] = 0;
                data[(h * width + w) * 3 + 2] = 255;
                break;
            }
            case 5: {
                data[(h * width + w) * 3] = 255;
                data[(h * width + w) * 3 + 1] = 0;
                data[(h * width + w) * 3 + 2] = 0;
                break;
            }
            case 6: {
                data[(h * width + w) * 3] = 0;
                data[(h * width + w) * 3 + 1] = 0;
                data[(h * width + w) * 3 + 2] = 255;
                break;
            }
            case 7: {
                data[(h * width + w) * 3] = 0;
                data[(h * width + w) * 3 + 1] = 0;
                data[(h * width + w) * 3 + 2] = 0;
                break;
            }
            default:
                break;
            }
        }
    }
    fwrite(data, 1, bits * 3, fp_out);
    fclose(fp_out);
    free(data);
    return 0;
}

int main() {
    // simplest_yuv420p_split("../res/longmao_420p.yuv", "../res/longmao_420p_y.yuv", "../res/longmao_420p_u.yuv", "../res/longmao_420p_v.yuv", 1680, 1050);
    // simplest_yuv444p_split("../res/longmao444.yuv", "../res/longmao444_y.yuv", "../res/longmao444_u.yuv", "../res/longmao444_v.yuv", 1680, 1050);
    // simplest_yuv420p_gray("../res/longmao_420p.yuv", "../res/longmao_gray.yuv", 1680, 1050);
    // simplest_yuv420p_half("../res/longmao_420p.yuv", "../res/longmao_half.yuv", 1680, 1050);
    // simplest_yuv420p_add_border("../res/longmao_420p.yuv", "../res/longmao_border.yuv", 1680, 1050, 20);
    // simplest_yuv420p_graybar("../res/graybar.yuv", 1680, 1050, 0, 255, 10);
    // simplest_yuv420p_psnr("../res/longmao_420p.yuv", "../res/longmao_blur_420p.yuv", 1680, 1050); // PSNR: 26.921398
    // simplest_rgb24_split("../res/longmao_rgb24.rgb", "../res/longmao_rgb24_r.rgb", "../res/longmao_rgb24_g.rgb", "../res/longmao_rgb24_b.rgb", 1680, 1050);
    simplest_rgb24_colorbar("../res/colorbar.rgb", 1680, 1050);
    return 0;
}