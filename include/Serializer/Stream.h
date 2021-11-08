#pragma once
#include <cstddef>
// stream interface
class Stream {
public:
    Stream() = default;
    virtual ~Stream() {};
    virtual void write(char const* psrc, size_t nbyte) = 0;
    virtual size_t read(char* pdst, size_t nbyte) = 0;
    virtual void clear() = 0;
};


