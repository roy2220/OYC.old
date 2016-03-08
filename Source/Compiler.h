#pragma once


#include <cstdint>

#include "ExpressionVisitor.h"


namespace OYC {

struct Program;
struct Function;

class CompilationContext;


class Compiler final : public ExpressionVisitor
{
    Compiler(const Compiler &) = delete;
    Compiler &operator=(const Compiler &) = delete;

public:
    inline explicit Compiler();

    Function generateFunction(const Program &);

private:
    CompilationContext *context_;
};

} // namespace OYC
