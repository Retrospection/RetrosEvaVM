//
// Created by Retros on 2022/11/5.
//

#ifndef RETROSEVAVM_LOGGER_H
#define RETROSEVAVM_LOGGER_H

#include <sstream>

class ErrorLogMessage : public std::basic_ostringstream<char> {
public:
    ~ErrorLogMessage() {
        std::cerr << "Fatal error: " << str() << std::endl;
        exit(EXIT_FAILURE);
    }
};

#define DIE ErrorLogMessage()

#define log(value) std::cout << #value << " = " << (value) << "\n";

#endif //RETROSEVAVM_LOGGER_H
