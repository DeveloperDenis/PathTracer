#ifndef FILE_IO_H
#define FILE_IO_H

static void write_image_to_bmp(char* fileName, Image* image)
{
    BITMAPFILEHEADER bmpHeader = {};
    bmpHeader.bfType = 'MB';
    bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
    bmpHeader.bfSize = bmpHeader.bfOffBits + sizeof(u32)*image->width*image->height;
    
    BITMAPINFOHEADER bmpInfoHeader = {};
    bmpInfoHeader.biSize = sizeof(bmpInfoHeader);
    bmpInfoHeader.biWidth = image->width;
    bmpInfoHeader.biHeight = -(s32)image->height;
    bmpInfoHeader.biPlanes = 1;
    bmpInfoHeader.biBitCount = 32;
    bmpInfoHeader.biCompression = BI_RGB;
    
    BITMAPINFO bmpInfo = {};
    bmpInfo.bmiHeader = bmpInfoHeader;
    
    HANDLE fileHandle = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
    DWORD bytesWritten;
    WriteFile(fileHandle, &bmpHeader, sizeof(bmpHeader), &bytesWritten, 0);
    WriteFile(fileHandle, &bmpInfo, sizeof(bmpInfo), &bytesWritten, 0);
    
    // TODO(denis): probably very slow, should probably precalculate all the u32 values
    // and then write them
    for (u32 i = 0; i < image->width*image->height; ++i)
    {
        v4f colour = image->pixels[i];
        
        u8 red = (u8)(255.0f*colour.r);
        u8 green = (u8)(255.0f*colour.g);
        u8 blue = (u8)(255.0f*colour.b);
        u8 alpha = (u8)(255.0f*colour.a);
        
        u32 intColour = (alpha << 24) | (red << 16) | (green << 8) | blue;
        WriteFile(fileHandle, &intColour, sizeof(u32), &bytesWritten, 0);
    }
}

#endif //FILE_IO_H
