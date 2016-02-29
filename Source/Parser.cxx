#include "Parser.h"

#include <cctype>
#include <cstdlib>
#include <iterator>

#include "Expression.h"
#include "Program.h"
#include "Statement.h"
#include "SyntaxError.h"


namespace OYC {

namespace {

void SetStatementPosition(Statement *, const Token &);
void ExpectToken(const Token &, TokenType);
void ExpectToken(const Token &, TokenType, TokenType);
std::string EvaluateStringLiteral(const std::string &);

bool isodigit(int);
int odigit2int(char);
int xdigit2int(char);

} // namespace


Program
Parser::readProgram()
{
    Program program;
    matchProgram(&program);
    return program;
}


Token
Parser::doReadToken()
{
    Token token = tokenReader_();

    while (token.type == TokenType::WhiteSpace || token.type == TokenType::Comment) {
        token = tokenReader_();
    }

    if (token.type == TokenType::Illegal) {
        throw SyntaxError::IllegalToken(token);
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
Parser::matchProgram(Program *match)
{
    programData_ = &match->data;
    matchStatements(TokenType::EndOfFile, &match->body);
    programData_ = nullptr;
}


void
Parser::matchStatements(TokenType terminator, std::vector<std::unique_ptr<Statement>> *match)
{
    if (terminator == TokenType::No) {
        std::unique_ptr<Statement> statement = matchStatement();

        if (statement != nullptr) {
            match->push_back(std::move(statement));
        }
    } else {
        for (const Token *token = &peekToken(1); token->type != terminator
             ; token = &peekToken(1)) {
            std::unique_ptr<Statement> statement = matchStatement();

            if (statement != nullptr) {
                match->push_back(std::move(statement));
            }
        }

        readToken();
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
    match->variableDeclarators.emplace_back();
    matchVariableDeclarator(&match->variableDeclarators.back());
    const Token *token = &peekToken(1);

    while (token->type == MakeTokenType(',')) {
        readToken();
        match->variableDeclarators.emplace_back();
        matchVariableDeclarator(&match->variableDeclarators.back());
        token = &peekToken(1);
    }

    ExpectToken(*token, MakeTokenType(';'));
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
    auto match = std::make_unique<IfStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    match->condition = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchStatements(MakeTokenType('}'), &match->thenBody);
    } else {
        matchStatements(TokenType::No, &match->thenBody);
    }

    token = &peekToken(1);

    if (token->type == TokenType::ElseKeyword) {
        readToken();
        token = &peekToken(1);

        if (token->type == MakeTokenType('{')) {
            readToken();
            matchStatements(MakeTokenType('}'), &match->elseBody);
        } else {
            matchStatements(TokenType::No, &match->elseBody);
        }
    }

    return match;
}


std::unique_ptr<Statement>
Parser::matchSwitchStatement()
{
}


std::unique_ptr<Statement>
Parser::matchWhileStatement()
{
    auto match = std::make_unique<WhileStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    match->condition = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchStatements(MakeTokenType('}'), &match->body);
    } else {
        matchStatements(TokenType::No, &match->body);
    }

    return match;
}


std::unique_ptr<Statement>
Parser::matchDoWhileStatement()
{
    auto match = std::make_unique<DoWhileStatement>();
    readToken();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchStatements(MakeTokenType('}'), &match->body);
    } else {
        matchStatements(TokenType::No, &match->body);
    }

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
    auto match = std::make_unique<ForStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    const Token *token = &peekToken(1);
    ExpectToken(*token, TokenType::AutoKeyword, MakeTokenType(';'));

    if (token->type == TokenType::AutoKeyword) {
        match->initialization = matchAutoStatement();
    } else {
        readToken();
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
    token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchStatements(MakeTokenType('}'), &match->body);
    } else {
        matchStatements(TokenType::No, &match->body);
    }

    return match;
}


std::unique_ptr<Statement>
Parser::matchForeachStatement()
{
    auto match = std::make_unique<ForeachStatement>();
    SetStatementPosition(match.get(), readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    ExpectToken(peekToken(1), TokenType::AutoKeyword);
    readToken();
    ExpectToken(peekToken(1), TokenType::Identifier);
    match->variableName1 = getIdentifier();
    ExpectToken(peekToken(1), MakeTokenType(','));
    readToken();
    ExpectToken(peekToken(1), TokenType::Identifier);
    match->variableName2 = getIdentifier();
    ExpectToken(peekToken(1), MakeTokenType(':'));
    readToken();
    match->collection = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchStatements(MakeTokenType('}'), &match->body);
    } else {
        matchStatements(TokenType::No, &match->body);
    }

    return match;
}


void
Parser::matchVariableDeclarator(VariableDeclarator *match)
{
    ExpectToken(peekToken(1), TokenType::Identifier);
    match->name = getIdentifier();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('=')) {
        readToken();
        match->initializer = matchExpression2();
    }
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
            result = std::move(match);
            break;
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
            result = std::move(match);
            break;
        }
    }

    return result;
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

        case MakeTokenType('.'): {
                auto match = std::make_unique<RetrievalExpression>();
                match->retrievee = std::move(result);
                match->key = matchExpression7();
                result = std::move(match);
                token = &peekToken(1);
                break;
            }

        case MakeTokenType('['): {
                auto match = std::make_unique<RetrievalExpression>();
                match->retrievee = std::move(result);
                readToken();
                match->key = matchExpression1();
                ExpectToken(peekToken(1), MakeTokenType(']'));
                readToken();
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
                    match->arguments.push_back(matchExpression2());
                    token = &peekToken(1);

                    while (token->type == MakeTokenType(',')) {
                        readToken();
                        match->arguments.push_back(matchExpression2());
                        token = &peekToken(1);
                    }

                    ExpectToken(*token, MakeTokenType(')'));
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
            match->type = PrimaryExpressionType::Identifier;
            match->identifier = getIdentifier();
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

    default:
        throw SyntaxError::UnexpectedToken(*token);
    }
}


