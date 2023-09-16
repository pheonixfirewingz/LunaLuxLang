#pragma once
#ifdef DEBUG_MODE
#define _GLIBCXX_DEBUG 1
#endif
#include <Ast.hpp>
namespace LunaLuxLang
{

class Parser
{
    const std::vector<Span> input;
    const std::string source;
    size_t current_index = 0;
    inline void error(const char *msg) const noexcept;
    inline void warn(const char *msg) const noexcept;
    [[nodiscard]] const bool isDataType(const Type type, bool full = false) const noexcept;
    [[nodiscard]] const bool isOpType(const Type type) const noexcept;
    [[nodiscard]] const bool isOsType(const Type type) const noexcept;
    [[nodiscard]] inline const std::string extractString() noexcept;
    [[nodiscard]] std::optional<std::variant<const BinaryOp, const Ptr<BinaryOp>>> parseBinary(
        bool is_sub = false) noexcept;
    [[nodiscard]] std::optional<const std::variant<const floatConstant, const IntConstant, const VariableDefine,
                                                   const FunctionCall, const Ptr<BinaryOp>>>
    parseBinaryLeaf() noexcept;
    [[nodiscard]] std::optional<const ExeBlock> parseBlock() noexcept;
    [[nodiscard]] std::optional<const Return> parseReturn() noexcept;
    [[nodiscard]] std::optional<const FunctionCall> parseCaller() noexcept;
    [[nodiscard]] std::optional<const Variable> parseVariable(bool is_global = false) noexcept;
    [[nodiscard]] std::variant<const IntConstant, const floatConstant> parseConstant() noexcept;

    inline const std::optional<const Span> eatTokenSpan() noexcept
    {
        if (current_index >= input.size())
            return {};
        return input[current_index++];
    }

    inline const std::optional<const Span> peekTokenSpan(const size_t offset = 0) const noexcept
    {
        if (current_index + offset >= input.size())
            return {};
        return input[current_index + offset];
    }

  public:
    explicit Parser(const std::string source_in, const std::vector<Span> input_in) noexcept
        : input(std::move(input_in))
        , source(std::move(source_in))
    {
    }
    [[nodiscard]] std::optional<ModuleBlock> parse(OsType type);
    [[nodiscard]] std::optional<Module> operator()() noexcept;
};
} // namespace LunaLuxLang