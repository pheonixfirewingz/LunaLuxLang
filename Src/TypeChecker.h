#pragma once
#include <Ast.hpp>
#include <Lexer.h>
#include <map>
#include <string>
namespace LunaLuxLang
{

class TypeChecker
{
    TypeChecker *inst;
    std::map<const std::string, const Function> function_lookup;
    std::map<const std::string, const Variable> global_variable_lookup;
    std::map<const std::string, const DataInfo> variable_stack;

    void error(const char* msg) noexcept;
    void error(const char *msg, const Function func) noexcept;

        protected : void register_(const Variable var)
    {
        global_variable_lookup.insert({var.name,var});
    }

    void register_(const Function fun)
    {
        function_lookup.insert({fun.name, fun});
    }

  public:
    explicit TypeChecker()
    {
        inst = this;
    };
    [[nodiscard]] bool operator()(Module module) noexcept;
    [[nodiscard]] bool checkFunction(const Function& func) noexcept;
};
} // namespace LunaLuxLang
