#include <CppGen.h>
#include <cstring>
#include <libos/DataType.h>
#include <libos/Error.h>
#include <libos/FileIO.h>
#include <string>

namespace LunaLuxLang
{
static inline const char *TypeToString(Type type)
{
    switch (type)
    {
    case Type::INT8:
        return "int8_t";
    case Type::INT16:
        return "int16_t";
    case Type::INT32:
        return "int32_t";
    case Type::INT64:
        return "int64_t";
    case Type::UINT8:
        return "uint8_t";
    case Type::UINT16:
        return "uint16_t";
    case Type::UINT32:
        return "uint32_t";
    case Type::UINT64:
        return "uint64_t";
    case Type::FLOAT32:
        return "float32_t";
    case Type::FLOAT64:
        return "float64_t";
    case Type::STRING:
        return "std::string";
    case Type::VEC2:
        return "runtime::vec2";
    case Type::VEC3:
        return "runtime::vec3";
    case Type::VEC4:
        return "runtime::vec4";
    case Type::QUAT:
        return "runtime::quat";
    case Type::VOID:
        return "void";
    case Type::MATRIX4:
        return "runtime::matrix4";
    default:
        return "TYPE_ERROR";
    }
}

void CppGen::gen(const Module &node) noexcept
{
    code << "#include <cstdint>\n";
    code << "#include <string>\n";
    std::string source;
    {
        losFileOpenInfo open_info = {};
        open_info.fileBits = LOS_FILE_BIT_READ;
        const char *ar = "$[binary_base]/LunaLuxLangRunTime.cpp";
        open_info.path = ar;
        open_info.path_size = std::strlen(ar);
        losFileHandle handle;
        if (losOpenFile(&handle, open_info) == LOS_SUCCESS)
        {
            char *buffer;
            size_t buffer_size = 0;
            if (losReadFile(handle, reinterpret_cast<void **>(&buffer), &buffer_size) != LOS_SUCCESS)
            {
                losCloseFile(handle);
                source = "/*Failed To read LunaLuxLangRunTime file*/\n";
            }
            else
            {
                losCloseFile(handle);
                source = std::string(std::move(buffer), std::move(buffer_size));
                delete buffer;
            }
        }
        else
            source = "/*Failed To Load LunaLuxLangRunTime*/\n";
    }
    for (const auto & _import_ : node.statements)
    {
        if (std::holds_alternative<const ModuleImport>(_import_))
            gen(std::get<const ModuleImport>(_import_));
    }
    code << source;
    code << "\nnamespace ";
    code << node.name;
    code << "\n{\n";
    for (const auto &block : node.statements)
    {
        if(std::holds_alternative<ModuleBlock>(block))
            gen(std::get<ModuleBlock>(block));
    }
    code << "\n}\n";
}

void CppGen::gen(const ModuleImport &node) noexcept
{
    if (node.cpp)
    {
        code << "#include <";
        for (size_t i = 0; i < node.path_part.size() - 1; i++)
        {
            code << node.path_part[i];
            code << "/";
        }
        code << node.path_part.back();
        code << ".h>\n";
    }
    else
    {
        losPrintError("CPPGen: import for LunaLuxLang source not supported yet");
        std::exit(EXIT_FAILURE);
    }
}

static inline const OsType os()
{
#if defined(ON_LINUX)
    return OsType::LINUX;
#endif
#if defined(ON_WINDOW)
    return OsType::WINDOWS;
#endif
#if defined(ON_MACOS)
    return OsType::MACOS;
#endif
#if defined(ON_BSD)
    return OsType::BSD;
#endif
}

void CppGen::gen(const ModuleBlock &node) const noexcept
{
    if (node.type != OsType::COMMON && node.type != os())
        return;

    for (auto stm : node.statements)
        std::visit([&inst = inst](const auto &stm_) { inst->gen(stm_); }, stm);
}

void CppGen::gen(const Function &node) noexcept
{
    if (node.ret_data_info.data_type != Type::VOID && !node.ret_data_info.is_ptr && node.name.compare("main") != 0)
        code << "[[nodiscard]] ";

    if (!node.ret_data_info.is_mutable)
        code << "const ";
    code << TypeToString(node.ret_data_info.data_type);
    if (node.ret_data_info.is_ptr)
        code << "*";
    code << " ";
    code << node.name;
    code << "(";
    gen(node.args);
    code << ")\n{\n";

    std::visit([&inst = inst](const auto &stm_) { inst->gen(stm_); }, node.data);
    code << "\n}\n";
}

void CppGen::gen(const Variable &node) noexcept
{
    if (!node.data_info.is_mutable)
        code << "const ";
    code << TypeToString(node.data_info.data_type);
    if (node.data_info.is_ptr)
        code << "*";
    if (node.data_info.is_ref)
        code << "&";
    code << " ";
    code << node.name;
    code << " = ";
    std::visit([&inst = inst](const auto &stm_) { inst->gen(stm_); }, node.init_data);
    code << ";\n";
}

void CppGen::gen(const Return &node) noexcept
{
    code << "return ";
    std::visit([&inst = inst](const auto &stm_) { inst->gen(stm_); }, node.data);
    code << ";";
}

template<class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void CppGen::gen(const ExeBlock &node) const noexcept
{
    for (auto &stm : node.statements)
        std::visit(overloaded{[&inst = inst](const auto &stm_) { inst->gen(stm_); },
                              [&inst = inst](const FunctionCall &stm_) { inst->gen(stm_, true); }},
                   stm);
}

void CppGen::gen(const floatConstant &node) noexcept
{
    code << std::to_string(node.number);
}

void CppGen::gen(const IntConstant &node) const noexcept
{
    std::visit([&inst = inst](const auto &stm_) { inst->gen(stm_); }, node.number);
}

void CppGen::gen(const int64_t &node) noexcept
{
    code << std::to_string(node);
}

void CppGen::gen(const uint64_t &node) noexcept
{
    code << std::to_string(node);
}

void CppGen::gen(const StringConstant &node) noexcept
{
    code << node.str;
}

void CppGen::gen(const VariableDefine &node) noexcept
{
    if (auto &data_info = node.data_info; data_info.has_value())
    {
        if (std::string str = TypeToString(data_info.value().data_type); str.compare("TYPE_ERROR") != 0)
        {
            if (is_caller)
                code << "/*";
            if (!data_info.value().is_mutable)
                code << "const ";
            code << str;
            if (data_info.value().is_ptr)
                code << "*";
            if (data_info.value().is_ref)
                code << "&";
            if (is_caller)
                code << "*/ ";
            else
                code << " ";
        }
    }
    code << node.name;
}

void CppGen::gen(const FunctionCall &node,bool alone) noexcept
{
    is_caller = true;
    code << node.name;
    code << "(";
    gen(node.args);
    code << ")";
    if(alone)
    {
        code << ";\n";
    }
    is_caller = false;
}

void CppGen::gen(const Nullptr &node) noexcept
{
    code << "nullptr";
}

void CppGen::gen(const Ptr<BinaryOp> &node) noexcept
{
    gen(*static_cast<const BinaryOp *>(node.ptr));
}

void CppGen::gen(const BinaryOp &node) noexcept
{
    std::visit([&inst = inst](const auto &stm_) { inst->gen(stm_); }, node.left);
    switch (node.op_type)
    {
    case Type::ADD:
        code << " + ";
        break;
    case Type::SUB:
        code << " - ";
        break;
    case Type::DIV:
        code << " / ";
        break;
    case Type::MUL:
        code << " * ";
        break;
    case Type::AND:
        code << " & ";
        break;
    case Type::MODULO:
        code << " % ";
        break;
    case Type::XOR:
        code << " ^ ";
        break;
    case Type::OR:
        code << " | ";
        break;
    default:
        code << "ERROR_OP_TYPE";
        break;
    }
    std::visit([&inst = inst](const auto &stm_) { inst->gen(stm_); }, node.right);
}

void CppGen::gen(const ArgsList &node) noexcept
{
    if (node.args.empty())
        return;
    for (size_t i = 0; i < node.args.size() - 1; i++)
    {
        auto &stm = node.args[i];
        std::visit([&inst = inst](const auto &stm_) { inst->gen(stm_); }, stm);
        code << ",";
    }
    auto &stm = node.args.back();
    std::visit([&inst = inst](const auto &stm_) { inst->gen(stm_); }, stm);
}
std::string CppGen::operator()(Module _module_) noexcept
{
    gen(_module_);
    return code.str();
}
} // namespace LunaLuxLang