//
// Created by Retros on 2022/11/5.
//

#ifndef RETROSEVAVM_EVAVALUE_H
#define RETROSEVAVM_EVAVALUE_H

#include <string>

struct EvaValue;

enum class EvaValueType {
    NUMBER,
    OBJECT
};

enum class ObjectType {
    STRING,
    CODE,
};

struct Object {
    Object(ObjectType type): type(type) {}
    ObjectType type;
};

struct StringObject: public Object {
    StringObject(const std::string& str)
        : Object(ObjectType::STRING), string(str) {}
    std::string string;
};

struct CodeObject: public Object {
    CodeObject(const std::string& name) : Object(ObjectType::CODE), name(name) {}
    /**
     * Name of the unit (usually function name).
     */
    std::string name;
    /**
     * Constant pool.
     */
    std::vector<EvaValue> constants;

    /**
     * Bytecode.
     */
    std::vector<uint8_t> code;
};


struct EvaValue {
    EvaValueType type;
    union {
        double number;
        Object* object;
    };
};

// ------------------------------------------------------------
// Constructors:

#define NUMBER(value) ((EvaValue){EvaValueType::NUMBER, .number = value})

#define ALLOC_STRING(value) ((EvaValue){EvaValueType::OBJECT, .object = new StringObject(value)})

#define ALLOC_CODE(name) ((EvaValue){EvaValueType::OBJECT, .object = new CodeObject(name)})


// ------------------------------------------------------------
// Accessors:
#define AS_NUMBER(evaValue) ((double)(evaValue).number)
#define AS_OBJECT(evaValue) ((Object*)(evaValue).object)

#define AS_STRING(evaValue) ((StringObject*)(evaValue).object)
#define AS_CPPSTRING(evaValue) (AS_STRING(evaValue)->string)

#define AS_CODE(evaValue) ((CodeObject*)(evaValue).object)


// ------------------------------------------------------------
// Testers:
#define IS_NUMBER(evaValue) ((evaValue).type == EvaValueType::NUMBER)
#define IS_OBJECT(evaValue) ((evaValue).type == EvaValueType::OBJECT)

#define IS_OBJECT_TYPE(evaValue, objectType) \
    (IS_OBJECT(evaValue) && AS_OBJECT(evaValue)->type == objectType)

#define IS_STRING(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::STRING)

#define IS_CODE(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::CODE)

std::string evaValueToTypeString(const EvaValue& evaValue) {
    if (IS_NUMBER(evaValue)) {
        return "NUMBER";
    } else if (IS_STRING(evaValue)) {
        return "STRING";
    } else if (IS_CODE(evaValue)) {
        return "CODE";
    } else {
        DIE << "evaValueToTypeString: unknown type " << (int)evaValue.type;
    }
    return "";
}

std::string evaValueToConstantString(const EvaValue& evaValue) {
    std::stringstream ss;
    if (IS_NUMBER(evaValue)) {
        ss << evaValue.number;
    } else if (IS_STRING(evaValue)) {
        ss << '"' << AS_CPPSTRING(evaValue) << '"';
    } else if (IS_CODE(evaValue)) {
        auto code = AS_CODE(evaValue);
        ss << "code " << code << ": " << code->name;
    } else {
        DIE << "evaValueToConstantString: unknown type " << (int)evaValue.type;
    }
    return ss.str();
}

/**
 * Output stream.
 */
std::ostream& operator<< (std::ostream& os, const EvaValue& evaValue) {
    return os << "EvaValue (" << evaValueToTypeString(evaValue)
              << "): " << evaValueToConstantString(evaValue);
}


#endif //RETROSEVAVM_EVAVALUE_H