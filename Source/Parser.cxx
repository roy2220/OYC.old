#include "Parser.h"

#include <cctype>
#include <cstdlib>
#include <iterator>

#include "Error.h"
#include "Expression.h"
#include "Program.h"
#include "ScopeGuard.h"
#include "Statement.h"


namespace OYC {

#define VARIABLE_NAME_CLEANUP                                     \
    [this, n = context_->getNumberOfVariableNames()] () -> void { \
        context_->deleteVariableNames(n);                         \
    }


class ParseContext final
{
    ParseContext(const ParseContext &) = delete;
    ParseContext &operator=(const ParseContext &) = delete;

public:
    explicit ParseContext(ParseContext *, FunctionLiteral *);

    int getNumberOfVariableNames() const;
    void addVariableName(const std::string *);
    void deleteVariableNames(int);
    const std::string *searchVariableName(const std::string &);

private:
    ParseContext *const super_;
    FunctionLiteral *const functionLiteral_;
    std::vector<const std::string *> variableNames_;
};


namespace {

void SetStatementPosition(Statement *, const Token &);
void ExpectToken(const Token &, TokenType);
void ExpectToken(const Token &, TokenType, TokenType);
void EvaluateStringLiteral(const std::string &, std::string *);
int UnescapeChar(const char **);

bool isodigit(int);
int odigit2int(char);
int xdigit2int(char);

} // namespace


Program
Parser::readProgram()
{
    Program program;
    programData_ = &program.data;
    matchProgramMain(&program.main);
    return program;
}


Token
Parser::doReadToken()
{
    Token token = input_();

    while (token.type == TokenType::WhiteSpace || token.type == TokenType::Comment) {
        token = input_();
    }

    return token;
}


const Token &
Parser::peekToken(int position)
{
    for (int n = position - static_cast<int>(prereadTokens_.size()); n >= 1; --n) {
        prereadTokens_.push_back(doReadToken());
    }

    return *std::next(prereadTokens_.begin(), position - 1);
}


Token
Parser::readToken()
{
    if (prereadTokens_.empty()) {
        return doReadToken();
    } else {
        Token token = std::move(prereadTokens_.front());
        prereadTokens_.pop_front();
        return token;
    }
}


void
Parser::matchProgramMain(FunctionLiteral *match)
{
    ParseContext context(nullptr, match);
    context_ = &context;
    match->isVariadic = true;
    matchStatements(&match->body, TokenType::EndOfFile);
    return;
}


void
Parser::matchStatements(std::vector<std::unique_ptr<Statement>> *match, TokenType terminator)
{
    if (terminator == TokenType::No) {
        std::unique_ptr<Statement> statement = matchStatement();

        if (statement != nullptr) {
            match->push_back(std::move(statement));
        }

        return;
    } else {
        for (const Token *token = &peekToken(1); token->type != terminator
             ; token = &peekToken(1)) {
            std::unique_ptr<Statement> statement = matchStatement();

            if (statement != nullptr) {
                match->push_back(std::move(statement));
            }
        }

        readToken();
        return;
    }
}


std::unique_ptr<Statement>
Parser::matchStatement()
{
    const Token *token = &peekToken(1);

    switch (token->type) {
    case MakeTokenType(';'):
        readToken();
        return nullptr;

    case TokenType::AutoKeyword:
        return matchAutoStatement();

    case TokenType::BreakKeyword:
        return matchBreakStatement();

    case TokenType::ContinueKeyword:
        return matchContinueStatement();

    case TokenType::ReturnKeyword:
        return matchReturnStatement();

    case TokenType::IfKeyword:
        return matchIfStatement();

    case TokenType::SwitchKeyword:
        return matchSwitchStatement();

    case TokenType::WhileKeyword:
        return matchWhileStatement();

    case TokenType::DoKeyword:
        return matchDoWhileStatement();

    case TokenType::ForKeyword:
        return matchForStatement();

    case TokenType::ForeachKeyword:
        return matchForeachStatement();

    default:
        return matchExpressionStatement();
    }
}


std::unique_ptr<Statement>
Parser::matchExpressionStatement()
{
    auto match = std::make_unique<ExpressionStatement>();
    SetStatementPosition(match.get(), peekToken(1));
    match->expression = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(';'));
    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchAutoStatement()
{
    auto match = std::make_unique<AutoStatement>();
    SetStatementPosition(match.get(), readToken());

    for (;;) {
        match->variableDeclarators.emplace_back();
        matchVariableDeclarator(&match->variableDeclarators.back());
        const Token *token = &peekToken(1);
        ExpectToken(*token, MakeTokenType(','), MakeTokenType(';'));

        if (token->type == MakeTokenType(',')) {
            readToken();
        } else {
            break;
        }
    }

    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchBreakStatement()
{
    auto match = std::make_unique<BreakStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType(';'));
    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchContinueStatement()
{
    auto match = std::make_unique<BreakStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType(';'));
    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchReturnStatement()
{
    auto match = std::make_unique<ReturnStatement>();
    SetStatementPosition(match.get(), readToken());
    const Token *token = &peekToken(1);

    if (token->type != MakeTokenType(';')) {
        match->result = matchExpression1();
        ExpectToken(peekToken(1), MakeTokenType(';'));
    }

    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchIfStatement()
{
    ScopeGuard scopeGuard(VARIABLE_NAME_CLEANUP);
    scopeGuard.commit();
    auto match = std::make_unique<IfStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    match->condition = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    matchBlock(&match->thenBody);
    const Token *token = &peekToken(1);

    if (token->type == TokenType::ElseKeyword) {
        readToken();
        matchBlock(&match->elseBody);
    }

    return match;
}


std::unique_ptr<Statement>
Parser::matchSwitchStatement()
{
    auto match = std::make_unique<SwitchStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    match->lhs = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    ExpectToken(peekToken(1), MakeTokenType('{'));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type != MakeTokenType('}')) {
        ExpectToken(*token, TokenType::CaseKeyword, TokenType::DefaultKeyword);
        bool defaultLabelFlag = token->type == TokenType::DefaultKeyword;

        for (;;) {
            match->caseClauses.emplace_back();
            matchCaseClause(&match->caseClauses.back());
            token = &peekToken(1);

            if (token->type == MakeTokenType('}')) {
                break;
            } else {
                if (token->type == TokenType::DefaultKeyword) {
                    if (defaultLabelFlag) {
                        throw Error::DuplicateDefaultLabel(*token);
                    }

                    defaultLabelFlag = true;
                }
            }
        }
    }

    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchWhileStatement()
{
    ScopeGuard scopeGuard(VARIABLE_NAME_CLEANUP);
    scopeGuard.commit();
    auto match = std::make_unique<WhileStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    match->condition = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    matchBlock(&match->body);
    return match;
}


std::unique_ptr<Statement>
Parser::matchDoWhileStatement()
{
    ScopeGuard scopeGuard(VARIABLE_NAME_CLEANUP);
    scopeGuard.commit();
    auto match = std::make_unique<DoWhileStatement>();
    readToken();
    matchBlock(&match->body);
    ExpectToken(peekToken(1), TokenType::WhileKeyword);
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    match->condition = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    ExpectToken(peekToken(1), MakeTokenType(';'));
    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchForStatement()
{
    ScopeGuard scopeGuard(VARIABLE_NAME_CLEANUP);
    scopeGuard.commit();
    auto match = std::make_unique<ForStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType(';')) {
        readToken();
    } else {
        ExpectToken(*token, TokenType::AutoKeyword);
        match->initialization = matchAutoStatement();
    }

    token = &peekToken(1);

    if (token->type != MakeTokenType(';')) {
        match->condition = matchExpression1();
        ExpectToken(peekToken(1), MakeTokenType(';'));
    }

    readToken();
    token = &peekToken(1);

    if (token->type != MakeTokenType(')')) {
        match->iteration = matchExpression1();
        ExpectToken(peekToken(1), MakeTokenType(')'));
    }

    readToken();
    matchBlock(&match->body);
    return match;
}


std::unique_ptr<Statement>
Parser::matchForeachStatement()
{
    ScopeGuard scopeGuard(VARIABLE_NAME_CLEANUP);
    scopeGuard.commit();
    auto match = std::make_unique<ForeachStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    ExpectToken(peekToken(1), TokenType::AutoKeyword);
    readToken();
    match->variableName1 = getVariableName();
    ExpectToken(peekToken(1), MakeTokenType(','));
    readToken();
    match->variableName2 = getVariableName();
    ExpectToken(peekToken(1), MakeTokenType(':'));
    readToken();
    match->collection = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    matchBlock(&match->body);
    return match;
}


void
Parser::matchVariableDeclarator(VariableDeclarator *match)
{
    match->name = getVariableName();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('=')) {
        readToken();
        match->initializer = matchExpression2();
    }

    return;
}


void
Parser::matchBlock(std::vector<std::unique_ptr<Statement>> *match)
{
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchStatements(match, MakeTokenType('}'));
        return;
    } else {
        matchStatements(match, TokenType::No);
        return;
    }
}


void
Parser::matchCaseClause(CaseClause *match)
{
    ScopeGuard scopeGuard(VARIABLE_NAME_CLEANUP);
    scopeGuard.commit();
    const Token *token = &peekToken(1);

    if (token->type == TokenType::CaseKeyword) {
        readToken();
        match->rhs = matchExpression1();
    } else {
        readToken();
    }

    ExpectToken(peekToken(1), MakeTokenType(':'));
    readToken();
    token = &peekToken(1);

    while (token->type != TokenType::CaseKeyword && token->type != TokenType::DefaultKeyword
           && token->type != MakeTokenType('}')) {
        std::unique_ptr<Statement> statement = matchStatement();

        if (statement != nullptr) {
            match->body.push_back(std::move(statement));
        }

        token = &peekToken(1);
    }

    return;
}


std::unique_ptr<Expression>
Parser::matchExpression1()
{
    std::unique_ptr<Expression> result = matchExpression2();
    const Token *token = &peekToken(1);

    while (token->type == MakeTokenType(',')) {
        auto match = std::make_unique<BinaryExpression>();
        match->operand1 = std::move(result);
        match->op = readToken().type;
        match->operand2 = matchExpression2();
        result = std::move(match);
        token = &peekToken(1);
    }

    return result;
}


std::unique_ptr<Expression>
Parser::matchExpression2()
{
    int precedence = 0;
    std::unique_ptr<Expression> result = matchExpression3(&precedence);
    const Token *token = &peekToken(1);

    switch (token->type) {
    case MakeTokenType('?'): {
            auto match = std::make_unique<TernaryExpression>();
            match->operand1 = std::move(result);
            match->op[0] = readToken().type;
            match->operand2 = matchExpression2();
            ExpectToken(peekToken(1), MakeTokenType(':'));
            match->op[1] = readToken().type;
            match->operand3 = matchExpression2();
            return match;
        }

    case MakeTokenType('='):
    case MakeTokenType('|', '='):
    case MakeTokenType('^', '='):
    case MakeTokenType('&', '='):
    case MakeTokenType('<', '<', '='):
    case MakeTokenType('>', '>', '='):
    case MakeTokenType('+', '='):
    case MakeTokenType('-', '='):
    case MakeTokenType('*', '='):
    case MakeTokenType('/', '='):
    case MakeTokenType('%', '='): {
            auto match = std::make_unique<BinaryExpression>();
            match->operand1 = std::move(result);
            match->op = readToken().type;
            match->operand2 = matchExpression2();
            return match;
        }

    default:
        return result;
    }
}


std::unique_ptr<Expression>
Parser::matchExpression3(int *precedence)
{
    int lowestPrecedence = *precedence + 1;
    std::unique_ptr<Expression> result = matchExpression4();
    const Token *token = &peekToken(1);

    switch (token->type) {
    case MakeTokenType('|', '|'):
        *precedence = 1;
        break;

    case MakeTokenType('&', '&'):
        *precedence = 2;
        break;

    case MakeTokenType('|'):
        *precedence = 3;
        break;

    case MakeTokenType('^'):
        *precedence = 4;
        break;

    case MakeTokenType('&'):
        *precedence = 5;
        break;

    case MakeTokenType('=', '='):
    case MakeTokenType('!', '='):
        *precedence = 6;
        break;

    case MakeTokenType('<'):
    case MakeTokenType('<', '='):
    case MakeTokenType('>', '='):
    case MakeTokenType('>'):
        *precedence = 7;
        break;

    case MakeTokenType('<', '<'):
    case MakeTokenType('>', '>'):
        *precedence = 8;
        break;

    case MakeTokenType('+'):
    case MakeTokenType('-'):
        *precedence = 9;
        break;

    case MakeTokenType('*'):
    case MakeTokenType('/'):
    case MakeTokenType('%'):
        *precedence = 10;
        break;

    default:
        *precedence = 0;
    }

    while (*precedence >= lowestPrecedence) {
        auto match = std::make_unique<BinaryExpression>();
        match->operand1 = std::move(result);
        match->op = readToken().type;
        match->operand2 = matchExpression3(precedence);
        result = std::move(match);
        token = &peekToken(1);
    }

    return result;
}


std::unique_ptr<Expression>
Parser::matchExpression4()
{
    const Token *token = &peekToken(1);

    switch (token->type) {
    case MakeTokenType('('):
        token = &peekToken(2);

        switch (token->type) {
        case TokenType::BoolKeyword:
        case TokenType::IntKeyword:
        case TokenType::FloatKeyword:
        case TokenType::StrKeyword: {
                auto match = std::make_unique<UnaryExpression>();
                match->type = UnaryExpressionType::Prefix;
                readToken();
                match->op = readToken().type;
                ExpectToken(peekToken(1), MakeTokenType(')'));
                readToken();
                match->operand = matchExpression4();
                return match;
            }

        default:
            return matchExpression5();
        }

    case MakeTokenType('+', '+'):
    case MakeTokenType('-', '-'):
    case MakeTokenType('+'):
    case MakeTokenType('-'):
    case MakeTokenType('!'):
    case MakeTokenType('~'):
    case TokenType::SizeofKeyword: {
            auto match = std::make_unique<UnaryExpression>();
            match->type = UnaryExpressionType::Prefix;
            match->op = readToken().type;
            match->operand = matchExpression4();
            return match;
        }

    default:
        return matchExpression5();
    }
}


std::unique_ptr<Expression>
Parser::matchExpression5()
{
    std::unique_ptr<Expression> result = matchExpression6();
    const Token *token = &peekToken(1);

    for (;;) {
        switch (token->type) {
        case MakeTokenType('+', '+'):
        case MakeTokenType('-', '-'): {
                auto match = std::make_unique<UnaryExpression>();
                match->type = UnaryExpressionType::Postfix;
                match->op = readToken().type;
                match->operand = std::move(result);
                result = std::move(match);
                token = &peekToken(1);
                break;
            }

        case MakeTokenType('.'):
        case MakeTokenType('['): {
                auto match = std::make_unique<RetrievalExpression>();
                match->retrievee = std::move(result);
                match->key = matchElementSelector();
                result = std::move(match);
                token = &peekToken(1);
                break;
            }

        case MakeTokenType('('): {
                auto match = std::make_unique<InvocationExpression>();
                match->invokee = std::move(result);
                readToken();
                token = &peekToken(1);

                if (token->type != MakeTokenType(')')) {
                    for (;;) {
                        match->arguments.push_back(matchArrayElement());
                        token = &peekToken(1);
                        ExpectToken(*token, MakeTokenType(','), MakeTokenType(')'));

                        if (token->type == MakeTokenType(',')) {
                            readToken();
                        } else {
                            break;
                        }
                    }
                }

                readToken();
                result = std::move(match);
                token = &peekToken(1);
                break;
            }

        default:
            return result;
        }
    }
}


std::unique_ptr<Expression>
Parser::matchExpression6()
{
    const Token *token = &peekToken(1);

    switch (token->type) {
    case MakeTokenType('('): {
            readToken();
            std::unique_ptr<Expression> result = matchExpression1();
            ExpectToken(peekToken(1), MakeTokenType(')'));
            readToken();
            return result;
        }

    case TokenType::NullKeyword: {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::Null;
            readToken();
            return match;
        }

    case TokenType::FalseKeyword:
    case TokenType::TrueKeyword: {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::Boolean;
            match->boolean = getBoolean();
            return match;
        }

    case TokenType::IntegerLiteral: {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::Integer;
            match->integer = getInteger();
            return match;
        }

    case TokenType::FloatingPointLiteral: {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::FloatingPoint;
            match->floatingPoint = getFloatingPoint();
            return match;
        }

    case TokenType::StringLiteral: {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::String;
            match->string = getString();
            return match;
        }

    case TokenType::Identifier: {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::VariableName;
            match->string = findVariableName();
            return match;
        }

    case MakeTokenType('{'): {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::ArrayLiteral;
            match->arrayLiteral = matchArrayLiteral();
            return match;
        }

    case TokenType::DictKeyword: {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::DictionaryLiteral;
            match->dictionaryLiteral = matchDictionaryLiteral();
            return match;
        }

    case TokenType::FuncKeyword: {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::FunctionLiteral;
            match->functionLiteral = matchFunctionLiteral();
            return match;
        }

    case TokenType::ThisKeyword: {
            auto match = std::make_unique<PrimaryExpression>();
            match->type = PrimaryExpressionType::This;
            readToken();
            return match;
        }

    default:
        throw Error::UnexpectedToken(*token, "primary-expression");
    }
}


std::unique_ptr<Expression>
Parser::matchElementSelector()
{
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('.')) {
        readToken();
        auto key = std::make_unique<PrimaryExpression>();
        key->type = PrimaryExpressionType::String;
        ExpectToken(peekToken(1), TokenType::Identifier);
        key->string = getIdentifier();
        return key;
    } else {
        readToken();
        std::unique_ptr<Expression> key = matchExpression1();
        ExpectToken(peekToken(1), MakeTokenType(']'));
        readToken();
        return key;
    }
}


