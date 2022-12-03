//
// Created by Retros on 2022/11/5.
//

#ifndef RETROSEVAVM_EVAVM_H
#define RETROSEVAVM_EVAVM_H

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "OpCode.h"
#include "Logger.h"
#include "EvaValue.h"
#include "EvaCompiler.h"
#include "parser/EvaParser.h"

using syntax::EvaParser;


/**
 * Reads the current byte in the bytecode
 * and advances the ip pointer.
 */
#define READ_BYTE() *ip++

/**
 * Reads a short word (2 bytes).
 */
#define READ_SHORT() (ip += 2, (uint16_t)((*(ip - 2) << 8) | *(ip - 1)))

/**
 * Converts bytecode index to a pointer.
 */
#define TO_ADDRESS(index) (&co->code[index])

/**
 * Gets a constant from the pool
 */
#define GET_CONST() co->constants[READ_BYTE()]

/**
 * Stack top (stack overflow after exceeding).
 */
const size_t STACK_LIMIT = 512;


#define BINARY_OP(op)               \
do {                                \
    auto op2 = AS_NUMBER(pop());    \
    auto op1 = AS_NUMBER(pop());    \
    push(NUMBER((op1 op op2)));     \
} while (false)

#define COMPARE_VALUES(op, v1, v2)  \
do {                                \
    bool res;                       \
    switch (op) {                   \
    case 0:                         \
        res = v1 < v2;              \
        break;                      \
    case 1:                         \
        res = v1 > v2;              \
        break;                      \
    case 2:                         \
        res = v1 == v2;             \
        break;                      \
    case 3:                         \
        res = v1 >= v2;             \
        break;                      \
    case 4:                         \
        res = v1 <= v2;             \
        break;                      \
    case 5:                         \
        res = v1 != v2;             \
        break;                      \
    }                               \
    push(BOOLEAN(res));             \
} while (false)


/**
 * Eva Virtual Machine
 */
class EvaVM {
public:
    EvaVM() : parser(std::make_unique<EvaParser>()), compiler(std::make_unique<EvaCompiler>()) {}

    /*
     * Executes a program.
     */
    EvaValue exec(const std::string &program) {
        // 1. parse the program
        auto ast = parser->parse(program);

        // 2. Compile program to Eva bytecode
        co = compiler->compile(ast);


        // code = {OP_CONST, 0, OP_CONST, 1, OP_ADD, OP_HALT};

        // 3. set instruction pointer to the beginning:
        ip = &co->code[0];

        sp = &stack[0];

        compiler-> disassembleBytecode();

        return eval();
    }

    /**
     * Main eval loop
     */
    EvaValue eval() {
        for (;;) {
            auto opcode = READ_BYTE();
            switch (opcode) {
                case OP_HALT: {
                    return pop();
                }
                case OP_CONST: {
                    push(GET_CONST());
                    break;
                }
                case OP_ADD: {
                    auto op2 = pop();
                    auto op1 = pop();

                    if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
                        auto v1 = AS_NUMBER(op1);
                        auto v2 = AS_NUMBER(op2);
                        push(NUMBER(v1 + v2));
                    } else if (IS_STRING(op1) && IS_STRING(op2)) {
                        auto v1 = AS_CPPSTRING(op1);
                        auto v2 = AS_CPPSTRING(op2);
                        push(ALLOC_STRING(v1 + v2));
                    }

                    break;
                }
                case OP_SUB: {
                    BINARY_OP(-);
                    break;
                }
                case OP_MUL: {
                    BINARY_OP(*);
                    break;
                }
                case OP_DIV: {
                    BINARY_OP(/);
                    break;
                }

                // ------------------------------------
                // Comparison

                case OP_COMPARE: {
                    auto op = READ_BYTE();

                    auto op2 = pop();
                    auto op1 = pop();

                    if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
                        auto v1 = AS_NUMBER(op1);
                        auto v2 = AS_NUMBER(op2);
                        COMPARE_VALUES(op, v1, v2);
                    } else if (IS_STRING(op1) && IS_STRING(op2)) {
                        auto v1 = AS_CPPSTRING(op1);
                        auto v2 = AS_CPPSTRING(op2);
                        COMPARE_VALUES(op, v1, v2);
                    }
                    break;
                }

                case OP_JMP_IF_FALSE: {
                    auto cond = AS_BOOLEAN(pop());
                    auto address = READ_SHORT();
                    if (!cond) {
                        ip = TO_ADDRESS(address);
                    }
                    break;
                }

                case OP_JMP: {
                    auto address = READ_SHORT();
                    ip = TO_ADDRESS(address);
                    break;
                }
                default:
                    break;
                    // DIE << "Unknown opcode: " << std::hex << static_cast<int>(opcode);
            }
        }
    }

    /**
     * Pushes a value onto the stack.
     */
    void push(EvaValue value) {
        if ((size_t) (sp - stack.begin()) == STACK_LIMIT) {
            DIE << "push(): Stack overflow.\n";
        }
        *sp = value;
        ++sp;
    }

    /**
     * Pops a value from the stack.
     */
    EvaValue pop() {
        if (stack.size() == 0) {
            DIE << "pop(): empty stack.\n";
        }
        --sp;
        return *sp;
    }

    /**
     * Compiler
     */
    std::unique_ptr<EvaCompiler> compiler;

    /**
     * Parser.
     */
    std::unique_ptr<EvaParser> parser;

    /**
     * Instruction pointer (aka Program Counter).
     */
    uint8_t *ip;

    /**
     * Stack pointer.
     */
    EvaValue *sp;

    /**
     * Operands stack.
     */
    std::array<EvaValue, STACK_LIMIT> stack;

    /**
     * Code Object;
     */
    CodeObject *co;
};

#endif //RETROSEVAVM_EVAVM_H
