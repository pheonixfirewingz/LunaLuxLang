#include <TypeChecker.h>
#include <libos/Error.h>

namespace LunaLuxLang
{
void TypeChecker::error(const char *msg) noexcept
{
    losPrintError(msg);
}

void TypeChecker::error(const char *msg, const Function func) noexcept
{
    error(std::string(((("TYPE CHECKER ERROR: Function - \"") + func.name) + "\" ") + msg).c_str());
}

bool TypeChecker::operator()(Module module) noexcept
{
    bool error_ = false;
    for (auto &stm : module.statements)
    {
        if (std::holds_alternative<ModuleBlock>(stm))
        {
            const auto temp = std::get<ModuleBlock>(stm);
            for (auto &obj : temp.statements)
                std::visit([&inst = inst](const auto &stm_) { inst->register_(stm_); }, obj);
        }
    }

    for (auto &stm : module.statements)
    {
        if (std::holds_alternative<ModuleBlock>(stm))
        {
            const auto temp = std::get<ModuleBlock>(stm);
            for (auto &func : temp.statements)
            {
                if (std::holds_alternative<const Function>(func))
                {
                    if (error_)
                        (void)checkFunction(std::get<const Function>(func));
                    else
                        error_ = checkFunction(std::get<const Function>(func));
                }
            }
        }
    }
    return error_;
}

bool TypeChecker::checkFunction(const Function &func) noexcept
{
    bool error_ = false;
    for (auto &arg : func.args.args)
    {
        if (std::holds_alternative<const VariableDefine>(arg))
        {
            auto &arg_ = std::get<const VariableDefine>(arg);
            [[unlikely]] if (global_variable_lookup.find(arg_.name) != global_variable_lookup.end())
            {
                error_ = true;
                error("Has an Arg that clashes with a global variable name", func);
            }
            variable_stack.emplace(
                std::make_pair<const std::string &, const DataInfo &>(arg_.name, arg_.data_info.value()));
        }
    }

    auto body = func.data;
    auto ret_dat = func.ret_data_info;
    [[likely]] if (std::holds_alternative<const ExeBlock>(body))
    {
        for (auto stm : std::get<const ExeBlock>(body).statements)
        {
        }
    }
    else
    {
        // finish return type checking and testing
        auto ret = std::get<const Return>(body);
        if (std::holds_alternative<const IntConstant>(ret.data))
        {
            if (std::holds_alternative<const int64_t>(std::get<const IntConstant>(ret.data).number))
            {
                if (ret_dat.data_type != Type::INT8 || ret_dat.data_type != Type::INT16 ||
                    ret_dat.data_type != Type::INT32 || ret_dat.data_type != Type::INT64)
                {
                    error_ = true;
                    error("return data dose not have the correct type", func);
                }
            }
            else
            {
                if (ret_dat.data_type != Type::UINT8 || ret_dat.data_type != Type::UINT16 ||
                    ret_dat.data_type != Type::UINT32 || ret_dat.data_type != Type::UINT64)
                {
                    error_ = true;
                    error("return data dose not have the correct type", func);
                }
            }
        }
        else if (std::holds_alternative<const FloatConstant>(ret.data))
        {
            if(ret_dat.data_type != Type::FLOAT32 ||
                 ret_dat.data_type != Type::FLOAT64)
            {
            error_ = true;
            error("return data dose not have the correct type", func);
            }
        }

    }
    variable_stack.clear();
    return error_;
}
} // namespace LunaLuxLang