bool
Parser::getBoolean()
{
    return readToken().type == TokenType::TrueKeyword;
}


unsigned long
Parser::getInteger()
{
    return std::strtoul(readToken().value.c_str(), nullptr, 0);
}


double
Parser::getFloatingPoint()
{
    return std::strtod(readToken().value.c_str(), nullptr);
}


const std::string *
Parser::getString()
{
    std::string string;
    EvaluateStringLiteral(readToken().value, &string);
    const Token *token = &peekToken(1);

    while (token->type == TokenType::StringLiteral) {
        EvaluateStringLiteral(readToken().value, &string);
        token = &peekToken(1);
    }

    std::pair<std::unordered_set<std::string>::iterator
              , bool> result = programData_->strings.insert(std::move(string));
    return &*result.first;
}


const std::string *
Parser::getIdentifier()
{
    std::pair<std::unordered_set<std::string>::iterator
              , bool> result = programData_->strings.insert(readToken().value);
    return &*result.first;
}


const ArrayLiteral *
Parser::matchArrayLiteral()
{
    programData_->arrayLiterals.emplace_back();
    ArrayLiteral *match = &programData_->arrayLiterals.back();
    readToken();
    const Token *token = &peekToken(1);

    if (token->type != MakeTokenType('}')) {
        for (;;) {
            match->elements.push_back(matchArrayElement());
            token = &peekToken(1);
            ExpectToken(*token, MakeTokenType(','), MakeTokenType('}'));

            if (token->type == MakeTokenType(',')) {
                readToken();
                token = &peekToken(1);

                if (token->type == MakeTokenType('}')) {
                    break;
                }
            } else {
                break;
            }
        }
    }

    readToken();
    return match;
}


