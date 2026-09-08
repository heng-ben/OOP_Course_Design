#ifndef PERSON_H
#define PERSON_H
#include <string>

class person {
private:
    std::string name;
    int age;

public:
    person(std::string name, int age);
    void show_info();
};

#endif
