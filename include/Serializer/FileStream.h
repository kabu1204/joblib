#pragma once
#include <fstream>
#include "Stream.h"

class FileStream :public std::fstream, public Stream {
public:
    FileStream(std::string const& filename, std::ios_base::openmode mode) 
        :std::fstream(filename, mode) {
        seekp(0, std::ios::end);
        seekg(0, std::ios::beg);
    };
    void write(char const* psrc, size_t nbyte)override {
        this->std::fstream::write(psrc, nbyte);
    }

    size_t read(char* pdst, size_t nbyte)override {
        this->std::fstream::read(pdst, nbyte);
        return nbyte;
    }

    void clear()override {
        throw "还没有实现\n";
    };

    bool open(std::string const& filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) {
        this->std::fstream::open(filename, mode);
        seekp(0, std::ios::end);
        seekg(0, std::ios::beg);
        return this->std::fstream::is_open();
    }
};