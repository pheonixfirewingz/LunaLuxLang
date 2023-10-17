#pragma once
#ifdef DEBUG_MODE
#define _GLIBCXX_DEBUG 1
#endif
#include <Lexer.h>
#include <libos/DataType.h>
#include <optional>
#include <string>
#include <variant>
#include <vector>
namespace LunaLuxLang
{
struct DataInfo
{
    const Type data_type;
    const bool is_global : 1;
    const bool is_ptr : 1;
    const bool is_ref : 1;
    const bool is_mutable : 1;
    explicit DataInfo(const Type type_in, bool is_global_in, bool is_ptr_in, bool is_ref_in,
                      bool is_mutable_in) noexcept
        : data_type(type_in)
        , is_global(is_global_in)
        , is_ptr(is_ptr_in)
        , is_ref(is_ref_in)
        , is_mutable(is_mutable_in)
    {
    }
};

template<typename T> struct Ptr
{
    const T *ptr;
    explicit Ptr(const T *ptr_in)
        : ptr(std::move(ptr_in))
    {
    }
    ~Ptr()
    {
        if (!ptr)
            delete ptr;
    }

    const T dePtr() const
    {
        return *ptr;
    }
};

struct IntConstant
{
    const std::variant<const uint64_t, const int64_t> number;
    explicit IntConstant(const std::variant<const uint64_t, const int64_t> number_in) noexcept
        : number(std::move(number_in))
    {
    }
};

struct FloatConstant
{
    const float64_t number;
    explicit FloatConstant(const float64_t number_in) noexcept
        : number(number_in)
    {
    }
};

struct StringConstant
{
    const std::string str;
    explicit StringConstant(const std::string str_in) noexcept
        : str(std::move(str_in))
    {
    }
};

struct Nullptr
{
};

struct VariableDefine
{
    const std::string name;
    const std::optional<DataInfo> data_info;
    explicit VariableDefine(const std::string name_in, const std::optional<DataInfo> data_info_in = {}) noexcept
        : name(std::move(name_in))
        , data_info(std::move(data_info_in))
    {
    }
};

struct ArgsList
{
    std::vector<std::variant<const VariableDefine, const FloatConstant, const IntConstant,const StringConstant, const Nullptr>> args;
};

struct FunctionCall
{
    const std::string name;
    const ArgsList args;
    explicit FunctionCall(const std::string name_in, const ArgsList args_in) noexcept
        : name(std::move(name_in))
        , args(std::move(args_in))
    {
    }
};

struct BinaryOp
{
    const Type op_type;
    const std::variant<const FloatConstant, const IntConstant, const VariableDefine, const FunctionCall,
                       const Ptr<BinaryOp>>
        left;
    const std::variant<const FloatConstant, const IntConstant, const VariableDefine, const FunctionCall,
                       const Ptr<BinaryOp>>
        right;
    explicit BinaryOp(const Type op_type_in,
                      const std::variant<const FloatConstant, const IntConstant, const VariableDefine,
                                         const FunctionCall, const Ptr<BinaryOp>>
                          left_in,
                      const std::variant<const FloatConstant, const IntConstant, const VariableDefine,
                                         const FunctionCall, const Ptr<BinaryOp>>
                          right_in) noexcept
        : op_type(op_type_in)
        , left(std::move(left_in))
        , right(std::move(right_in))
    {
    }
};

struct Variable
{
    const std::string name;
    const DataInfo data_info;
    const std::variant<const FloatConstant, const IntConstant, const Nullptr, const FunctionCall, const BinaryOp>
        init_data;
    explicit Variable(
        const std::string name_in, const DataInfo data_info_in,
        const std::variant<const FloatConstant, const IntConstant, const Nullptr, const FunctionCall, const BinaryOp>
            init_data_in = Nullptr()) noexcept
        : name(std::move(name_in))
        , data_info(std::move(data_info_in))
        , init_data(std::move(init_data_in))
    {
    }
};

struct Return
{
    const std::variant<const IntConstant, const FloatConstant, const FunctionCall, const VariableDefine, const Nullptr,
                       const BinaryOp>
        data;
    explicit Return(const std::variant<const IntConstant, const FloatConstant, const FunctionCall, const VariableDefine,
                                       const Nullptr, const BinaryOp>
                        data_in) noexcept
        : data(std::move(data_in))
    {
    }
};

struct ExeBlock
{
    std::vector<std::variant<const Variable, const Return, const FunctionCall>> statements;
};

struct Function
{
    const DataInfo ret_data_info;
    const bool is_public;
    const std::string name;
    const std::variant<const ExeBlock, const Return> data;
    const ArgsList args;
    explicit Function(const DataInfo ret_data_info_in, const bool is_public_in, const std::string name_in,
                      const std::variant<const ExeBlock, const Return> data_in, const ArgsList args_in) noexcept
        : ret_data_info(ret_data_info_in)
        , is_public(is_public_in)
        , name(std::move(name_in))
        , data(std::move(data_in))
        , args(std::move(args_in))
    {
    }
};

enum class OsType
{
    COMMON,
    WINDOWS,
    MACOS,
    LINUX,
    BSD
};

struct ModuleBlock
{
    const OsType type;
    std::vector<std::variant<const Variable, const Function>> statements;
    explicit ModuleBlock(const OsType type_in)
        : type(type_in)
    {
    }
};

struct ModuleImport
{
    const bool cpp;
    std::vector<std::string> path_part;
    explicit ModuleImport(bool cpp_in)
        : cpp(cpp_in)
    {
    }
};

struct Module
{
    std::vector<std::variant<const ModuleImport,ModuleBlock>> statements;
    const std::string name;
    explicit Module(const std::string name_in = {}) noexcept
        : name(std::move(name_in))
    {
    }
};
} // namespace LunaLuxLang