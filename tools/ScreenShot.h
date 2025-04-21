#pragma once
#pragma pack(2)//Ӱ���ˡ����롱

#include "../core.h"


namespace GLframework
{

    typedef unsigned char  BYTE;
    typedef unsigned short WORD;
    typedef unsigned long  DWORD;
    typedef long    LONG;

    //BMP�ļ�ͷ��
    struct BITMAPFILEHEADER
    {
        WORD  bfType;		//�ļ����ͱ�ʶ������ΪASCII�롰BM��
        DWORD bfSize;		//�ļ��ĳߴ磬��byteΪ��λ
        WORD  bfReserved1;	//�����֣�����Ϊ0
        WORD  bfReserved2;	//�����֣�����Ϊ0
        DWORD bfOffBits;	//һ����byteΪ��λ��ƫ�ƣ���BITMAPFILEHEADER�ṹ�忪ʼ��λͼ����
    };

    //BMP��Ϣͷ��
    struct BITMAPINFOHEADER
    {
        DWORD biSize;			//����ṹ��ĳߴ�
        LONG  biWidth;			//λͼ�Ŀ��
        LONG  biHeight;			//λͼ�ĳ���
        WORD  biPlanes;			//The number of planes for the target device. This value must be set to 1.
        WORD  biBitCount;		//һ�������м�λ
        DWORD biCompression;    //0����ѹ����1��RLE8��2��RLE4
        DWORD biSizeImage;      //4�ֽڶ����ͼ�����ݴ�С
        LONG  biXPelsPerMeter;  //������/�ױ�ʾ��ˮƽ�ֱ���
        LONG  biYPelsPerMeter;  //������/�ױ�ʾ�Ĵ�ֱ�ֱ���
        DWORD biClrUsed;        //ʵ��ʹ�õĵ�ɫ����������0��ʹ�����еĵ�ɫ������
        DWORD biClrImportant;	//��Ҫ�ĵ�ɫ����������0�����еĵ�ɫ����������Ҫ
    };

    //һ�����ص���ɫ��Ϣ
    struct RGBColor
    {
        char B;		//��
        char G;		//��
        char R;		//��
    };

    //����ɫ����д��һ��BMP�ļ���
    //FileName:�ļ���
    //ColorBuffer:��ɫ����
    //ImageWidth:ͼ����
    //ImageHeight:ͼ�񳤶�
    inline void WriteBMP(const char* FileName, unsigned char* GrayscaleBuffer, int ImageWidth, int ImageHeight)
    {
        const int BufferSize = ImageHeight * ImageWidth;

        BITMAPFILEHEADER fileHeader;
        fileHeader.bfType = 0x4D42;
        fileHeader.bfReserved1 = 0;
        fileHeader.bfReserved2 = 0;
        fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + BufferSize + 1024; // ���ϵ�ɫ���С
        fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 1024; // ƫ����������ɫ��

        BITMAPINFOHEADER bitmapHeader = { 0 };
        bitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapHeader.biHeight = ImageHeight;
        bitmapHeader.biWidth = ImageWidth;
        bitmapHeader.biPlanes = 1;
        bitmapHeader.biBitCount = 8;  // 8λ�Ҷ�
        bitmapHeader.biSizeImage = BufferSize;
        bitmapHeader.biCompression = 0;

        FILE* fp;
        fopen_s(&fp, FileName, "wb");

        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
        fwrite(&bitmapHeader, sizeof(BITMAPINFOHEADER), 1, fp);

        // д��Ҷȵ�ɫ��
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

        // �����ֵת��Ϊ�Ҷ�
        for (int i = 0; i < WindowSizeX * WindowSizeY; ++i) {
            // �������ֵ�Ѿ���һ����0��1֮��
            GrayscaleBuffer[i] = static_cast<unsigned char>(depthValues[i] * 255.0f);
        }

        // ���Ҷ�����д��BMP�ļ�
        WriteBMP("depth_output5.bmp", GrayscaleBuffer, WindowSizeX, WindowSizeY);

        delete[] GrayscaleBuffer;
    }
}
