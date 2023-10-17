#include <CppGen.h>
#include <Lexer.h>
#include <Parser.h>
#include <TypeChecker.h>
#include <cstring>
#include <libos/Defines.h>
#include <libos/Error.h>
#include <libos/FileIO.h>
using namespace LunaLuxLang;
#undef DEBUG_MODE
#ifdef DEBUG_MODE
const char *ar = "./../test.lls";
const char *args =
    "-I/home/digitech/Desktop/LunaLuxLang/Includes -std=gnu++23 -fno-exceptions -Ofast -march=native -mtune=native";
#endif

// engine entry point
int main(int argc, char *argv[])
{
    /*for(int i  = 0; i < argc; i++)
    {
        losPrintDebug(argv[i]);
    }*/
    // libos abi check
    if (libOSABIVersion() != LOS_ABI_VERSION_1_1)
    {
        losPrintError("libOS ABI Version not LOS_ABI_VERSION_1_1");
        return -1;
    }
    // args check
#ifndef DEBUG_MODE
    if (argc <= 1)
    {
        losPrintError("you can't give no LunaLuxLangCompiler args");
        return EXIT_FAILURE;
    }
    argc = argc - 1;
#else
    argc = 0;
#endif
    int error = 0;
    // libos start
    libOSInit();
    {
        // read source file
        std::string source;
        {
            // we are creating the file io file infomation to open the file
            losFileOpenInfo open_info = {};
            open_info.fileBits = LOS_FILE_BIT_READ;
#ifdef DEBUG_MODE
            open_info.path = ar;
            open_info.path_size = std::strlen(ar);
#else
            open_info.path = argv[1];
            open_info.path_size = std::strlen(argv[1]);
#endif
            // open the source file
            losFileHandle handle;
            if (losOpenFile(&handle, open_info) != LOS_SUCCESS)
                // if we fail to open source file the source file don't waste time doing the next steps
                return EXIT_FAILURE;
            // we need to create a raw pointer to get the file data
            char *buffer;
            size_t buffer_size = 0;
            // read the source file
            if (losReadFile(handle, reinterpret_cast<void **>(&buffer), &buffer_size) != LOS_SUCCESS)
            {
                // if we fail to read source file the source file don't waste time doing the next steps
                losCloseFile(handle);
                return EXIT_FAILURE;
            }
            // we don't need the file anymore so let's close it and move the C lang string to a c++ std::string
            losCloseFile(handle);
            source = std::string(std::move(buffer), std::move(buffer_size));
            // we need to clean our raw pointer
            delete buffer;
        }
        if (auto _module_ = Parser(std::move(source), std::move(Lexer(source)))(); _module_.has_value())
        {
            // let's type check the language to make sure the developer is writing correct code
            if (TypeChecker()(_module_.value()))
            {
                losPrintError("type checker failed");
                return EXIT_FAILURE;
            }
            // temp for test runner
            if (argc <= 2)
            {
                if (std::strcmp(argv[2], "-n_cpp") == 0)
                {
                    libOSCleanUp();
                    return 0;
                }
            }
            // generate cpp from the LunaLuxLang AST
            std::string code = CppGen()(_module_.value());
            {
                // we are creating the file io file infomation to open the file
                // create temp file
                losFileOpenInfo open_info = {};
                open_info.fileBits = LOS_FILE_BIT_WRITE | LOS_FILE_BIT_CREATE;
                std::stringstream stream;
#ifdef DEBUG_MODE
                stream << ar;
#else
                stream << argv[1];
#endif
                stream << ".cpp";
                auto string_ = stream.str();
                open_info.path = string_.c_str();
                open_info.path_size = string_.size();
                // open the temp file
                losFileHandle handle;
                if (losOpenFile(&handle, open_info) != LOS_SUCCESS)
                    return EXIT_FAILURE;
                // write to the temp file
                if (losWriteFile(handle, reinterpret_cast<const void *>(code.c_str()), code.size()) != LOS_SUCCESS)
                {
                    // if we fail to write temp file the temp file don't waste time doing the next steps
                    losCloseFile(handle);
                    return EXIT_FAILURE;
                }
                // we don't need the file anymore so let's close it
                losCloseFile(handle);
            }
            // temp for test runner
            if (argc <= 2)
            {
                if (std::strcmp(argv[2], "-n_cpp_compiler") == 0)
                {
                    libOSCleanUp();
                    return 0;
                }
            }
            // we start building the command to pass the source to c++
            std::stringstream stream;
            stream << "g++ ";
#ifdef DEBUG_MODE
            stream << args;
            stream << " -c ";
#else

            for (int i = 2; i < argc; i++)
            {
                stream << argv[i];
                stream << " ";
            }
            stream << " -c ";
#endif
#ifdef DEBUG_MODE
            stream << ar;
#else
            stream << argv[1];
#endif
            stream << ".cpp";
            stream << " -o ";
#ifdef DEBUG_MODE
            stream << ar;
#else
            stream << argv[1];
#endif
            stream << ".o";
#ifdef DEBUG_MODE
            printf("PASSING TO G++: %s\n", stream.str().c_str());
#endif
            // compile cpp temp file by passing command to the OS
            error = std::system(stream.str().c_str());
            {
                // clean up temp file
                losFileOpenInfo open_info = {};
                open_info.fileBits = LOS_FILE_BIT_DELETE_AFTER_CLOSE;
                std::stringstream stream;
#ifdef DEBUG_MODE
                stream << ar;
#else
                stream << argv[1];
#endif
                stream << ".cpp";
                auto string_ = stream.str();
                open_info.path = string_.c_str();
                open_info.path_size = string_.size();
                losFileHandle handle;
                // let's open it to delete it as it was a temp file
                if (losOpenFile(&handle, open_info) != LOS_SUCCESS)
                    return EXIT_FAILURE;
                // we don't need the file anymore so let's close it
                losCloseFile(handle);
            }
        }
        else
            error = 1;
    }
    // libos cleanup
    libOSCleanUp();
    return error;
}