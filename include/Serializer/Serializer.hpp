#pragma once
#include "Stream.h"
class Serializer{
public:
    Serializer(Stream* const pstm) :m_iobuf(pstm) {};

    Stream* stream() { return m_iobuf; }
    void stream(Stream* const pstm) { m_iobuf = pstm; }

    template <typename T> Serializer& operator<<(T const& value);
    template <typename T> Serializer& operator>>(T& value);
    void write(char const* psrc, size_t nbyte) { m_iobuf->write(psrc, nbyte); };
    size_t read(char* pdst, size_t nbyte) { return m_iobuf->read(pdst, nbyte); };

    void clear() {
        m_iobuf->clear();
    }
private:
    Stream* m_iobuf;
};

// is POD type
template<typename T> 
typename std::enable_if<std::is_trivially_copyable<T>::value, void>::type
input_type(Serializer& buf, T const& value) {
    char const* psrc = reinterpret_cast<char const*>(&value);
    buf.write(psrc, sizeof(T));
}

template<typename T> 
typename std::enable_if<std::is_trivially_copyable<T>::value, void>::type
output_type(Serializer& buf, T& value) {
    char* pdst = reinterpret_cast<char*>(&value);
    buf.read(pdst, sizeof(T));
}

template<typename T>
typename std::enable_if<!std::is_trivially_copyable<T>::value, void>::type
input_type(Serializer& buf, T const& value) {
    input_type_impl(buf, value);
}

template<typename T>
typename std::enable_if<!std::is_trivially_copyable<T>::value, void>::type
output_type(Serializer& buf, T& value) {
    output_type_impl(buf, value);
}

#include "InputTypeImpl.hpp"

template<typename T>
inline Serializer& Serializer::operator<<(T const& value) {
    input_type<T>(*this, value);
    return *this;
}

template<typename T>
inline Serializer& Serializer::operator>>(T& value) {
    output_type<T>(*this, value);
    return *this;
}