const DictionaryLiteral *
Parser::matchDictionaryLiteral()
{
    programData_->dictionaryLiterals.emplace_back();
    DictionaryLiteral *match = &programData_->dictionaryLiterals.back();
    readToken();
    ExpectToken(peekToken(1), MakeTokenType('{'));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type != MakeTokenType('}')) {
        for (;;) {
            match->elements.push_back(matchDictionaryElement());
            token = &peekToken(1);
            ExpectToken(*token, MakeTokenType(','), MakeTokenType('}'));

            if (token->type == MakeTokenType(',')) {
                readToken();
                token = &peekToken(1);

                if (token->type == MakeTokenType('}')) {
                    break;
                }
            } else {
                break;
            }
        }
    }

    readToken();
    return match;
}


const FunctionLiteral *
Parser::matchFunctionLiteral()
{
    ScopeGuard scopeGuard([this, c = context_] () -> void {
        context_ = c;
    });

    programData_->functionLiterals.emplace_back();
    FunctionLiteral *match = &programData_->functionLiterals.back();
    ParseContext context(context_, match);
    context_ = &context;
    scopeGuard.commit();
    readToken();
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type != MakeTokenType(')')) {
        for (;;) {
            ExpectToken(*token, TokenType::AutoKeyword, MakeTokenType('.', '.', '.'));

            if (token->type == TokenType::AutoKeyword) {
                readToken();
                match->parameters.push_back(getVariableName());
                token = &peekToken(1);
                ExpectToken(*token, MakeTokenType(','), MakeTokenType(')'));

                if (token->type == MakeTokenType(',')) {
                    readToken();
                    token = &peekToken(1);
                } else {
                    break;
                }
            } else {
                match->isVariadic = true;
                readToken();
                ExpectToken(peekToken(1), MakeTokenType(')'));
                break;
            }
        }
    }

    readToken();
    ExpectToken(peekToken(1), MakeTokenType('{'));
    readToken();
    matchStatements(&match->body, MakeTokenType('}'));
    return match;
}


