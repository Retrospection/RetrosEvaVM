//
// Created by Retros on 2022/11/5.
//

#ifndef RETROSEVAVM_EVACOMPILER_H
#define RETROSEVAVM_EVACOMPILER_H


#include "EvaValue.h"
#include "OpCode.h"
#include "parser/EvaParser.h"

#include <map>
#include <string>




#define ALLOC_CONST(TESTER, ACCESSOR, ALLOCATOR, VALUE)                             \
  do {                                                                              \
    for (auto i = 0; i < co->constants.size(); i++) {                               \
      if (!TESTER(co->constants[i])) {                                              \
        continue;                                                                   \
      }                                                                             \
      if (ACCESSOR(co->constants[i]) == VALUE) {                                    \
        return i;                                                                   \
      }                                                                             \
    }                                                                               \
    co->constants.push_back(ALLOCATOR(value));                                      \
} while (false)



// Generic binary operator: (+ 1 2) OP_CONST, OP_CONST, OP_ADD
#define GEN_BINARY_OP(op)  \
    do {                   \
        gen(exp.list[1]);  \
        gen(exp.list[2]);  \
        emit(op);          \
    } while (false)





class EvaCompiler {
public:
    EvaCompiler() {}

    /**
     * Main compile API.
     */
    CodeObject* compile(const Exp& exp) {
        // Allocate new code object;
        co = AS_CODE(ALLOC_CODE("main"));

        gen(exp);

        return co;
    }

    /**
     * Main compile loop.
     */
    void gen(const Exp& exp) {
        switch (exp.type) {
            /**
             * -------------------------------------------------------
             * Numbers.
             */
            case ExpType::NUMBER: {
                emit(OP_CONST);
                emit(numericConstIdx(exp.number));
                break;
            }

            /**
             * -------------------------------------------------------
             * Strings.
             */
            case ExpType::STRING: {
                emit(OP_CONST);
                emit(stringConstIdx(exp.string));
                break;
            }

            /**
             * -------------------------------------------------------
             * Symbols (variables, operators).
             */
            case ExpType::SYMBOL: {
                /*
                 * Boolean
                 */
                if (exp.string == "true" || exp.string == "false") {
                    emit(OP_CONST);
                    emit(booleanConstIdx(exp.string == "true") ? true : false);
                }
                break;
            }

            /**
             * -------------------------------------------------------
             * List.
             */
            case ExpType::LIST: {
                auto tag = exp.list[0];

                /**
                 * -------------------------------------------------------
                 * Special cases.
                 */
                if (tag.type == ExpType::SYMBOL) {
                    auto op = tag.string;

                    // -------------------------------------------------------
                    // Binary math operations.
                    if (op == "+") {
                        GEN_BINARY_OP(OP_ADD);
                    }

                    else if (op == "-") {
                        GEN_BINARY_OP(OP_SUB);
                    }

                    else if (op == "*") {
                        GEN_BINARY_OP(OP_MUL);
                    }

                    else if (op == "/") {
                        GEN_BINARY_OP(OP_DIV);
                    }
                    // -------------------------------------------------------
                    // Compare operations.
                    else if (compareOps_.count(op) != 0) {
                        gen(exp.list[1]);
                        gen(exp.list[2]);
                        emit(OP_COMPARE);
                        emit(compareOps_[op]);
                    }

                    // -------------------------------------------------------
                    // Branch instuction.

                    /**
                     * (if <test> <consequent> <alternate>)
                     */
                    if (op == "if") {
                        gen(exp.list[1]);

                        // Else branch. Init with 0 address, will be patched.
                        emit(OP_JMP_IF_FALSE);
                        // we use 2-byte address
                        emit(0);
                        emit(0);

                        auto elseJmpAddr = getOffset() - 2;

                        // Emit <consequent>
                        gen(exp.list[2]);

                        emit(OP_JMP);
                        // we use 2-byte address
                        emit(0);
                        emit(0);

                        auto endAddr = getOffset() - 2;

                        // patch the else branch address.
                        auto elseBranchAddr = getOffset();
                        patchJumpAddress(elseJmpAddr, elseBranchAddr);

                        // Emit <alternate> if we have it.
                        if (exp.list.size() == 4) {
                            gen(exp.list[3]);
                        }

                        // Patch the end.
                        auto endBranchAddr = getOffset();
                        patchJumpAddress(endAddr, endBranchAddr);
                    }
                }
                break;
            }

        }
    }

private:

    /**
     * Returns current bytecode offset.
     */
    size_t getOffset() { return co->code.size(); }

    /**
     * Allocates a numeric constant.
     */
    size_t numericConstIdx(double value) {
        ALLOC_CONST(IS_NUMBER, AS_NUMBER, NUMBER, value);
        return co->constants.size() - 1;
    }

    /**
     * Allocates a boolean constant.
     */
    size_t booleanConstIdx(bool value) {
        ALLOC_CONST(IS_BOOLEAN, AS_BOOLEAN, BOOLEAN, value);
        return co->constants.size() - 1;
    }

    /**
     * Allocates a string constant.
     */
    size_t stringConstIdx(const std::string& value) {
        ALLOC_CONST(IS_STRING, AS_CPPSTRING, ALLOC_STRING, value);
        return co->constants.size() - 1;
    }

    /**
     * Emits data to the bytecode.
     */
    void emit(uint8_t code) { co->code.push_back(code); }

    /**
     * Writes byte at offset.
     */
    void writeByteAtOffset(size_t offset, uint8_t value) {
        co->code[offset] = value;
    }

    /**
     * Patches jump address.
     */
    void patchJumpAddress(size_t offset, uint16_t value) {
        writeByteAtOffset(offset, (value >> 8) & 0xff);
        writeByteAtOffset(offset + 1, value & 0xff);
    }

    /**
     * Compiling code object.
     */
    CodeObject* co;

    /**
     * Compare ops map.
     */
    static std::map<std::string, uint8_t> compareOps_;
};

std::map<std::string, uint8_t> EvaCompiler::compareOps_ = {
        { "<", 0 }, { ">", 1 } , { "==", 2 },
        { ">=", 3 }, { "<=", 4 }, { "!=", 5 },
};

#endif //RETROSEVAVM_EVACOMPILER_H
