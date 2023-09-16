#include <TypeChecker.h>

namespace LunaLuxLang
{
void TypeChecker::error(const char *msg) noexcept
{
}

bool TypeChecker::operator()(Module module) noexcept
{
    for (auto &stm : module.statements)
    {
        if(std::holds_alternative<ModuleBlock>(stm))
        {
            const auto temp = std::get<ModuleBlock>(stm);
            for (auto &obj : temp.statements)
                std::visit([&inst = inst](const auto &stm_) { inst->register_(stm_); },obj);
        }
    }
    return false;
}
} // namespace LunaLuxLang
