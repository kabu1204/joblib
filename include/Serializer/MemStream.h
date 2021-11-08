#pragma once
#include "Stream.h"
#include <queue>

class MemStream : public std::queue<char>, public Stream {
public:
    void write(char const* psrc, size_t nbyte)override {
        while (nbyte--) {
            push(*psrc);
            ++psrc;
        }
    }

    size_t read(char* pdst, size_t nbyte)override {
        while (nbyte--) {
            *pdst = front();
            pop();
            ++pdst;
        }
        return nbyte;
    }

    void clear()override {
        while (!empty())pop();
    };
};