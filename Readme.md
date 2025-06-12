# Cross-Platform Memory Pool Implementation

A high-performance, thread-safe memory pool implementation in C that works on both Windows and Linux platforms. Features virtual address mapping, on-demand allocation, and automatic reference counting.

## Features

- **Virtual Address Mapping**: Presents a contiguous virtual address space while using multiple memory blocks internally
- **On-Demand Allocation**: Memory blocks are only allocated when actually written to
- **Reference Counting**: Automatic memory management with reference counting
- **Thread Safety**: Uses platform-specific synchronization primitives (CRITICAL_SECTION on Windows, pthread_mutex on Linux)
- **Cross-Platform**: Works on both Windows and Linux systems
- **Offset-based Access**: Simple API using offsets rather than direct pointers

## API Documentation

### `MemoryPool* pool_create(size_t total_size, size_t max_block_size)`
Creates a new memory pool with specified total size and maximum block size.

### `void pool_destroy(MemoryPool* pool)`
Destroys the memory pool and frees all allocated resources.

### `size_t pool_write(MemoryPool* pool, size_t offset, const void* src, size_t size)`
Writes data to the memory pool at specified offset.

### `size_t pool_read(MemoryPool* pool, void* dest, size_t offset, size_t size)`
Reads data from the memory pool at specified offset.

## Usage Example

```c
#include "memory_pool.h"
#include <stdio.h>
#include <string.h>

int main() {
    // Create pool with 4KB total size and 1KB max block size
    MemoryPool* pool = pool_create(4096, 1024);
    
    // Write data
    const char* msg = "Test message";
    size_t written = pool_write(pool, 0, msg, strlen(msg)+1);
    
    // Read data
    char buffer[256];
    size_t read = pool_read(pool, buffer, 0, strlen(msg)+1);
    printf("Read: %s\n", buffer);
    
    // Clean up
    pool_destroy(pool);
    return 0;
}
```

## Building

### Linux
```bash
gcc -o memory_pool memory_pool.c -lpthread
```

### Windows
```bash
cl memory_pool.c /D_WIN32
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.