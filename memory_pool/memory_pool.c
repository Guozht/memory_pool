#include "memory_pool.h"
#include <stdlib.h>
#include <string.h>

#define ALIGN_SIZE 8
#define ALIGN(x) (((x) + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))
#define VIRTUAL_ADDR_START 0x10000000

MemoryPool* pool_create(size_t total_size, size_t max_block_size) {
    MemoryPool* pool = malloc(sizeof(MemoryPool));
    pool->total_size = ALIGN(total_size);
    pool->max_block_size = ALIGN(max_block_size);
    pool->used_size = 0;
    pool->next_virtual_addr = (void*)VIRTUAL_ADDR_START;
    pool->block_list = NULL;
    
#ifdef _WIN32
    InitializeCriticalSection(&pool->pool_lock);
#else
    pthread_mutex_init(&pool->pool_lock, NULL);
#endif

    size_t remaining = pool->total_size;
    while (remaining > 0) {
        size_t block_size = (remaining > pool->max_block_size) ? 
                          pool->max_block_size : remaining;
        
        MemoryBlock* block = malloc(sizeof(MemoryBlock));
        block->real_addr = NULL;
        block->virtual_addr = pool->next_virtual_addr;
        block->size = block_size;
        block->ref_count = 0;
        block->is_allocated = 0;
        block->next = pool->block_list;
        
#ifdef _WIN32
        InitializeCriticalSection(&block->lock);
#else
        pthread_mutex_init(&block->lock, NULL);
#endif
        
        pool->block_list = block;
        pool->next_virtual_addr = (char*)pool->next_virtual_addr + block_size;
        remaining -= block_size;
    }
    
    return pool;
}

static MemoryBlock* find_block(MemoryPool* pool, size_t offset) {
    void* target_addr = (char*)VIRTUAL_ADDR_START + offset;
    MemoryBlock* block = pool->block_list;
    while (block) {
        if (target_addr >= block->virtual_addr && 
            target_addr < (char*)block->virtual_addr + block->size) {
            return block;
        }
        block = block->next;
    }
    return NULL;
}

size_t pool_write(MemoryPool* pool, size_t offset, const void* src, size_t size) {
    if (!src) return 0;
    
    size_t bytes_written = 0;
    size_t current_offset = offset;
    
    while (bytes_written < size) {
        MemoryBlock* block = find_block(pool, current_offset);
        if (!block) break;
        
#ifdef _WIN32
        EnterCriticalSection(&block->lock);
#else
        pthread_mutex_lock(&block->lock);
#endif
        
        size_t block_offset = (char*)VIRTUAL_ADDR_START + current_offset - (char*)block->virtual_addr;
        size_t copy_size = block->size - block_offset;
        if (copy_size > size - bytes_written) copy_size = size - bytes_written;
        
        if (!block->is_allocated) {
            block->real_addr = malloc(block->size);
            block->is_allocated = 1;
            block->ref_count = 1;
            pool->used_size += block->size;
        }/* else {
            block->ref_count++;
        }*/
        
        memcpy((char*)block->real_addr + block_offset, 
              (const char*)src + bytes_written, 
              copy_size);
        
        bytes_written += copy_size;
        current_offset += copy_size;
        
#ifdef _WIN32
        LeaveCriticalSection(&block->lock);
#else
        pthread_mutex_unlock(&block->lock);
#endif
    }
    
    return bytes_written;
}

size_t pool_read(MemoryPool* pool, void* dest, size_t offset, size_t size) {
    if (!dest) return 0;
    
    size_t bytes_read = 0;
    size_t current_offset = offset;
    
#ifdef _WIN32
    EnterCriticalSection(&pool->pool_lock);
#else
    pthread_mutex_lock(&pool->pool_lock);
#endif
    
    MemoryBlock* block = pool->block_list;
    
    while (block && bytes_read < size) {
        void* target_addr = (char*)VIRTUAL_ADDR_START + current_offset;
        if (target_addr >= block->virtual_addr && 
            target_addr < (char*)block->virtual_addr + block->size) {
            
#ifdef _WIN32
            EnterCriticalSection(&block->lock);
#else
            pthread_mutex_lock(&block->lock);
#endif
            
            if (!block->is_allocated) {
#ifdef _WIN32
                LeaveCriticalSection(&block->lock);
#else
                pthread_mutex_unlock(&block->lock);
#endif
                break;
            }
            
            size_t block_offset = (char*)target_addr - (char*)block->virtual_addr;
            size_t copy_size = block->size - block_offset;
            if (copy_size > size - bytes_read) copy_size = size - bytes_read;
            
            memcpy((char*)dest + bytes_read, 
                  (char*)block->real_addr + block_offset, 
                  copy_size);
            
            bytes_read += copy_size;
            current_offset += copy_size;
            block->ref_count--;
            
            if (block->ref_count == 0 && block->is_allocated) {
                free(block->real_addr);
                block->real_addr = NULL;
                block->is_allocated = 0;
                pool->used_size -= block->size;
            }
            
#ifdef _WIN32
            LeaveCriticalSection(&block->lock);
#else
            pthread_mutex_unlock(&block->lock);
#endif
            
            block = pool->block_list;
        } else {
            block = block->next;
        }
    }
    
#ifdef _WIN32
    LeaveCriticalSection(&pool->pool_lock);
#else
    pthread_mutex_unlock(&pool->pool_lock);
#endif
    
    return bytes_read;
}

void pool_destroy(MemoryPool* pool) {
#ifdef _WIN32
    EnterCriticalSection(&pool->pool_lock);
#else
    pthread_mutex_lock(&pool->pool_lock);
#endif
    
    MemoryBlock* curr = pool->block_list;
    
    while (curr) {
        MemoryBlock* next = curr->next;
        if (curr->is_allocated) {
            free(curr->real_addr);
        }
#ifdef _WIN32
        DeleteCriticalSection(&curr->lock);
#else
        pthread_mutex_destroy(&curr->lock);
#endif
        free(curr);
        curr = next;
    }
    
#ifdef _WIN32
    LeaveCriticalSection(&pool->pool_lock);
    DeleteCriticalSection(&pool->pool_lock);
#else
    pthread_mutex_unlock(&pool->pool_lock);
    pthread_mutex_destroy(&pool->pool_lock);
#endif
    
    free(pool);
}
