
# 跨平台内存池实现

一个高性能、线程安全的C语言内存池实现，支持Windows和Linux平台。提供虚拟地址映射、按需分配和自动引用计数功能。

## 特性

- **虚拟地址映射**：呈现连续的虚拟地址空间，内部使用多个内存块实现
- **按需分配**：内存块仅在写入时实际分配
- **引用计数**：通过引用计数实现自动内存管理
- **线程安全**：使用平台特定的同步原语（Windows使用CRITICAL_SECTION，Linux使用pthread_mutex）
- **跨平台支持**：兼容Windows和Linux系统
- **偏移量访问**：使用偏移量而非直接指针的简洁API

## API 文档

### `MemoryPool* pool_create(size_t total_size, size_t max_block_size)`
创建具有指定总大小和最大块大小的新内存池。

### `void pool_destroy(MemoryPool* pool)`
销毁内存池并释放所有分配的资源。

### `size_t pool_write(MemoryPool* pool, size_t offset, const void* src, size_t size)`
在指定偏移量处向内存池写入数据。

### `size_t pool_read(MemoryPool* pool, void* dest, size_t offset, size_t size)`
从内存池指定偏移量处读取数据。

## 使用示例

```c
#include "memory_pool.h"
#include <stdio.h>
#include <string.h>

int main() {
    // 创建4KB总大小、1KB最大块大小的内存池
    MemoryPool* pool = pool_create(4096, 1024);
    
    // 写入数据
    const char* msg = "测试消息";
    size_t written = pool_write(pool, 0, msg, strlen(msg)+1);
    
    // 读取数据
    char buffer[256];
    size_t read = pool_read(pool, buffer, 0, strlen(msg)+1);
    printf("读取内容: %s\n", buffer);
    
    // 清理资源
    pool_destroy(pool);
    return 0;
}
```

## 编译说明

### Linux
```bash
gcc -o memory_pool memory_pool.c -lpthread
```

### Windows
```bash
cl memory_pool.c /D_WIN32
```

## 许可证

本项目采用MIT许可证 - 详见[LICENSE](LICENSE)文件。

## 贡献指南

欢迎提交Pull Request。对于重大变更，请先提出问题讨论您希望进行的修改。