std::unique_ptr<Expression>
Parser::matchExpression7()
{
    auto match = std::make_unique<PrimaryExpression>();
    match->type = PrimaryExpressionType::String;
    readToken();
    ExpectToken(peekToken(1), TokenType::Identifier);
    match->string = getIdentifier();
    return match;
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
    std::string string = EvaluateStringLiteral(readToken().value);
    const Token *token = &peekToken(1);

    while (token->type == TokenType::StringLiteral) {
        string += EvaluateStringLiteral(readToken().value);
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
              , bool> result = programData_->identifiers.insert(readToken().value);
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
            match->elements.push_back(matchExpression2());
            token = &peekToken(1);

            if (token->type == MakeTokenType(',')) {
                readToken();
                token = &peekToken(1);

                if (token->type == MakeTokenType('}')) {
                    break;
                }
            } else {
                ExpectToken(*token, MakeTokenType('}'));
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
            ExpectToken(*token, MakeTokenType('.'), MakeTokenType('['));

            if (token->type == MakeTokenType('.')) {
                std::unique_ptr<Expression> first = matchExpression7();
                ExpectToken(peekToken(1), MakeTokenType('='));
                readToken();
                match->elements.emplace_back(std::move(first), matchExpression2());
            } else {
                readToken();
                std::unique_ptr<Expression> first = matchExpression1();
                ExpectToken(peekToken(1), MakeTokenType(']'));
                readToken();
                ExpectToken(peekToken(1), MakeTokenType('='));
                readToken();
                match->elements.emplace_back(std::move(first), matchExpression2());
            }

            token = &peekToken(1);

            if (token->type == MakeTokenType(',')) {
                readToken();
                token = &peekToken(1);

                if (token->type == MakeTokenType('}')) {
                    break;
                }
            } else {
                ExpectToken(*token, MakeTokenType('}'));
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
    programData_->functionLiterals.emplace_back();
    FunctionLiteral *match = &programData_->functionLiterals.back();
    readToken();
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type != MakeTokenType(')')) {
        bool variadicFlag = false;
        ExpectToken(*token, TokenType::AutoKeyword, MakeTokenType('.', '.', '.'));

        if (token->type == TokenType::AutoKeyword) {
            readToken();
            ExpectToken(peekToken(1), TokenType::Identifier);
            match->parameters.push_back(getIdentifier());
        } else {
            variadicFlag = true;
            readToken();
        }

        token = &peekToken(1);

        while (!variadicFlag && token->type == MakeTokenType(',')) {
            readToken();
            token = &peekToken(1);
            ExpectToken(*token, TokenType::AutoKeyword, MakeTokenType('.', '.', '.'));

            if (token->type == TokenType::AutoKeyword) {
                readToken();
                ExpectToken(peekToken(1), TokenType::Identifier);
                match->parameters.push_back(getIdentifier());
            } else {
                variadicFlag = true;
                readToken();
            }

            token = &peekToken(1);
        }

        ExpectToken(*token, MakeTokenType(')'));
        match->isVariadic = variadicFlag;
    }

    readToken();
    ExpectToken(peekToken(1), MakeTokenType('{'));
    readToken();
    matchStatements(MakeTokenType('}'), &match->body);
}


namespace {

void
SetStatementPosition(Statement *statement, const Token &token)
{
    statement->lineNumber = token.lineNumber;
    statement->columnNumber = token.columnNumber;
}


void
ExpectToken(const Token &token, TokenType tokenType)
{
    if (token.type != tokenType) {
        throw SyntaxError::UnexpectedToken(token, tokenType);
    }
}


void
ExpectToken(const Token &token, TokenType tokenType1, TokenType tokenType2)
{
    if (token.type != tokenType1 && token.type != tokenType2) {
        throw SyntaxError::UnexpectedToken(token, tokenType1, tokenType2);
    }
}


std::string
EvaluateStringLiteral(const std::string &stringLiteral)
{
    std::string::const_iterator it = stringLiteral.begin() + 1;
    unsigned char c = *it;
    std::string string;

    for (;;) {
        if (c == '\\') {
            ++it;
            c = *it;
            int ascii;

            switch (c) {
            case '\"':
            case '\'':
            case '\?':
            case '\\':
                ascii = *it++;
                c = *it;
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                ascii = odigit2int(*it++);
                c = *it;

                if (isodigit(c)) {
                    ascii = ascii << 3 | odigit2int(*it++);
                    c = *it;

                    if (isodigit(c)) {
                        ascii = ascii << 3 | odigit2int(*it++);
                        c = *it;
                    }
                }

                break;

            case 'a':
                ascii = '\a';
                ++it;
                c = *it;
                break;

            case 'b':
                ascii = '\b';
                ++it;
                c = *it;
                break;

            case 'f':
                ascii = '\f';
                ++it;
                c = *it;
                break;

            case 'n':
                ascii = '\n';
                ++it;
                c = *it;
                break;

            case 'r':
                ascii = '\r';
                ++it;
                c = *it;
                break;

            case 't':
                ascii = '\t';
                ++it;
                c = *it;
                break;

            case 'v':
                ascii = '\v';
                ++it;
                c = *it;
                break;

            case 'x':
                ++it;
                ascii = xdigit2int(*it++);
                c = *it;

                if (std::isxdigit(c)) {
                    ascii = ascii << 4 | xdigit2int(*it++);
                    c = *it;
                }

                break;
            }

            string += ascii;
        } else {
            if (c == '\"') {
                return string;
            } else {
                string += *it++;
                c = *it;
            }
        }
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

} // namespace OYC
