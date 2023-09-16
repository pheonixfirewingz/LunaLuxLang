#pragma once
#ifdef DEBUG_MODE
#    define _GLIBCXX_DEBUG 1
#endif
#include <string>
#include <vector>
namespace LunaLuxLang
{
// string data defined as a number
enum class Type : const unsigned char
{
    IDENTIFIER,           // A-Z + _
    INT_IDENTIFIER,       // 0...-9...
    FLOAT_INT_IDENTIFIER, // 0.0...-9.9...
    IMPORT,               // import
    CPP,                  // cpp
    MODULE,               // module
    FUNC,                 // func
    PUBLIC_FUNC,          // public
    RET,                  // ret
    R_SQUIGGLY,           // }
    L_SQUIGGLY,           // {
    R_BOX,                // ]
    L_BOX,                // [
    R_CURLY,              // )
    L_CURLY,              // (
    SIDE_EYES,            // :
    COMMA,                // ,
    DOT,                  // .
    HASH_TAG,             // #
    ADD,                  // +
    SUB,                  // -
    MUL,                  // *
    DIV,                  // /
    AND,                  // and
    OR,                   // or
    XOR,                  // xor
    MODULO,               // mod
    EQUAL,                // =
    R_ARROW,              // >
    L_ARROW,              // <
    GLOBAL,               // global
    PTR,                  // ptr
    REF,                  // ref
    MUT,                  // mut
    INT8,                 // int8
    INT16,                // int16
    INT32,                // int32
    INT64,                // int64
    UINT8,                // uint8
    UINT16,               // uint16
    UINT32,               // uint32
    UINT64,               // uint64
    FLOAT32,              // float32
    FLOAT64,              // float64
    STRING,               // string
    VEC2,                 // vec2
    VEC3,                 // vec3
    VEC4,                 // vec4
    QUAT,                 // quat
    MATRIX4,              // matrix4
    VOID,                 // void
    WINDOWS,              // windows
    MACOS,                // mac
    LINUX,                // linux
    BSD,                  // bsd
};

struct Span
{
    long offset;
    long size;
    unsigned long line;
    Type type;
};

[[nodiscard]] std::vector<Span> Lexer(const std::string &input) noexcept;
} // namespace LunaLuxLang