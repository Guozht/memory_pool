
#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct MemoryBlock {
    void* real_addr;
    void* virtual_addr;
    size_t size;
    size_t ref_count;
    int is_allocated;
    struct MemoryBlock* next;
#ifdef _WIN32
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
} MemoryBlock;

typedef struct {
    size_t total_size;
    size_t max_block_size;
    size_t used_size;
    void* next_virtual_addr;
    MemoryBlock* block_list;
#ifdef _WIN32
    CRITICAL_SECTION pool_lock;
#else
    pthread_mutex_t pool_lock;
#endif
} MemoryPool;

MemoryPool* pool_create(size_t total_size, size_t max_block_size);
void pool_destroy(MemoryPool* pool);
size_t pool_write(MemoryPool* pool, size_t offset, const void* src, size_t size);
size_t pool_read(MemoryPool* pool, void* dest, size_t offset, size_t size);

#endif
