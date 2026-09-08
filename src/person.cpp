#include "Person.h"
#include <iostream>

person::person(std::string name, int age) {
    this->name = name;
    this->age = age;
}

void person::show_info() {
    std::cout << "姓名：" << name << "，年龄：" << age << std::endl;
}