std::unique_ptr<Expression>
Parser::matchArrayElement()
{
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('.', '.', '.')) {
        auto match = std::make_unique<PrimaryExpression>();
        match->type = PrimaryExpressionType::Varargs;
        readToken();
        return match;
    } else {
        return matchExpression2();
    }
}


std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>
Parser::matchDictionaryElement()
{
    ExpectToken(peekToken(1), MakeTokenType('.'), MakeTokenType('['));
    std::unique_ptr<Expression> key = matchElementSelector();
    ExpectToken(peekToken(1), MakeTokenType('='));
    readToken();
    return std::make_pair(std::move(key), matchExpression2());
}


const std::string *
Parser::getVariableName()
{
    ExpectToken(peekToken(1), TokenType::Identifier);
    const std::string *variableName = getIdentifier();
    context_->addVariableName(variableName);
    return variableName;
}


const std::string *
Parser::findVariableName()
{
    Token token = readToken();
    const std::string *variableName = context_->searchVariableName(token.value);

    if (variableName == nullptr) {
        throw Error::UndeclaredVariable(token);
    }

    return variableName;
}


ParseContext::ParseContext(ParseContext *super, FunctionLiteral *functionLiteral)
  : super_(super),
    functionLiteral_(functionLiteral)
{
}


int
ParseContext::getNumberOfVariableNames() const
{
    return static_cast<int>(variableNames_.size());
}


