#pragma once
/**                                   
 * @file STLTypeImpl.hpp                    
 * @author dyldw
 * @version 1.0
 * @date 2021-11-10 19:31
 * @brief STL的序列化和反序列化方法
 * @details 
 * 实现了简单的vector, list, string, stack, queue, map, set, 
 * unordered_map, unordered_set, tuple的序列化和反序列化。
 * 如果想要更改或者拓展其他STL类型的序列化，只需要在这个文件增加
 * `void input_type_impl(Serializer& buf, Type const& t)`和
 * `void output_type_impl(Serializer& buf, Type& t)`即可。
 * @note 此文件在Serializer中include
 */
#include "Serializer.hpp"

#include <vector>
template <typename Ty, typename Alloc>
void input_type_impl(Serializer& buf, std::vector<Ty, Alloc> const& con) {
    buf << con.size();
    for (auto const& elem : con) {
        buf << elem;
    }
}

template < typename Ty, typename Alloc>
void output_type_impl(Serializer& buf, std::vector<Ty, Alloc>& con) {
    size_t n;
    buf >> n;
    con.clear();
    con.reserve(n);
    while (n--) {
        Ty tmp;
        buf >> tmp;
        con.push_back(tmp);
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
    con.clear();
    while (n--) {
        Ty tmp;
        buf >> tmp;
        con.push_back(tmp);
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
    con.clear();
    size_t n;
    buf >> n;
    if (n == 0)return;
    char* p = new char[n + 1];
    buf.read(p, n + 1);
    con = std::string(p);
    delete[] p;
}

#include <queue>
template <class Ty, class Container>
void input_type_impl(Serializer& buf, std::queue<Ty, Container> const& que) {
    std::queue<Ty, Container> tmp = que;
    size_t n = que.size();
    buf << n;
    while (!tmp.empty()) {
        buf << tmp.front();
        tmp.pop();
    }
}
template <class Ty, class Container>
void output_type_impl(Serializer& buf, std::queue<Ty, Container>& que) {
    while (que.size())que.pop();
    size_t n;
    buf >> n;
    while (n--) {
        Ty tmp;
        buf >> tmp;
        que.push(tmp);
    }
}

#include <stack>
#include <algorithm>
template <class Ty, class Container>
void input_type_impl(Serializer& buf, std::stack<Ty, Container> const& stack) {
    std::stack<Ty, Container> tmp = stack;
    size_t n = stack.size();
    buf << n;
    std::vector<Ty> vec;
    while (!tmp.empty()) {
        vec.push_back(tmp.top());
        tmp.pop();
    }
    std::reverse(vec.begin(), vec.end());
    
    for (auto const& elem : vec)
    {
        buf << elem;
    }
}

template <class Ty, class Container>
void output_type_impl(Serializer& buf, std::stack<Ty, Container>& stack) {
    while (stack.size())stack.pop();
    size_t n;
    buf >> n;
    while (n--) {
        Ty tmp;
        buf >> tmp;
        stack.push(tmp);
    }
}
//std::pair
#include <map>
template <typename T1, typename T2>
void input_type_impl(Serializer& buf, std::pair<T1,T2> const& con) {
    buf << con.first << con.second;
}
template <typename T1, typename T2>
void output_type_impl(Serializer& buf, std::pair<T1, T2>& con) {
    buf >> con.first >> con.second;
}
//map
template <class Kty, class Ty, class Pr , class Alloc>
void input_type_impl(Serializer& buf, std::map<Kty,Ty,Pr,Alloc> const& con) {
    buf << con.size();
    for (auto const& elem : con){
        buf << elem;
    }
}
template <class Kty, class Ty, class Pr, class Alloc>
void output_type_impl(Serializer& buf, std::map<Kty, Ty, Pr, Alloc>& con) {
    size_t n;
    buf >> n;
    con.clear();
    while (n--) {
        std::pair<Kty, Ty> tmp;
        buf >> tmp;
        con[tmp.first] = tmp.second;
    }
}

#include <set>
template <class Kty, class Pr , class Alloc>
void input_type_impl(Serializer& buf, std::set<Kty, Pr, Alloc> const& set) {
    buf << set.size();
    for (auto const& elem : set){
        buf << elem;
    }
}

template <class Kty, class Pr, class Alloc>
void output_type_impl(Serializer& buf, std::set<Kty, Pr, Alloc>& set) {
    size_t n;
    buf >> n;
    while (n--) {
        Kty tmp;
        buf >> tmp;
        set.insert(tmp);
    }
}

#include <unordered_set>
template <class Kty, class Hasher , class Keyeq, class Alloc>
void input_type_impl(Serializer& buf, std::unordered_set<Kty,Hasher,Keyeq, Alloc> const& set) {
    buf << set.size();
    for (auto const& elem : set) {
        buf << elem;
    }
}
template <class Kty, class Hasher, class Keyeq, class Alloc>
void output_type_impl(Serializer& buf, std::unordered_set<Kty, Hasher, Keyeq, Alloc>& set) {
    size_t n;
    buf >> n;
    while (n--) {
        Kty tmp;
        buf >> tmp;
        set.insert(tmp);
    }
}
#include <unordered_map>
template <class Kty, class Ty, class Hasher, class Keyeq, class Alloc>
void input_type_impl(Serializer& buf, std::unordered_map<Kty, Ty, Hasher, Keyeq, Alloc> const& map) {
    buf << map.size();
    for (auto const& elem : map){
        buf << elem;
    }
}

template <class Kty, class Ty, class Hasher, class Keyeq, class Alloc>
void output_type_impl(Serializer& buf, std::unordered_map<Kty, Ty, Hasher, Keyeq, Alloc>& map) {
    size_t n;
    buf >> n;
    map.clear();
    map.reserve(n);
    while (n--) {
        typename Alloc::value_type tmp;
        buf >> tmp;
        map[tmp.first] = tmp.second;
    }
}

#include <tuple>
template<typename Tuple, std::size_t... Is>
void input_tuple_impl(Serializer& buf, Tuple const& tuple, std::index_sequence<Is...>) {
    std::initializer_list<int>{((buf << std::get<Is>(tuple)), 0)...};
}
template <typename... Args>
void input_type_impl(Serializer& buf, std::tuple<Args...> const& tuple) {
    input_tuple_impl(buf, tuple, std::index_sequence_for<Args...>{});
}
template<typename Tuple, std::size_t... Is>
void output_tuple_impl(Serializer& buf, Tuple& tuple, std::index_sequence<Is...>) {
    std::initializer_list<int>{((buf >> std::get<Is>(tuple)), 0)...};
}
template <typename... Args>
void output_type_impl(Serializer& buf, std::tuple<Args...>& tuple) {
    output_tuple_impl(buf, tuple, std::index_sequence_for<Args...>{});
}

// 还有其他STL的序列化还没有实现，需要的话可以参照上面自行扩展
// 非POD的自定义类型(例如包含指针)的序列化需要自行重载<<和>>