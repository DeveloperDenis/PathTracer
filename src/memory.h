#ifndef MEMORY_H
#define MEMORY_H

static inline void* memory_alloc(u32 numBytes)
{
    return VirtualAlloc(0, numBytes, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
}
static inline void memory_free(void* data)
{
    VirtualFree(data, 0, MEM_RELEASE);
}


#endif //MEMORY_H
