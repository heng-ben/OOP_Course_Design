#include "person.h"
#include <iostream>

int main() {
    person stu("张三", 20);
    stu.show_info();
    std::cout << "编译运行成功！" << std::endl;
    system("pause");
    return 0;
}
