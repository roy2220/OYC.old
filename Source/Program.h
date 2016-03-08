#pragma once


#include <list>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>


namespace OYC {

struct Expression;
struct Statement;


struct ArrayLiteral
{
    std::vector<std::unique_ptr<Expression>> elements;
};


struct DictionaryLiteral
{
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> elements;
};


struct FunctionLiteral
{
    std::vector<const std::string *> parameters;
    bool isVariadic = false;
    std::vector<const std::string *> superVariableNames;
    std::vector<std::unique_ptr<Statement>> body;
};


struct ProgramData
{
    std::unordered_set<std::string> strings;
    std::list<ArrayLiteral> arrayLiterals;
    std::list<DictionaryLiteral> dictionaryLiterals;
    std::list<FunctionLiteral> functionLiterals;
};


struct Program
{
    ProgramData data;
    FunctionLiteral main;
};

} // namespace OYC
