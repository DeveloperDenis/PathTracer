#ifndef FILE_IO_H
#define FILE_IO_H

static void write_image_to_bmp(char* fileName, Image* image)
{
    u32 imageSizeBytes = image->width*image->height*sizeof(u32);
    u32 fileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + imageSizeBytes;
    
    void* fileData = VirtualAlloc(0, fileSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    assert(fileData);
    
    BITMAPFILEHEADER* bmpHeader = (BITMAPFILEHEADER*)fileData;
    bmpHeader->bfType = 'MB';
    bmpHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
    bmpHeader->bfSize = bmpHeader->bfOffBits + sizeof(u32)*image->width*image->height;
    
    BITMAPINFO* bmpInfo = (BITMAPINFO*)(bmpHeader + 1);
    bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo->bmiHeader.biWidth = image->width;
    bmpInfo->bmiHeader.biHeight = -(s32)image->height;
    bmpInfo->bmiHeader.biPlanes = 1;
    bmpInfo->bmiHeader.biBitCount = 32;
    bmpInfo->bmiHeader.biCompression = BI_RGB;
    
    HANDLE fileHandle = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
    u32* imagePixels = (u32*)(bmpInfo + 1);
    
    for (u32 i = 0; i < image->width*image->height; ++i)
    {
        v4f colour = image->pixels[i];
        
        // NOTE: gamma correction for gamma = 2.0
        colour.r = (f32)sqrt(colour.r);
        colour.g = (f32)sqrt(colour.g);
        colour.b = (f32)sqrt(colour.b);
        colour.a = (f32)sqrt(colour.a);
        
        u8 red = (u8)(255.0f*colour.r);
        u8 green = (u8)(255.0f*colour.g);
        u8 blue = (u8)(255.0f*colour.b);
        u8 alpha = (u8)(255.0f*colour.a);
        
        imagePixels[i] = (u32)((alpha << 24) | (red << 16) | (green << 8) | blue);
    }
    
    DWORD bytesWritten;
    WriteFile(fileHandle, fileData, fileSize, &bytesWritten, 0);
    assert(bytesWritten == fileSize);
    
    VirtualFree(imagePixels, 0, MEM_RELEASE);
}

#endif //FILE_IO_H
