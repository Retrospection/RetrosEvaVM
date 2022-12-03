//
// Created by Retros on 2022/11/5.
//

#ifndef RETROSEVAVM_OPCODE_H
#define RETROSEVAVM_OPCODE_H

#include "Logger.h"

/**
 * Stops the program
 */
#define OP_HALT 0x00

/**
 * Pushes a const onto the stack.
 */
#define OP_CONST 0x01


#define OP_ADD 0x02
#define OP_SUB 0x03
#define OP_MUL 0x04
#define OP_DIV 0x05

#define OP_COMPARE 0x06

/**
 * Control flow: jump if the value on the stack is false.
 */
#define OP_JMP_IF_FALSE 0x07

/**
 * Unconditional jump
 */
#define OP_JMP 0x08

/**
 * Returns global variable.
 */
#define OP_GET_GLOBAL 0x09

/**
 * Sets global variable value.
 */
#define OP_SET_GLOBAL 0x10

/**
 * Pops the value from the stack.
 */
#define OP_POP 0x11

/**
 * Returns local variable.
 */
#define OP_GET_LOCAL 0x12

/**
 * Sets local variable value.
 */
#define OP_SET_LOCAL 0x13

/**
 * Pops the value from the stack.
 */
#define OP_SCOPE_EXIT 0x14

// -----------------------------------------------------------

#define OP_STR(op) \
  case OP_##op:    \
    return #op

std::string opcodeToString(uint8_t opcode) {
    switch (opcode) {
        OP_STR(HALT);
        OP_STR(CONST);
        OP_STR(ADD);
        OP_STR(SUB);
        OP_STR(MUL);
        OP_STR(DIV);
        OP_STR(COMPARE);
        OP_STR(JMP_IF_FALSE);
        OP_STR(JMP);
        OP_STR(GET_GLOBAL);
        OP_STR(SET_GLOBAL);
        OP_STR(POP);
        OP_STR(GET_LOCAL);
        OP_STR(SET_LOCAL);
        OP_STR(SCOPE_EXIT);
        default: {
            DIE << "opcodeToString: unknown opcode: " << (int) opcode;
        }
        return "Unknown";
    }
}

#endif //RETROSEVAVM_OPCODE_H
