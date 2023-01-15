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
#include "Global.h"
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
    EvaVM() :
        parser(std::make_unique<EvaParser>()),
        global(std::make_shared<Global>()),
        compiler(std::make_unique<EvaCompiler>(global)) {
        setGlobalVariables();
    }

    /*
     * Executes a program.
     */
    EvaValue exec(const std::string &program) {
        // 1. parse the program
        auto ast = parser->parse("(begin " + program + ")");

        // 2. Compile program to Eva bytecode
        co = compiler->compile(ast);


        // code = {OP_CONST, 0, OP_CONST, 1, OP_ADD, OP_HALT};

        // 3. set instruction pointer to the beginning:
        ip = &co->code[0];

        sp = &stack[0];
        bp = &stack[0];

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

                // -------------------------
                // Global Variable value:
                case OP_GET_GLOBAL: {
                    auto globalIndex = READ_BYTE();
                    push(global->get(globalIndex).value);
                    break;
                }

                case OP_SET_GLOBAL: {
                    auto globalIndex = READ_BYTE();
                    auto value = peek(0);
                    global->set(globalIndex, value);
                    push(global->get(globalIndex).value);
                    break;
                }

                case OP_POP: {
                    pop();
                    break;
                }

                case OP_GET_LOCAL: {
                    auto localIndex = READ_BYTE();
                    if (localIndex < 0 || localIndex >= stack.size()) {
                        DIE << "OP_GET_LOCAL: invalid variable index: " << (int)localIndex;
                    }
                    push(bp[localIndex]);
                    break;
                }

                case OP_SET_LOCAL: {
                    auto localIndex = READ_BYTE();
                    auto value = peek(0);
                    if (localIndex < 0 || localIndex >= stack.size()) {
                        DIE << "OP_SET_LOCAL: invalid variable index: " << (int)localIndex;
                    }
                    bp[localIndex] = value;
                    break;
                }

                case OP_SCOPE_EXIT: {
                    auto count = READ_BYTE();

                    // move the result above the vars:
                    *(sp - 1 - count) = peek(0);

                    popN(count);
                    break;
                }


                default:
                    DIE << "Unknown opcode: " << std::hex << static_cast<int>(opcode);
            }

            for (int i = 0; i < STACK_LIMIT; i++)
                if (std::abs(AS_NUMBER(stack[i]) - 0) > 1e-6)
                    std::cout << AS_NUMBER(stack[i]) << " ";
            std::cout << std::endl;
            std::cout << sp - bp << std::endl;
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
     * Peeks an element from the stack.
     */
    EvaValue peek(size_t offset = 0) {
        if (stack.size() == 0) {
            DIE << "peek(): empty stack.\n";
        }
        return *(sp - 1 - offset);
    }

    /**
     * Pops multiple values from the stack.
     */
    void popN(size_t count) {
        if (stack.size() == 0) {
            DIE << "popN(): empty stack.\n";
        }
        sp -= count;
    }


    /**
     * Sets up global variables and functions.
     */
    void setGlobalVariables() {
        global->addConst("VERSION", 1);
        global->addConst("y", 20);
    }

    /**
     * Global objects.
     */
    std::shared_ptr<Global> global;

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
     * Base pointer.
     */
    EvaValue *bp;

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
