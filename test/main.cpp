//
// Created by yuchengye on 2021/11/5.
// change by dongyilong on 2021/11/8.
//
#include "FileStream.h"
#include "MemStream.h"
#include "Serializer.hpp"
#include <list>
#include <iostream>
class Person {
    std::list<int> m_data;
    std::string m_name;
public:
    void set_name(std::string name) {
        m_name = name;
    }
    void add_num(int i) {
        m_data.push_back(i);
    }
    void show() {
        std::cout << "name = " << m_name << "\n";
        for (auto num : m_data) {
            std::cout << num << " ";
            std::cout << "\n";
        }
    }
    friend Serializer& operator<<(Serializer& in, Person const& d) {
        in << d.m_name;
        in << d.m_data;
        return in;
    }
    friend Serializer& operator>>(Serializer& out, Person& d) {
        out >> d.m_name;
        out >> d.m_data;
        return out;
    }
};

int main() {
    std::cout << "序列化测试\n";
    Person person;
    person.set_name("dyldw");
    person.add_num(1);
    person.add_num(2);
    person.add_num(3);
    person.add_num(4);
    person.show();

    std::cout << " Memory Stream\n";
    MemStream streambuf;
    Serializer serializer(&streambuf);

    serializer << person;

    Person memperson;

    serializer >> memperson;

    person.show();
    memperson.show();

    Person fileperson;

    std::cout << " File Stream\n";
    FileStream filestream("test.dat", std::ios::binary|std::ios::out|std::ios::in);
    if (!filestream.is_open()) {
        std::cout << "打开文件失败\n";
        exit(0);
    }
    serializer.stream(&filestream);

    serializer << person;
    serializer >> fileperson;
    filestream.close();
    person.show();
    fileperson.show();
    std::cout << "File Stream还有bug\n";
}