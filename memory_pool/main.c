#include "memory_pool.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
int main() {

    FILE* fp = fopen("src.zip", "rb");
    assert(fp != NULL);
    FILE * fp2 = fopen("dest.zip", "wb");
    assert(fp2 != NULL);

    fseek(fp, 0, SEEK_END);
    size_t remain_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    size_t total_size = remain_size;
    

    MemoryPool* pool = pool_create(total_size, 524288); // 总大小4KB，最大块1KB
    unsigned char* data = malloc(4096); // 获取内存块首地址
    assert(data!= NULL);
    while (remain_size > 0) {
        size_t block_size = (remain_size > 4096 ? 4096 : remain_size);
        assert(fread(data, 1, block_size, fp) == block_size);
        assert(pool_write(pool, total_size - remain_size, data, block_size) == block_size);
        remain_size -= block_size;
    }
    size_t write_size = 0;
    while (write_size < total_size) {
        size_t block_size = (total_size - write_size) > 4096 ? 4096 : (total_size - write_size);
        assert(pool_read(pool, data, write_size, block_size) == block_size);
        assert(fwrite(data, 1, block_size, fp2) == block_size);
        write_size += block_size;
    }

    
    // 测试跨块写入
    const char* msg = "This is a test message that spans multiple blocks";
    size_t written = pool_write(pool, 0, msg, strlen(msg)+1);
    printf("Written %zu bytes\n", written);
    
    // 测试跨块读取
    char buffer[256];
    assert(pool_read(pool, buffer, 0, strlen(msg)+1) == strlen(msg)+1);
    printf("Read bytes: %s\n", buffer);
    
    // 测试引用计数
    assert(pool_read(pool, buffer, 0, 10) == 0); // 再次读取部分数据
    
    pool_destroy(pool);
    return 0;
}
