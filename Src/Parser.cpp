#include <Parser.h>
#include <assert.h>
namespace LunaLuxLang
{

#define TYPE_IS_NOT(x)              \
    const auto t = peekTokenSpan(); \
    t.value().type != x

#define IS_NOT(t, x) (t.has_value() && t.value().type != x)

#define IS(t, x) (t.has_value() && t.value().type == x)

#define IS_OR_ERROR(x, msg)         \
    if (IS_NOT(peekTokenSpan(), x)) \
    {                               \
        error(msg);                 \
        return {};                  \
    }

[[nodiscard]] const bool Parser::isDataType(const Type type, bool full) const noexcept
{
    switch (type)
    {
    [[fallthrough]] case Type::INT8:
    [[fallthrough]] case Type::INT16:
    [[fallthrough]] case Type::INT32:
    [[fallthrough]] case Type::INT64:
    [[fallthrough]] case Type::UINT8:
    [[fallthrough]] case Type::UINT16:
    [[fallthrough]] case Type::UINT32:
    [[fallthrough]] case Type::UINT64:
    [[fallthrough]] case Type::FLOAT32:
    [[fallthrough]] case Type::FLOAT64:
    [[fallthrough]] case Type::STRING:
    [[fallthrough]] case Type::VEC2:
    [[fallthrough]] case Type::VEC3:
    [[fallthrough]] case Type::VEC4:
    [[fallthrough]] case Type::QUAT:
    [[fallthrough]] case Type::VOID:
    case Type::MATRIX4:
        return true;
        default : break;
    }
    if (full)
    {
        switch (type)
        {
        [[fallthrough]] case Type::IDENTIFIER:
        [[fallthrough]] case Type::PTR:
        [[fallthrough]] case Type::MUT:
        case Type::REF:
            return true;
            default : break;
        }
    }
    return false;
}

const bool Parser::isOpType(const Type type) const noexcept
{
    switch (type)
    {
    [[fallthrough]] case Type::ADD:
    [[fallthrough]] case Type::SUB:
    [[fallthrough]] case Type::MUL:
    [[fallthrough]] case Type::DIV:
    [[fallthrough]] case Type::AND:
    [[fallthrough]] case Type::OR:
    [[fallthrough]] case Type::XOR:
    case Type::MODULO:
        return true;
        default : break;
    }
    return false;
}

const bool Parser::isOsType(const Type type) const noexcept
{
    switch (type)
    {
    [[fallthrough]] case Type::WINDOWS:
    [[fallthrough]] case Type::MACOS:
    [[fallthrough]] case Type::LINUX:
    case Type::BSD:
        return true;
        default : break;
    }
    return false;
}

[[nodiscard]] const std::string Parser::extractString() noexcept
{
    const auto span = eatTokenSpan().value();
    return source.substr(span.offset, span.size);
}

std::optional<std::variant<const BinaryOp, const Ptr<BinaryOp>>> Parser::parseBinary(bool is_sub) noexcept
{
    if (const auto left = parseBinaryLeaf(); left.has_value())
    {
        const auto op = eatTokenSpan();
        if (!isOpType(op.value().type))
        {
            error("this is not an math operation");
            return {};
        }
        if (isOpType(peekTokenSpan(1).value().type))
        {
            if (const auto right = parseBinary(true); right.has_value())
                return BinaryOp(op.value().type, std::move(left.value()),
                                std::move(std::get<const Ptr<BinaryOp>>(right.value())));
        }
        else
        {
            if (const auto right = parseBinaryLeaf(); right.has_value())
            {
                if (!is_sub)
                    return BinaryOp(op.value().type, std::move(left.value()), std::move(right.value()));
                else
                    return Ptr<BinaryOp>(
                        new BinaryOp(op.value().type, std::move(left.value()), std::move(right.value())));
            }
        }
    }
    return {};
}

std::optional<const std::variant<const FloatConstant, const IntConstant, const VariableDefine, const FunctionCall,
                                 const Ptr<BinaryOp>>>
Parser::parseBinaryLeaf() noexcept
{
    if (IS(peekTokenSpan(), Type::IDENTIFIER) && IS(peekTokenSpan(1), Type::L_CURLY))
    {
        if (const auto caller = parseCaller(); caller.has_value())
            return caller.value();
        else
            return {};
    }
    else if (IS(peekTokenSpan(), Type::IDENTIFIER))
        return VariableDefine(extractString());
    else
    {
        const auto data = parseConstant();
        if (std::holds_alternative<const FloatConstant>(data))
            return std::get<const FloatConstant>(data);
        else
            return std::get<const IntConstant>(data);
    }
    return {};
}

void Parser::error(const char *msg) const noexcept
{
    const Span &span = peekTokenSpan(-1).value();
    const std::string data = source.substr(span.offset, span.size);
    printf("PARSING ERROR: <%lu:%s> -> %s\n", span.line, data.c_str(), msg);
    fflush(stdout);
}

void Parser::warn(const char *msg) const noexcept
{
    const Span &span = peekTokenSpan(-1).value();
    const std::string data = source.substr(span.offset, span.size);
    printf("PARSING WARNING: <%lu:%s> -> %s\n", span.line, data.c_str(), msg);
    fflush(stdout);
}

[[nodiscard]] std::optional<const ExeBlock> Parser::parseBlock() noexcept
{
    ExeBlock block;
    (void)eatTokenSpan();
    while (IS_NOT(peekTokenSpan(), Type::R_SQUIGGLY))
    {
        if (IS(peekTokenSpan(), Type::RET))
        {
            const auto ret = parseReturn();
            if (ret.has_value())
                block.statements.emplace_back(std::move(ret.value()));
            else
                return {};
        }
        else if (IS(peekTokenSpan(), Type::IDENTIFIER) && IS(peekTokenSpan(1), Type::L_CURLY))
        {
            if (const auto caller = parseCaller(); caller.has_value())
                block.statements.emplace_back(std::move(caller.value()));
            else
                return {};
        }
        else if (isDataType(peekTokenSpan().value().type))
        {
            if (auto var = parseVariable(); var.has_value())
                block.statements.emplace_back(std::move(var.value()));
            else
                return {};
        }
        else
            (void)eatTokenSpan();
    }
    (void)eatTokenSpan();
    return std::move(block);
}

std::optional<const Return> Parser::parseReturn() noexcept
{
    (void)eatTokenSpan();
    const auto temp = peekTokenSpan(1);
    if (IS(peekTokenSpan(), Type::IDENTIFIER) && IS(temp, Type::L_CURLY))
    {
        if (const auto caller = parseCaller(); caller.has_value())
            return Return(std::move(caller.value()));
        else
            return {};
    }
    else if (IS(peekTokenSpan(), Type::L_CURLY) ||
             (IS(peekTokenSpan(), Type::INT_IDENTIFIER) && (temp.has_value() && isOpType(temp.value().type))) ||
             (IS(peekTokenSpan(), Type::FLOAT_INT_IDENTIFIER) && (temp.has_value() && isOpType(temp.value().type))) ||
             (IS(peekTokenSpan(), Type::IDENTIFIER) && (temp.has_value() && isOpType(temp.value().type))))
    {
        if (const auto binary = parseBinary(); binary.has_value())
            return Return(std::move(std::get<const BinaryOp>(binary.value())));
        else
            return {};
    }
    else if (IS(peekTokenSpan(), Type::IDENTIFIER))
        return std::move(Return(std::move(VariableDefine(extractString()))));
    else
    {
        const auto data = parseConstant();
        if (std::holds_alternative<const FloatConstant>(data))
            return std::move(Return(std::move(std::get<const FloatConstant>(data))));
        else
            return std::move(Return(std::move(std::get<const IntConstant>(data))));
    }
}

std::optional<const FunctionCall> Parser::parseCaller() noexcept
{
    const std::string name = extractString();
    // check for "("
    (void)eatTokenSpan();
    ArgsList args;
    // check for ")" before we even try to parse it
    for (size_t i = 0; IS_NOT(peekTokenSpan(i), Type::R_CURLY); i++)
    {
        if (IS_NOT(peekTokenSpan(i), Type::INT_IDENTIFIER && IS_NOT(peekTokenSpan(i), Type::FLOAT_INT_IDENTIFIER) &&
                                         IS_NOT(peekTokenSpan(i), Type::IDENTIFIER) &&
                                         IS_NOT(peekTokenSpan(i), Type::STRING) &&
                                         IS_NOT(peekTokenSpan(i), Type::COMMA)))
        {
            error("used something in the function call args that is not allowed");
            return {};
        }
    }
    if (IS(peekTokenSpan(), Type::R_CURLY))
        (void)eatTokenSpan();
    else
    {
        while (IS_NOT(peekTokenSpan(), Type::R_CURLY))
        {
            if (IS(peekTokenSpan(), Type::IDENTIFIER))
                args.args.emplace_back(std::move(VariableDefine(extractString())));
            else if (IS(peekTokenSpan(), Type::STRING))
            {
                args.args.emplace_back(std::move(StringConstant(extractString())));
            }
            else
            {
                const auto data = parseConstant();
                if (std::holds_alternative<const FloatConstant>(data))
                    args.args.emplace_back(std::move(std::get<const FloatConstant>(data)));
                else
                    args.args.emplace_back(std::move(std::get<const IntConstant>(data)));
            }
            IS_OR_ERROR(Type::COMMA && IS_NOT(peekTokenSpan(), Type::R_CURLY), "we think you forgot a comma \',\'")
            if (IS(peekTokenSpan(), Type::COMMA))
                (void)eatTokenSpan();
        }
        (void)eatTokenSpan();
    }
    return FunctionCall(std::move(name), std::move(args));
}

std::optional<const Variable> Parser::parseVariable(bool is_global) noexcept
{
    bool is_mutable = false;
    Type type = eatTokenSpan().value().type;
    if (type == Type::MUT)
    {
        is_mutable = true;
        type = eatTokenSpan().value().type;
    }
    if (type == Type::VOID)
    {
        error(is_global ? "global variable can not be a void" : "variable can not be a void");
        return {};
    }
    bool is_ptr = false;
    bool is_ref = false;
    if (IS(peekTokenSpan(), Type::PTR))
    {
        is_ptr = true;
        (void)eatTokenSpan();
        if (IS(peekTokenSpan(), Type::REF))
        {
            error("ptr refs are not allowed");
            return {};
        }
    }
    else
    {
        if (IS(peekTokenSpan(), Type::REF))
        {
            is_ref = true;
            (void)eatTokenSpan();
        }

        if (IS(peekTokenSpan(), Type::PTR))
        {
            error("ptr refs are not allowed");
            return {};
        }
    }
    DataInfo info(type, is_global, is_ptr, is_ref, is_mutable);
    IS_OR_ERROR(Type::IDENTIFIER, is_global ? "invalid name for global variable" : "invalid name for variable");
    const auto name = extractString();
    IS_OR_ERROR(Type::EQUAL, "we think you missed an \'=\'");
    (void)eatTokenSpan();
    if (!is_global)
    {
        if (IS(peekTokenSpan(), Type::IDENTIFIER) && IS(peekTokenSpan(1), Type::L_CURLY))
        {
            if (const auto caller = parseCaller(); caller.has_value())
                return Variable(std::move(name), std::move(info), std::move(caller.value()));
            else
                return {};
        }
        else if (IS(peekTokenSpan(), Type::L_CURLY) || IS(peekTokenSpan(), Type::INT_IDENTIFIER) ||
                 IS(peekTokenSpan(), Type::FLOAT_INT_IDENTIFIER) || IS(peekTokenSpan(), Type::IDENTIFIER))
        {
            if (const auto binary = parseBinary(); binary.has_value())
                return Variable(std::move(name), std::move(info), std::move(std::get<const BinaryOp>(binary.value())));
            else
                return {};
        }
    }

    const auto data = parseConstant();
    if (std::holds_alternative<const FloatConstant>(data))
        return Variable(std::move(name), std::move(info), std::move(std::get<const FloatConstant>(data)));
    else
        return Variable(std::move(name), std::move(info), std::move(std::get<const IntConstant>(data)));
}

std::variant<const IntConstant, const FloatConstant> Parser::parseConstant() noexcept
{
    if (IS(peekTokenSpan(), Type::INT_IDENTIFIER))
    {
        const std::string number = extractString();
        return IntConstant(number.contains('-') ? std::stoll(number) : std::stoull(number));
    }
    else
        return FloatConstant(std::stod(extractString()));
}

std::optional<ModuleBlock> Parser::parse(OsType type)
{
    ModuleBlock _module_(type);
    bool is_block = false;
NEXT_STATEMENT:
    auto token = eatTokenSpan();
    if (IS(token, Type::L_SQUIGGLY) && _module_.statements.empty())
    {
        is_block = true;
        goto NEXT_STATEMENT;
    }

    if (IS(token, Type::GLOBAL))
    {
        if (auto var = parseVariable(true); var.has_value())
            _module_.statements.emplace_back(std::move(var.value()));
        else
            return {};
    }
    else if (IS(token, Type::FUNC) || IS(token, Type::PUBLIC_FUNC))
    {
        bool is_public = IS(token, Type::PUBLIC_FUNC);
        IS_OR_ERROR(Type::IDENTIFIER, "function name location is not a valid name")
        const std::string name = extractString();
        // check for "("
        IS_OR_ERROR(Type::L_CURLY, "the function args are missing or missing an '('")
        (void)eatTokenSpan();
        ArgsList args;
        // check for ")" before we even try to parse it
        for (size_t i = 0; IS_NOT(peekTokenSpan(i), Type::R_CURLY); i++)
        {
            if (!isDataType(peekTokenSpan(i).value().type, true) && IS_NOT(peekTokenSpan(i), Type::COMMA))
            {
                error("the function args dose not end it's missing ')'");
                return {};
            }
        }

        if (IS(peekTokenSpan(), Type::R_CURLY))
            (void)eatTokenSpan();
        else
        {
            while (IS_NOT(peekTokenSpan(), Type::R_CURLY))
            {
                if (isDataType(peekTokenSpan().value().type) || IS(peekTokenSpan(), Type::MUT))
                {
                    bool is_mutable = false;
                    if (IS(peekTokenSpan(), Type::MUT))
                    {
                        is_mutable = true;
                        (void)eatTokenSpan();
                    }
                    Type type = eatTokenSpan().value().type;
                    bool is_ptr = false;
                    bool is_ref = false;
                    if (IS(peekTokenSpan(), Type::PTR))
                    {
                        is_ptr = true;
                        (void)eatTokenSpan();
                        if (IS(peekTokenSpan(), Type::REF))
                        {
                            error("ptr refs are not allowed");
                            return {};
                        }
                    }
                    else
                    {
                        if (IS(peekTokenSpan(), Type::REF))
                        {
                            is_ref = true;
                            (void)eatTokenSpan();
                            if (IS(peekTokenSpan(), Type::PTR))
                            {
                                error("ptr refs are not allowed");
                                return {};
                            }
                        }
                    }
                    if (!isDataType(type))
                    {
                        error("arg not typed with a valid data type");
                        return {};
                    }
                    if (type == Type::VOID && !is_ptr && !is_ref && is_mutable)
                    {
                        warn("due to type being void mutable is ignored");
                        is_mutable = false;
                    }
                    DataInfo info(type, false, is_ptr, is_ref, is_mutable);
                    IS_OR_ERROR(Type::IDENTIFIER, "invalid name for arg");
                    args.args.emplace_back(std::move(VariableDefine(extractString(), std::move(info))));
                }
                IS_OR_ERROR(Type::COMMA && IS_NOT(peekTokenSpan(), Type::R_CURLY), "we think you forgot a comma \',\'")
                if (IS(peekTokenSpan(), Type::COMMA))
                    (void)eatTokenSpan();
            }
            (void)eatTokenSpan();
        }

        IS_OR_ERROR(Type::SUB, "error missing return arrow syntax")
        (void)eatTokenSpan();
        IS_OR_ERROR(Type::R_ARROW, "error missing return arrow syntax")

        (void)eatTokenSpan();
        bool is_mutable = false;
        if (IS(peekTokenSpan(), Type::MUT))
        {
            is_mutable = true;
            (void)eatTokenSpan();
        }

        if (!isDataType(peekTokenSpan().value().type))
        {
            error("this is not a function valid return type");
            return {};
        }
        const auto data_type = eatTokenSpan().value();
        bool is_ptr = false;
        if (IS(peekTokenSpan(), Type::PTR))
        {
            is_ptr = true;
            (void)eatTokenSpan();
        }

        if (data_type.type == Type::VOID)
            is_mutable = true;

        const DataInfo info(data_type.type, false, is_ptr, false, is_mutable);

        IS_OR_ERROR(Type::RET && IS_NOT(peekTokenSpan(), Type::L_SQUIGGLY),
                    "missing single line function return or function block")

        if (IS(peekTokenSpan(), Type::RET))
        {
            const auto ret = parseReturn();
            if (ret.has_value())
            {
                Function function(std::move(info), std::move(is_public), std::move(name), std::move(ret.value()),
                                  std::move(args));
                _module_.statements.emplace_back(std::move(function));
            }
            else
                return {};
        }
        else
        {
            const auto block = parseBlock();
            if (block.has_value())
            {
                Function function(std::move(info), std::move(is_public), std::move(name), std::move(block.value()),
                                  std::move(args));
                _module_.statements.emplace_back(std::move(function));
            }
            else
            {
                error("failed to parse block");
                return {};
            }
        }
    }
    else
    {
        error("unknown text is in the source file! maybe a comment is missing /* or */");
        return {};
    }

    if (is_block)
    {
        if (IS_NOT(peekTokenSpan(), Type::R_SQUIGGLY))
            goto NEXT_STATEMENT;
        else
            (void)eatTokenSpan();
    }
    else
    {
        if (peekTokenSpan(1).has_value() && !isOsType(peekTokenSpan().value().type))
            goto NEXT_STATEMENT;
    }
    return _module_;
}

[[nodiscard]] std::optional<Module> Parser::operator()() noexcept
{
    if (input.size() < 2)
    {
        error("empty file");
        return {};
    }

    if (TYPE_IS_NOT(Type::MODULE))
        error("this is not a defined module");
    (void)eatTokenSpan();

    if (TYPE_IS_NOT(Type::IDENTIFIER))
        error("this is not a validend module name");
    auto token = eatTokenSpan();
    Module _module_(source.substr(token.value().offset, token.value().size).c_str());
    while (peekTokenSpan(1).has_value())
    {
        if (IS(peekTokenSpan(), Type::IMPORT))
        {
            (void)eatTokenSpan();
            bool is_cpp = false;
            if (IS(peekTokenSpan(), Type::CPP))
            {
                (void)eatTokenSpan();
                is_cpp = true;
            }
            ModuleImport import(is_cpp);
            if (IS(peekTokenSpan(1), Type::DOT))
            {
                while (IS(peekTokenSpan(1), Type::DOT))
                {
                    auto str = extractString();
                    import.path_part.emplace_back(std::move(str));
                    if (IS(peekTokenSpan(), Type::DOT))
                        (void)eatTokenSpan();
                }
                import.path_part.emplace_back(std::move(extractString()));
            }
            else
                import.path_part.emplace_back(std::move(extractString()));
            _module_.statements.emplace_back(std::move(import));
        }
        else if (isOsType(peekTokenSpan().value().type))
        {
            const auto os = eatTokenSpan();
            OsType type = OsType::COMMON;
            switch (os.value().type)
            {
            case Type::WINDOWS:
                type = OsType::WINDOWS;
                break;
            case Type::MACOS:
                type = OsType::MACOS;
                break;
            case Type::LINUX:
                type = OsType::LINUX;
                break;
            case Type::BSD:
                type = OsType::BSD;
                break;
            default:
                break;
            }
            if (const auto block = parse(type); block.has_value())
                _module_.statements.push_back(block.value());
            else
                return {};
        }
        else
        {
            if (const auto block = parse(OsType::COMMON); block.has_value())
                _module_.statements.push_back(block.value());
            else
                return {};
        }
    }
    return _module_;
}
} // namespace LunaLuxLang