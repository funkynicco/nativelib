#include "StdAfx.h"

#include <NativeLib/SystemLayer/SystemLayer.h>

#pragma comment(lib, "DbgHelp.lib")

//#error Rewrite this project to be a Unit Test instead / how to generate output? console write?

void AssertHandler(const nl::assert::Assert& assert)
{
    auto message = nl::String::Format("Expression: {}\nFunction: {}\nFile: {}:{}", assert.Expression, assert.Function, assert.Filename, assert.Line);
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
    MessageBoxA(nullptr, message, "Assert failed", MB_OK | MB_ICONERROR);
}

bool SetupNativeLibSystemLayer()
{
    nl::systemlayer::SystemLayerFunctions functions = {};
    if (!nl::systemlayer::GetDefaultSystemLayerFunctions(&functions))
        return false;
    
    functions.AssertHandler = AssertHandler;
    
    nl::systemlayer::SetSystemLayerFunctions(&functions);
    return true;
}

int main(int, char**)
{
    if (!SetupNativeLibSystemLayer())
    {
        std::cout << "Setup NativeLib system layer failed!" << std::endl;
        return 1;
    }

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

    //auto xx = scanner.Peek();

    nl::Vector<nl::parsing::Token> tokens;
    while (auto token = scanner.Next())
    {
        tokens.Add(std::move(token));
    }

    for (const auto& token : tokens)
    {
        std::cout << std::setw(11) << std::left << nl::parsing::TokenTypeToString(token) << " " << token << " (Line: " << token.GetLine() << ")" << std::endl;
    }

    return 0;
}