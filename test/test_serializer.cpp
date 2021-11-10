#include <Serializer/Serializer.hpp>
#include <Serializer/FileStream.h>
#include <Serializer/MemStream.h>

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

int main(){
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
    // xxxxxx
    Person fileperson;

    std::cout << " File Stream\n";
    FileStream filestream("test.dat");
    if (!filestream.is_open()) {
        std::cout << "打开文件失败\n";
        exit(0);
    }
    //filestream.write("hello world!", sizeof("hello world!") + 1);
    serializer.stream(&filestream);

    serializer << person;
    //文件流需要移动读指针
    filestream.seekg(0, std::ios::beg);
    serializer >> fileperson;
    person.show();
    fileperson.show();
    filestream.close();
}