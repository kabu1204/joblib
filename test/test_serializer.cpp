#include <Serializer/Serializer.hpp>
#include <Serializer/FileStream.h>
#include <Serializer/MemStream.h>

#define TEST_ASSERT(expr, msg) \
if(!expr){ std::cerr<<#expr<<" fail. msg="<<msg<<" file="<<__FILE__<<" line="<<__LINE__<<"\n"; std::exit(0);}
#define TEST_Serializer(S, a, eqFunc, msg) { \
    S.clear();      \
    S << a;         \
    decltype(a) b;  \
    S >> b;         \
    TEST_ASSERT(eqFunc(a, b), msg);   \
    std::clog<<"pass\n";              \
}

struct Person {
    std::list<int> m_data;
    std::string m_name;
    bool operator==(Person const& other) {
        return m_data == other.m_data && m_name == other.m_name;
    }
    friend Serializer& operator<<(Serializer& in, Person const& d) {
        in << d.m_name << d.m_data; return in;
    }
    friend Serializer& operator>>(Serializer& out, Person& d) {
        out >> d.m_name >> d.m_data; return out;
    }
};

class TEST_Inf {
   
};
int main() {
    MemStream streambuf;
    Serializer S(&streambuf);

    Person person;
    person.m_name = "dyldw";
    person.m_data.push_back(1);
    person.m_data.push_back(2);
    auto person_equal = [](auto a, auto b) { return a == b; };
    TEST_Serializer(S, person, person_equal, "Person");

    {
        std::vector<double> vec{ 3.14,1.59,2.65,3.58 };
        auto equal = [](auto a, auto b) { return a == b; };
        TEST_Serializer(S, vec, equal, "std::vector");
    }

    {
        std::stack<int> stack;
        stack.push(1); stack.push(2); stack.push(3);
        auto equal = [](auto a, auto b) {
            if (a.size() != b.size())return false;
            while (!a.empty()) {
                if (a.top() != b.top())return false;
                a.pop(); b.pop();
            }
            return true;
        };
        TEST_Serializer(S, stack, equal, "std::stack");
    }

    {
        std::queue<int> que;
        que.push(1); que.push(2); que.push(3);
        auto equal = [](auto a, auto b) {
            if (a.size() != b.size())return false;
            while (!a.empty()) {
                if (a.front() != b.front())return false;
                a.pop(); b.pop();
            }
            return true;
        };
        TEST_Serializer(S, que, equal, "std::queue");
    }

    {
        std::set<int> set;
        set.insert(1); set.insert(2); set.insert(3);
        auto equal = [](auto a, auto b) { return a == b; };
        TEST_Serializer(S, set, equal, "std::set");
    }

    {
        std::unordered_set<int> set;
        set.insert(1); set.insert(2); set.insert(3);
        auto equal = [](auto a, auto b) { return a == b; };
        TEST_Serializer(S, set, equal, "std::unordered_set");
    }

    {
        std::map<int, int> map;
        map[1] = 1; map[2] = 2; map[3] = 3;
        auto equal = [](auto a, auto b) { return a == b; };
        TEST_Serializer(S, map, equal, "std::map");
    }

    {
        std::unordered_map<int, int> map;
        map[1] = 1; map[2] = 2; map[3] = 3;
        auto equal = [](auto a, auto b) { return a == b; };
        TEST_Serializer(S, map, equal, "std::unordered_map");
    }

    {
        std::tuple<int, double> tuple{ 1,3.14 };
        auto equal = [](auto a, auto b) { return a == b; };
        TEST_Serializer(S, tuple, equal, "std::tuple");
    }
}