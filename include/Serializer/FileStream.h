#pragma once
#include "Stream.h"
#include <fstream>

// 文件流，默认模式会在打开文件时清空文件中原有内容
/*
    FileStream filestream("test.dat");
    serializer.stream(&filestream);
    serializer << person;
    //文件流需要移动读指针
    filestream.seekg(0);
    serializer >> fileperson;
*/
#ifdef _WIN32
#define OPEN_MODE std::ios_base::open_mode
#else 
#define OPEN_MODE std::ios_base::openmode
#endif

class FileStream :public std::fstream, public Stream {
public:
    FileStream(
        std::string const& filename, 
        OPEN_MODE mode = std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc)
        :std::fstream(filename, mode) {};

    void write(char const* psrc, size_t nbyte)override {
        this->std::fstream::write(psrc, nbyte);
    }

    size_t read(char* pdst, size_t nbyte)override {
        this->std::fstream::read(pdst, nbyte);
        return nbyte;
    }

    void clear()override {
        this->std::fstream::clear();
    };
};