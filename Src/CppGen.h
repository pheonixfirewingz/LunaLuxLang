#pragma once
#ifdef DEBUG_MODE
#define _GLIBCXX_DEBUG 1
#endif
#include <Ast.hpp>
#include <sstream>
#include <string>
namespace LunaLuxLang
{
class CppGen
{
    CppGen *inst;
    std::stringstream code;
    bool is_caller = false;
    void gen(const Ptr<BinaryOp> &node) noexcept;
    void gen(const Module &node) noexcept;
    void gen(const ModuleImport &node) noexcept;
    void gen(const ModuleBlock &node) const noexcept;
    void gen(const Function &node) noexcept;
    void gen(const Variable &node) noexcept;
    void gen(const Return &node) noexcept;
    void gen(const ExeBlock &node) const noexcept;
    void gen(const FunctionCall &node,bool alone = false) noexcept;
    void gen(const BinaryOp &node) noexcept;
    void gen(const ArgsList &node) noexcept;
    void gen(const Nullptr &node) noexcept;
    void gen(const VariableDefine &node) noexcept;
    void gen(const floatConstant &node) noexcept;
    void gen(const StringConstant &node) noexcept;
    void gen(const IntConstant &node) const noexcept;
    void gen(const int64_t &node) noexcept;
    void gen(const uint64_t &node) noexcept;

  public:
    explicit CppGen() noexcept
    {
        inst = this;
    }
    [[nodiscard]] std::string operator()(Module module) noexcept;
};
} // namespace LunaLuxLang