#include <Translator.h>
#if IS_MSVC
#    pragma warning(push)
#    pragma warning(disable : 33010)
#    pragma warning(disable : 26451)
#    pragma warning(disable : 26495)
#endif
#include <rapidjson/document.h>
#if IS_MSVC
#    pragma warning(pop)
#endif
#include <libos/Error.h>
#include <libos/FileIO.h>
#include <rapidjson/fwd.h>
#include <map>
#include <string>
#include <sstream>
#include <vector>
namespace LunaLuxLang
{
using namespace rapidjson;
static const inline std::vector<std::wstring> split(const std::wstring &str)
{
    std::vector<std::wstring> result;
    size_t start = 0;
    size_t end = str.find(L'\n');
    while (end != std::wstring::npos)
    {
        result.push_back(str.substr(start, (end - start) + 1));
        end = str.find(L'\n', (start = end + 1));
    }
    result.push_back(str.substr(start, str.length() - start));
    return result;
}

static const inline std::vector<std::wstring> split(const std::wstring &str, const wchar_t *delimiter)
{
    std::vector<std::wstring> result;
    size_t start = 0;
    size_t end = str.find_first_of(delimiter);
    while (end != std::wstring::npos)
    {
        std::wstring view(str.data() + start, end - start);
        if (!view.empty())
            result.push_back(view);
        result.push_back(std::wstring(str.data() + end, 1));
        start = end + 1;
        end = str.find_first_of(delimiter, start);
    }
    std::wstring last(str.data() + start, str.length() - start);
    if (!last.empty())
        result.push_back(last);
    return result;
}

static inline std::wstring replace_words(const std::wstring& old, const std::map<std::wstring, std::wstring>& words)
{
    std::wstring str = old.substr(old.find_first_of(L"\n") + 1);
    std::wstring data;
    const std::vector<std::wstring> splits = split(str,L"\n\"\'(){}[]-=+/*<>.,\\| \t;#");
    //todo replace with words from map
    for(std::wstring _str:splits)
    {
        auto word = words.find(_str);
        if(word != words.end())
            data += word->second;
        else
            data += _str;
    }
    return data;
}

std::wstring Translator::operator()(const std::wstring input) const
{
    std::map<std::wstring, std::wstring> words;
    std::vector<std::wstring> lines = split(input);
    std::wstring line = lines[0];
    std::string path = "$[binary_base]/lang/@.json";
    size_t pos = line.find(' ');
    if (pos != std::wstring::npos)
        line = line.substr(pos + 1, 5);
    char *str;
    losUnicodeToBytes(line.c_str(), &str);
    path.replace(path.find('@'), 1, str);
    delete str;
    losFileHandle handle;
    losFileOpenInfo file;
    file.fileBits = LOS_FILE_BIT_READ | LOS_FILE_BIT_UNICODE;
    file.path = path.c_str();
    file.path_size = path.size();
    if (losOpenFile(&handle, file) != LOS_SUCCESS)
    {
        losPrintError((std::string("LunaLux::LunaScript<Translator>: failed to open file -> ") + path).c_str());
        return input;
    }
    wchar_t *buff;
    size_t buff_size;
    if (losReadFile(handle, (void **)&buff, &buff_size) != LOS_SUCCESS)
    {
        losPrintError((std::string("LunaLuxLang<Translator>: failed to read file -> ") + path).c_str());
        losCloseFile(handle);
        return input;
    }
    if (losCloseFile(handle) != LOS_SUCCESS)
    {
        losPrintError((std::string("LunaLuxLang<Translator>: failed to read file -> ") + path).c_str());
        return input;
    }
    std::wstring json_string(buff, 0, buff_size);
    GenericDocument<UTF16<>> document;
    document.Parse(json_string.c_str());
    if (!document.IsObject())
    {
        losPrintError("LunaLuxLang<Translator>: JSON document is not an object");
        return input;
    }

    for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it)
    {
        if (!it->name.IsString() || !it->value.IsString())
        {
            losPrintError("LunaLuxLang<Translator>: JSON object contains non-string values");
            return input;
        }
        words[it->value.GetString()] = it->name.GetString();
    }
    return replace_words(input, words);
}

} // namespace LunaLuxLang
