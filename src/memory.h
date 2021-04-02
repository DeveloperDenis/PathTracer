#ifndef MEMORY_H
#define MEMORY_H

static inline void* allocMemory(u32 numBytes)
{
    return VirtualAlloc(0, numBytes, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
}
static inline void freeMemory(void* data)
{
    VirtualFree(data, 0, MEM_RELEASE);
}


#endif //MEMORY_H
