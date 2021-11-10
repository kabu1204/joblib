#pragma once
/**
 * @file Serializer.hpp
 * @author dyldw
 * @version 1.0
 * @date 2021-11-10 19:31
 * @brief 序列化器 Serializer类, 依赖Stream接口,Stream.h
 * @details
 * 文件中实现了Serializer类,支持POD类型和STL常见容器的序列化，反序列化。
 * 对于自定义的类型具有良好的扩展性，用户只需要在自定义类中增加两个友元函数
 * 此文件中include的STLTypeImpl文件实现了常见STL类型的序列化和反序列化,STLTypeImpl文件还有待改进，不稳定。
 * `friend Serializer& operator>>(Serializer& out, Type& t)`和
 * `friend Serializer& operator<<(Serializer& out, Type const& t)`即可完成序列化支持。
 */
#include "debug.h"
#include "Stream.h"
class Serializer{
public:
    Serializer() :m_iobuf(nullptr) {};
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

#include "STLTypeImpl.hpp"

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