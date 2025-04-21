#pragma once
#pragma pack(2)//影响了“对齐”

#include "../core.h"


namespace GLframework
{

    typedef unsigned char  BYTE;
    typedef unsigned short WORD;
    typedef unsigned long  DWORD;
    typedef long    LONG;

    //BMP文件头：
    struct BITMAPFILEHEADER
    {
        WORD  bfType;		//文件类型标识，必须为ASCII码“BM”
        DWORD bfSize;		//文件的尺寸，以byte为单位
        WORD  bfReserved1;	//保留字，必须为0
        WORD  bfReserved2;	//保留字，必须为0
        DWORD bfOffBits;	//一个以byte为单位的偏移，从BITMAPFILEHEADER结构体开始到位图数据
    };

    //BMP信息头：
    struct BITMAPINFOHEADER
    {
        DWORD biSize;			//这个结构体的尺寸
        LONG  biWidth;			//位图的宽度
        LONG  biHeight;			//位图的长度
        WORD  biPlanes;			//The number of planes for the target device. This value must be set to 1.
        WORD  biBitCount;		//一个像素有几位
        DWORD biCompression;    //0：不压缩，1：RLE8，2：RLE4
        DWORD biSizeImage;      //4字节对齐的图像数据大小
        LONG  biXPelsPerMeter;  //用象素/米表示的水平分辨率
        LONG  biYPelsPerMeter;  //用象素/米表示的垂直分辨率
        DWORD biClrUsed;        //实际使用的调色板索引数，0：使用所有的调色板索引
        DWORD biClrImportant;	//重要的调色板索引数，0：所有的调色板索引都重要
    };

    //一个像素的颜色信息
    struct RGBColor
    {
        char B;		//蓝
        char G;		//绿
        char R;		//红
    };

    //将颜色数据写到一个BMP文件中
    //FileName:文件名
    //ColorBuffer:颜色数据
    //ImageWidth:图像宽度
    //ImageHeight:图像长度
    inline void WriteBMP(const char* FileName, unsigned char* GrayscaleBuffer, int ImageWidth, int ImageHeight)
    {
        const int BufferSize = ImageHeight * ImageWidth;

        BITMAPFILEHEADER fileHeader;
        fileHeader.bfType = 0x4D42;
        fileHeader.bfReserved1 = 0;
        fileHeader.bfReserved2 = 0;
        fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + BufferSize + 1024; // 加上调色板大小
        fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 1024; // 偏移量包括调色板

        BITMAPINFOHEADER bitmapHeader = { 0 };
        bitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapHeader.biHeight = ImageHeight;
        bitmapHeader.biWidth = ImageWidth;
        bitmapHeader.biPlanes = 1;
        bitmapHeader.biBitCount = 8;  // 8位灰度
        bitmapHeader.biSizeImage = BufferSize;
        bitmapHeader.biCompression = 0;

        FILE* fp;
        fopen_s(&fp, FileName, "wb");

        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
        fwrite(&bitmapHeader, sizeof(BITMAPINFOHEADER), 1, fp);

        // 写入灰度调色板
        for (int i = 0; i < 256; i++) {
            unsigned char gray = i;
            unsigned char palette[] = { gray, gray, gray, 0 };
            fwrite(palette, sizeof(palette), 1, fp);
        }

        fwrite(GrayscaleBuffer, BufferSize, 1, fp);

        fclose(fp);
    }

    inline void ScreenShot(int WindowSizeX, int WindowSizeY, const std::vector<float>& depthValues)
    {
        unsigned char* GrayscaleBuffer = new unsigned char[WindowSizeX * WindowSizeY];

        // 将深度值转换为灰度
        for (int i = 0; i < WindowSizeX * WindowSizeY; ++i) {
            // 假设深度值已经归一化到0到1之间
            GrayscaleBuffer[i] = static_cast<unsigned char>(depthValues[i] * 255.0f);
        }

        // 将灰度数据写入BMP文件
        WriteBMP("depth_output5.bmp", GrayscaleBuffer, WindowSizeX, WindowSizeY);

        delete[] GrayscaleBuffer;
    }
}
