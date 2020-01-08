#include "StdAfx.h"

void* AllocateCallback(size_t size) { return malloc(size); }
void* ReallocateCallback(void* ptr, size_t size) { return realloc(ptr, size); }
void FreeCallback(void* ptr) { return free(ptr); }

void AssertHandler(const nl::assert::Assert& assert)
{
    auto message = nl::LargeString<>::Format("Expression: {}\nFunction: {}\nFile: {}:{}", assert.Expression, assert.Function, assert.Filename, assert.Line);
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
    MessageBoxA(nullptr, message, "Assert failed", MB_OK | MB_ICONERROR);
}

int main(int, char**)
{
    nl::assert::SetAssertHandler(AssertHandler);
    nl::memory::SetMemoryManagement(AllocateCallback, ReallocateCallback, FreeCallback);

    nl::parsing::Scanner scanner(R"(

    {
        "glossary": {
            "title": "example glossary",
		    "GlossDiv": {
                "title": "S",
			    "GlossList": {
                    "GlossEntry": {
                        "ID": "SGML",
					    "SortAs": "SGML",
					    "GlossTerm": "Standard Generalized Markup Language",
					    "Acronym": "SGML",
					    "Abbrev": "ISO 8879:1986",
					    "GlossDef": {
                            "para": "A meta-markup language, used to create markup languages such as DocBook.",
						    "GlossSeeAlso": ["GML", "XML"]
                        },
					    "GlossSee": "markup"
                    }
                }
            }
        }
    }

    )");

    while (auto token = scanner.Next())
    {
        std::cout << std::setw(11) << std::left << nl::parsing::TokenTypeToString(token) << " " << token << " (Line: " << token.GetLine() << ")" << std::endl;
    }

    return 0;
}