#include "Parser.h"

#include <cstdlib>
#include <cctype>
#include <iterator>

#include "Expressions.h"
#include "SyntaxError.h"


namespace OYC {

namespace {

const TokenType TernaryOpSet[][2] = {
    {MakeTokenType('?'), MakeTokenType(':')},
    {TokenType::No, TokenType::No}
};

const TokenType BinaryOpSet1[] = {
    MakeTokenType(','),
    TokenType::No
};

const TokenType BinaryOpSet2[] = {
    MakeTokenType('='),
    MakeTokenType('|', '='),
    MakeTokenType('^', '='),
    MakeTokenType('&', '='),
    MakeTokenType('<', '<', '='),
    MakeTokenType('>', '>', '='),
    MakeTokenType('+', '='),
    MakeTokenType('-', '='),
    MakeTokenType('*', '='),
    MakeTokenType('/', '='),
    MakeTokenType('%', '='),
    TokenType::No
};

const TokenType BinaryOpSet3[] = {
    MakeTokenType('|', '|'),
    TokenType::No
};

const TokenType BinaryOpSet4[] = {
    MakeTokenType('&', '&'),
    TokenType::No
};

const TokenType BinaryOpSet5[] = {
    MakeTokenType('|'),
    TokenType::No
};

const TokenType BinaryOpSet6[] = {
    MakeTokenType('^'),
    TokenType::No
};

const TokenType BinaryOpSet7[] = {
    MakeTokenType('&'),
    TokenType::No
};

const TokenType BinaryOpSet8[] = {
    MakeTokenType('=', '='),
    MakeTokenType('!', '='),
    TokenType::No
};

const TokenType BinaryOpSet9[] = {
    MakeTokenType('<'),
    MakeTokenType('<', '='),
    MakeTokenType('>', '='),
    MakeTokenType('>'),
    TokenType::No
};

const TokenType BinaryOpSet10[] = {
    MakeTokenType('<', '<'),
    MakeTokenType('>', '>'),
    TokenType::No
};

const TokenType BinaryOpSet11[] = {
    MakeTokenType('+'),
    MakeTokenType('-'),
    TokenType::No
};

const TokenType BinaryOpSet12[] = {
    MakeTokenType('*'),
    MakeTokenType('/'),
    MakeTokenType('%'),
    TokenType::No
};

const TokenType *const BinaryOpSets[] = {
    BinaryOpSet1,
    BinaryOpSet2,
    BinaryOpSet3,
    BinaryOpSet4,
    BinaryOpSet5,
    BinaryOpSet6,
    BinaryOpSet7,
    BinaryOpSet8,
    BinaryOpSet9,
    BinaryOpSet10,
    BinaryOpSet11,
    BinaryOpSet12,
};

const TokenType UnaryOpSet1[] = {
    MakeTokenType('+', '+'),
    MakeTokenType('-', '-'),
    MakeTokenType('+'),
    MakeTokenType('-'),
    MakeTokenType('!'),
    MakeTokenType('~'),
    TokenType::No
};

const TokenType UnaryOpSet2[] = {
    MakeTokenType('+', '+'),
    MakeTokenType('-', '-'),
    TokenType::No
};


void ExpectToken(const Token &, TokenType);
std::string EvaluateStringLiteral(const std::string &);

bool isodigit(int);
int odigit2int(char);
int xdigit2int(char);

} // namespace


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
Parser::matchBody(std::vector<std::unique_ptr<Statement>> *body, TokenType bodyTerminator)
{
    if (bodyTerminator == TokenType::No) {
        std::unique_ptr<Statement> statement = matchStatement();

        if (statement != nullptr) {
            body->push_back(std::move(statement));
        }
    } else {
        for (const Token *token = &peekToken(1); token->type != bodyTerminator
             ; token = &peekToken(1)) {
            std::unique_ptr<Statement> statement = matchStatement();

            if (statement != nullptr) {
                body->push_back(std::move(statement));
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
Parser::matchAutoStatement()
{
    auto match = std::make_unique<AutoStatement>();
    SetStatementPosition(match, readToken());
    match->variableDeclarators.push(matchVariableDeclarator());
    const Token *token = &peekToken(1);

    while (token->type == MakeTokenType(',')) {
        readToken();
        match->variableDeclarators.push(matchVariableDeclarator());
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
    SetStatementPosition(match, readToken());
    ExpectToken(peekToken(1), MakeTokenType(';'));
    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchContinueStatement()
{
    auto match = std::make_unique<BreakStatement>();
    SetStatementPosition(match, readToken());
    ExpectToken(peekToken(1), MakeTokenType(';'));
    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchReturnStatement()
{
    auto match = std::make_unique<ReturnStatement>();
    SetStatementPosition(match, readToken());
    const Token *token = &peekToken(1);

    if (token->type != MakeTokenType(';')) {
        match->result = matchExpression();
        ExpectToken(peekToken(1), MakeTokenType(';'));
    }

    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchExpressionStatement()
{
    auto match = std::make_unique<ReturnStatement>();
    SetStatementPosition(match, peekToken(1));
    match->expression = matchExpression();
    ExpectToken(peekToken(1), MakeTokenType(';'));
    readToken();
    return match;
}


std::unique_ptr<Statement>
Parser::matchIfStatement()
{
    auto match = std::make_unique<IfStatement>();
    SetStatementPosition(match, readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    match->condition = matchExpression();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchBody(&match->thenBody, MakeTokenType('}'));
    } else {
        matchBody(&match->thenBody, TokenType::No);
    }

    token = &peekToken(1);

    if (token->type == TokenType::ElseKeyword) {
        readToken();
        token = &peekToken(1);

        if (token->type == MakeTokenType('{')) {
            readToken();
            matchBody(&match->elseBody, MakeTokenType('}'));
        } else {
            matchBody(&match->elseBody, TokenType::No);
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
    SetStatementPosition(match, readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    match->condition = matchExpression();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchBody(&match->body, MakeTokenType('}'));
    } else {
        matchBody(&match->body, TokenType::No);
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
        matchBody(&match->body, MakeTokenType('}'));
    } else {
        matchBody(&match->body, TokenType::No);
    }

    ExpectToken(peekToken(1), TokenType::WhileKeyword);
    SetStatementPosition(match, readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    match->condition = matchExpression();
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
    SetStatementPosition(match, readToken());
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
        match->condition = matchExpression();
        ExpectToken(peekToken(1), MakeTokenType(';'));
    }

    readToken();
    token = &peekToken(1);

    if (token->type != MakeTokenType(')')) {
        match->iteration = matchExpression();
        ExpectToken(peekToken(1), MakeTokenType(')'));
    }

    readToken();
    token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchBody(&match->body, MakeTokenType('}'));
    } else {
        matchBody(&match->body, TokenType::No);
    }

    return match;
}


std::unique_ptr<Statement>
Parser::matchForeachStatement()
{
    auto match = std::make_unique<ForeachStatement>();
    SetStatementPosition(match, readToken());
    ExpectToken(peekToken(1), MakeTokenType('('));
    readToken();
    ExpectToken(peekToken(1), TokenType::AutoKeyword);
    readToken();
    ExpectToken(peekToken(1), TokenType::Identifier);
    match->identifier1 = getIdentifier();
    ExpectToken(peekToken(1), MakeTokenType(','));
    readToken();
    ExpectToken(peekToken(1), TokenType::Identifier);
    match->identifier2 = getIdentifier();
    ExpectToken(peekToken(1), MakeTokenType(':'));
    readToken();
    match->aggregate = matchExpression1();
    ExpectToken(peekToken(1), MakeTokenType(')'));
    readToken();
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('{')) {
        readToken();
        matchBody(&match->body, MakeTokenType('}'));
    } else {
        matchBody(&match->body, TokenType::No);
    }

    return match;
}


std::unique_ptr<Expression>
Parser::matchExpression1()
{
    std::unique_ptr<Expression> result = matchExpression2();
    const Token *token = &peekToken(1);

    for (;;) {
        bool foundFlag = false;

        for (const TokenType *p = BinaryOpSet1; *p != TokenType::No; ++p) {
            if (token->type == *p) {
                foundFlag = true;
                auto match = std::make_unique<BinaryExpression>();
                match->operand1 = std::move(result);
                match->op = readToken().type;
                match->operand2 = matchExpression2();
                result = std::move(match);
                token = &peekToken(1);
                break;
            }
        }

        if (!foundFlag) {
            break;
        }
    }

    return result;
}


std::unique_ptr<Expression>
Parser::matchExpression2()
{
    std::unique_ptr<Expression> result = matchExpression3(3);
    const Token *token = &peekToken(1);
    bool foundFlag = false;

    for (const TokenType (*p)[2] = TernaryOpSet; (*p)[0] != TokenType::No
                                                 && (*p)[1] != TokenType::No; ++p) {
        if (token->type == (*p)[0]) {
            foundFlag = true;
            auto match = std::make_unique<TernaryExpression>();
            match->operand1 = std::move(result);
            match->op[0] = readToken().type;
            match->operand2 = matchExpression2();
            ExpectToken(peekToken(1), (*p)[1]);
            match->op[1] = readToken().type;
            match->operand3 = matchExpression2();
            result = std::move(match);
            break;
        }
    }

    if (!foundFlag) {
        for (const TokenType *p = BinaryOpSet2; *p != TokenType::No; ++p) {
            if (token->type == *p) {
                auto match = std::make_unique<BinaryExpression>();
                match->operand1 = std::move(result);
                match->op = readToken().type;
                match->operand2 = matchExpression2();
                result = std::move(match);
                break;
            }
        }
    }

    return result;
}


std::unique_ptr<Expression>
Parser::matchExpression3(int lowestPrecedence)
{
    std::unique_ptr<Expression> result = matchExpression4();

    for (int precedence = 12; precedence >= lowestPrecedence; --precedence) {
        const TokenType *binaryOpSet = BinaryOpSets[precedence - 1];
        const Token *token = &peekToken(1);

        for (;;) {
            bool foundFlag = false;

            for (const TokenType *p = binaryOpSet; *p != TokenType::No; ++p) {
                if (token->type == *p) {
                    foundFlag = true;
                    auto match = std::make_unique<BinaryExpression>();
                    match->operand1 = std::move(result);
                    match->op = readToken().type;
                    match->operand2 = matchExpression3(precedence + 1);
                    result = std::move(match);
                    token = &peekToken(1);
                    break;
                }
            }

            if (!foundFlag) {
                break;
            }
        }
    }

    return result;
}


std::unique_ptr<Expression>
Parser::matchExpression4()
{
    const Token *token = &peekToken(1);

    for (const TokenType *p = UnaryOpSet1; *p != TokenType::No; ++p) {
        if (token->type == *p) {
            auto match = std::make_unique<UnaryExpression>();
            match->type = UnaryExpressionType::Prefix;
            match->op = readToken().type;
            match->operand = matchExpression4();
            return match;
        }
    }

    return matchExpression5();
}


std::unique_ptr<Expression>
Parser::matchExpression5()
{
    std::unique_ptr<Expression> result = matchExpression6();
    const Token *token = &peekToken(1);

    for (;;) {
        bool foundFlag = false;

        for (const TokenType *p = UnaryOpSet2; *p != TokenType::No; ++p) {
            if (token->type == *p) {
                foundFlag = true;
                auto match = std::make_unique<UnaryExpression>();
                match->type = UnaryExpressionType::Postfix;
                match->op = readToken().type;
                match->operand = std::move(result);
                result = std::move(match);
                token = &peekToken(1);
                break;
            }
        }

        if (!foundFlag) {
            if (token->type == MakeTokenType('.')) {
                auto match = std::make_unique<RetrievalExpression>();
                match->retrievee = std::move(result);
                match->key = matchExpression7();
                result = std::move(match);
                token = &peekToken(1);
            } else if (token->type == MakeTokenType('[')) {
                auto match = std::make_unique<RetrievalExpression>();
                match->retrievee = std::move(result);
                readToken();
                match->key = matchExpression1();
                ExpectToken(peekToken(1), MakeTokenType(']'));
                readToken();
                result = std::move(match);
                token = &peekToken(1);
            } else if (token->type == MakeTokenType('(')) {
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
            } else {
                break;
            }
        }
    }

    return result;
}


std::unique_ptr<Expression>
Parser::matchExpression6()
{
    const Token *token = &peekToken(1);

    if (token->type == MakeTokenType('(')) {
        readToken();
        std::unique_ptr<Expression> result = matchExpression1();
        ExpectToken(peekToken(1), MakeTokenType(')'));
        readToken();
        return result;
    } else {
        auto match = std::make_unique<PrimaryExpression>();

        switch (token->type) {
        case TokenType::NilKeyword:
            match->type = PrimaryExpressionType::Nil;
            break;

        case TokenType::FalseKeyword:
        case TokenType::TrueKeyword:
            match->type = PrimaryExpressionType::Boolean;
            match->boolean = getBoolean();
            break;

        case TokenType::IntegerLiteral:
            match->type = PrimaryExpressionType::Integer;
            match->integer = getInteger();
            break;

        case TokenType::FloatingPointLiteral:
            match->type = PrimaryExpressionType::FloatingPoint;
            match->floatingPoint = getFloatingPoint();
            break;

        case TokenType::StringLiteral:
            match->type = PrimaryExpressionType::String;
            match->string = getString();
            break;

        case TokenType::Identifier:
            match->type = PrimaryExpressionType::Identifier;
            match->identifier = getIdentifier();
            break;

        case MakeTokenType('{'):
            match->type = PrimaryExpressionType::ArrayLiteral;
            match->arrayLiteral = matchArrayLiteral();
            break;

        case TokenType::DictKeyword:
            match->type = PrimaryExpressionType::DictionaryLiteral;
            match->dictionaryLiteral = matchDictionaryLiteral();
            break;

        case TokenType::FuncKeyword:
            match->type = PrimaryExpressionType::FunctionLiteral;
            match->functionLiteral = matchFunctionLiteral();
            break;

        default:
            ExpectToken(*token, TokenType::No);
        }

        return match;
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
    // std::string string = EvaluateStringLiteral(readToken().value);
    // const Token *token = &peekToken(1);

    // while (token->type == TokenType::StringLiteral) {
    //     string.append(EvaluateStringLiteral(readToken().value));
    //     token = &peekToken(1);
    // }

    // std::pair<std::unordered_set<std::string>::iterator
    //           , bool> result = programData_->strings.insert(std::move(string));
    // return &*result.first;
}


const std::string *
Parser::getIdentifier()
{
    // std::pair<std::unordered_set<std::string>::iterator
    //           , bool> result = programData_->identifiers.insert(readToken().value);
    // return &*result.first;
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

        readToken();
    }

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
            if (token->type == MakeTokenType('.')) {
                std::unique_ptr<Expression> first = matchExpression7();
                ExpectToken(peekToken(1), MakeTokenType('='));
                readToken();
                match->elements.emplace_back(std::move(first), matchExpression2());
            } if (token->type == MakeTokenType('[')) {
                readToken();
                std::unique_ptr<Expression> first = matchExpression1();
                ExpectToken(peekToken(1), MakeTokenType(']'));
                readToken();
                ExpectToken(peekToken(1), MakeTokenType('='));
                readToken();
                match->elements.emplace_back(std::move(first), matchExpression2());
            } else {
                ExpectToken(*token, TokenType::No);
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

        readToken();
    }

    return match;
}


const FunctionLiteral *
Parser::matchFunctionLiteral()
{
}


namespace {

void
ExpectToken(const Token &token, TokenType tokenType)
{
    if (token.type != tokenType) {
        throw SyntaxError::UnexpectedToken(token, tokenType);
    }
}


std::string
EvaluateStringLiteral(const std::string &stringLiteral)
{
    std::string string;
    std::string::const_iterator it = stringLiteral.begin() + 1;
    unsigned char c = *it;

    while (c != '\"') {
        if (c == '\\') {
            int ascii;
            ++it;
            c = *it;

            if (isodigit(c)) {
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
            } else if (c == 'X' || c == 'x') {
                ++it;
                ascii = xdigit2int(*it++);
                c = *it;

                if (std::isxdigit(c)) {
                    ascii = ascii << 4 | xdigit2int(*it++);
                    c = *it;
                }
            } else {
                switch (c) {
                case '\"':
                case '\'':
                case '\?':
                case '\\':
                    ascii = c;
                    break;

                case 'a':
                    ascii = '\a';
                    break;

                case 'b':
                    ascii = '\b';
                    break;

                case 'f':
                    ascii = '\f';
                    break;

                case 'n':
                    ascii = '\n';
                    break;

                case 'r':
                    ascii = '\r';
                    break;

                case 't':
                    ascii = '\t';
                    break;

                case 'v':
                    ascii = '\v';
                    break;
                }

                ++it;
                c = *it;
            }

            string += ascii;
        } else {
            string += *it++;
            c = *it;
        }
    }

    return string;
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