void
ParseContext::addVariableName(const std::string *variableName)
{
    variableNames_.push_back(variableName);
    return;
}


void
ParseContext::deleteVariableNames(int numberOfVariableNames)
{
    variableNames_.erase(variableNames_.begin() + numberOfVariableNames, variableNames_.end());
    return;
}


const std::string *
ParseContext::searchVariableName(const std::string &variableName)
{
    for (const std::string *x : variableNames_) {
        if (*x == variableName) {
            return x;
        }
    }

    if (super_ == nullptr) {
        return nullptr;
    } else {
        const std::string *result = super_->searchVariableName(variableName);

        if (result != nullptr) {
            functionLiteral_->superVariableNames.push_back(result);
            variableNames_.push_back(result);
        }

        return result;
    }
}


namespace {

void
SetStatementPosition(Statement *statement, const Token &token)
{
    statement->lineNumber = token.lineNumber;
    statement->columnNumber = token.columnNumber;
    return;
}


void
ExpectToken(const Token &token, TokenType tokenType)
{
    if (token.type != tokenType) {
        throw Error::UnexpectedToken(token, tokenType);
    }

    return;
}


void
ExpectToken(const Token &token, TokenType tokenType1, TokenType tokenType2)
{
    if (token.type != tokenType1 && token.type != tokenType2) {
        throw Error::UnexpectedToken(token, tokenType1, tokenType2);
    }

    return;
}


void
EvaluateStringLiteral(const std::string &stringLiteral, std::string *string)
{
    const char *p = stringLiteral.data() + 1;
    unsigned char c = *p;

    for (;;) {
        if (c == '\"') {
            return;
        } else {
            *string += c == '\\' ? UnescapeChar(&p) : *p++;
            c = *p;
        }
    }
}


int
UnescapeChar(const char **escapeChar)
{
    const char *&p = *escapeChar;
    ++p;
    unsigned char c = *p;

    switch (c) {
        int ascii;

    case '\"':
    case '\'':
    case '\?':
    case '\\':
        return *p++;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        ascii = odigit2int(*p++);
        c = *p;

        if (isodigit(c)) {
            ascii = ascii << 3 | odigit2int(*p++);
            c = *p;

            if (isodigit(c)) {
                ascii = ascii << 3 | odigit2int(*p++);
            }
        }

        return ascii;

    case 'a':
        ++p;
        return '\a';

    case 'b':
        ++p;
        return '\b';

    case 'f':
        ++p;
        return '\f';

    case 'n':
        ++p;
        return '\n';

    case 'r':
        ++p;
        return '\r';

    case 't':
        ++p;
        return '\t';

    case 'v':
        ++p;
        return '\v';

    case 'x':
        ++p;
        ascii = xdigit2int(*p++);
        c = *p;

        if (std::isxdigit(c)) {
            ascii = ascii << 4 | xdigit2int(*p++);
        }

        return ascii;
    }
}


bool
isodigit(int c)
{
    return c >= '0' && c <= '7';
}


int
odigit2int(char c)
{
    return c - '0';
}


int
xdigit2int(char c)
{
    if (c >= 'a') {
        return 10 + (c - 'a');
    } else if (c >= 'A') {
        return 10 + (c - 'A');
    } else {
        return c - '0';
    }
}

} // namespace


#undef VARIABLE_NAME_CLEANUP

} // namespace OYC
