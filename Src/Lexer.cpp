#include <Lexer.h>
#include <algorithm>
#include <array>
#include <regex>
namespace LunaLuxLang
{
static unsigned long current_line = 1;
static const std::regex tokenRegex(
    R"((\"[^\"]*\")|(\/\*[\s\S]*?\*\/)|(\[\[[^\]]*\]\])|([A-Za-z_][A-Za-z_0-9]*)|(-?\d+(\.\d+)?)|([^;^' '^\t])|(\n))");
using namespace std::literals::string_view_literals;
static constexpr std::array<std::pair<std::string_view, Type>, 52> lpt{{{"import"sv, Type::IMPORT},
                                                                        {"cpp"sv, Type::CPP},
                                                                        {"module"sv, Type::MODULE},
                                                                        {"func"sv, Type::FUNC},
                                                                        {"public"sv, Type::PUBLIC_FUNC},
                                                                        {"ret"sv, Type::RET},
                                                                        {":"sv, Type::SIDE_EYES},
                                                                        {"("sv, Type::L_CURLY},
                                                                        {"}"sv, Type::R_SQUIGGLY},
                                                                        {"{"sv, Type::L_SQUIGGLY},
                                                                        {"]"sv, Type::R_BOX},
                                                                        {"["sv, Type::L_BOX},
                                                                        {")"sv, Type::R_CURLY},
                                                                        {"<"sv, Type::L_ARROW},
                                                                        {">"sv, Type::R_ARROW},
                                                                        {","sv, Type::COMMA},
                                                                        {"."sv, Type::DOT},
                                                                        {"#"sv, Type::HASH_TAG},
                                                                        {"+"sv, Type::ADD},
                                                                        {"-"sv, Type::SUB},
                                                                        {"*"sv, Type::MUL},
                                                                        {"/"sv, Type::DIV},
                                                                        {"and"sv, Type::AND},
                                                                        {"or"sv, Type::OR},
                                                                        {"xor"sv, Type::XOR},
                                                                        {"modulo"sv, Type::MODULO},
                                                                        {"="sv, Type::EQUAL},
                                                                        {"global"sv, Type::GLOBAL},
                                                                        {"mut"sv, Type::MUT},
                                                                        {"ptr"sv, Type::PTR},
                                                                        {"ref"sv, Type::REF},
                                                                        {"int8"sv, Type::INT8},
                                                                        {"int16"sv, Type::INT16},
                                                                        {"int32"sv, Type::INT32},
                                                                        {"int64"sv, Type::INT64},
                                                                        {"uint8"sv, Type::UINT8},
                                                                        {"uint16"sv, Type::UINT16},
                                                                        {"uint32"sv, Type::UINT32},
                                                                        {"uint64"sv, Type::UINT64},
                                                                        {"float32"sv, Type::FLOAT32},
                                                                        {"float64"sv, Type::FLOAT64},
                                                                        {"string"sv, Type::STRING},
                                                                        {"vec2"sv, Type::VEC2},
                                                                        {"vec3"sv, Type::VEC3},
                                                                        {"vec4"sv, Type::VEC4},
                                                                        {"quat"sv, Type::QUAT},
                                                                        {"matrix4"sv, Type::MATRIX4},
                                                                        {"void"sv, Type::VOID},
                                                                        {"windows"sv, Type::WINDOWS},
                                                                        {"apple_mac"sv, Type::MACOS},
                                                                        {"linux"sv, Type::LINUX},
                                                                        {"bsd"sv, Type::BSD}}};

struct TypeData
{
    Type type;
    unsigned long line;
};

[[nodiscard]] static constexpr inline const Type lookup(std::string_view k)
{
    if (const auto i = std::find_if(begin(lpt), end(lpt), [&k](const auto &v) { return v.first == k; }); i != end(lpt))
        return i->second;
    return Type::IDENTIFIER;
}

[[nodiscard]] inline TypeData determineType(const std::string_view &src) noexcept
{
    if (src.starts_with('\"') && src.ends_with('\"'))
        return {Type::STRING, current_line};
    char *end;
    std::strtod(src.data(), &end);
    if (end == src.data() + src.length())
    {
        if (src.find('.') != std::string_view::npos)
            return {Type::FLOAT_INT_IDENTIFIER, current_line};
        return {Type::INT_IDENTIFIER, current_line};
    }
    return {lookup(src), current_line};
}

[[nodiscard]] std::vector<Span> Lexer(const std::string &input) noexcept
{
    std::vector<Span> tokens;
    auto regexIter = std::sregex_iterator(input.begin(), input.end(), tokenRegex);
    const auto regexEnd = std::sregex_iterator();
    while (regexIter != regexEnd)
    {
        const auto &match = *regexIter;
        auto token_string = std::string_view(input.data() + match.position(), match.length());
        if (token_string.compare("\n") == 0)
            current_line++;
        else
        {
            if (!token_string.starts_with("/*") && !token_string.ends_with("*/"))
            {
                auto type_data = determineType(token_string);
                tokens.emplace_back(Span{match.position(), match.length(), type_data.line, type_data.type});
            }
        }
        ++regexIter;
    }
    return tokens;
}
} // namespace LunaLuxLang