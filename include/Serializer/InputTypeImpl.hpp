#pragma once
// 此处是循环include，如果编译过不了把下面这个include注释就好
// #include "Serializer.hpp"

#include <vector>
template <typename Ty, typename Alloc>
void input_type_impl(Serializer& buf, std::vector<Ty, Alloc> const& con) {
    buf << con.size();
    for (auto const& elem : con) {
        buf << elem;
    }
}

template < typename Ty, typename Alloc>
void outnput_type_impl(Serializer& buf, std::vector<Ty, Alloc>& con) {
    size_t n;
    buf >> n;
    con.reserve(n);
    while (n--) {
        typename std::vector<Ty, Alloc>::value_type T;
        buf >> T;
        con.push_back(T);
    }
}

#include <list>
template <typename Ty, typename Alloc>
void input_type_impl(Serializer& buf, std::list<Ty, Alloc>const& con) {
    buf << con.size();
    for (auto const& elem : con) {
        buf << elem;
    }
}

template < typename Ty, typename Alloc>
void output_type_impl(Serializer& buf, std::list<Ty, Alloc>& con) {
    size_t n;
    buf >> n;
    while (n--) {
        typename std::list<Ty, Alloc>::value_type T;
        buf >> T;
        con.push_back(T);
    }
}

#include <string>
template <typename Ty, typename Traits, typename Alloc>
void input_type_impl(Serializer& buf, std::basic_string<Ty, Traits, Alloc>const& con) {
    buf << con.size();
    if (con.size() == 0)return;
    buf.write(con.c_str(), con.size() + 1);
}

template <typename Ty, typename Traits, typename Alloc>
void output_type_impl(Serializer& buf, std::basic_string<Ty, Traits, Alloc>& con) {
    size_t n;
    buf >> n;
    if (n == 0)return;
    char* p = new char[n + 1];
    buf.read(p, n + 1);
    con = std::string(p);
    delete[] p;
}
//以下STL的序列化还没有实现，需要的话可以参照上面自行扩展
//非POD的自定义类型(例如包含指针)的序列化需要自行重载<<和>>
#include <queue>

#include <stack>

#include <map>

#include <set>

#include <unordered_map>

#include <unordered_set>

