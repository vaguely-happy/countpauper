﻿#include "stdafx.h"
#include "BNF.h"
#include "Parser.h"
#include "StringEncoding.h"
#include <regex>

namespace Angel::Parser::BNF
{

PossibleMatch Nothing::Parse(const std::string_view data, const Progress&) const
{
    return Match( data );
}

PossibleMatch Literal::Parse(const std::string_view data, const Progress&) const
{
    if (data.substr(0,literal.size()).compare(literal)==0)
        return Match( data.substr(literal.size()) );
    else
        return PossibleMatch();
}


PossibleMatch RegularExpression::Parse(const std::string_view data, const Progress&) const
{

    std::wstring wdata = from_utf8(data);
    std::wstring wexpression = from_utf8(expression);
    std::wregex  wexpr(wexpression);
    std::wsmatch match;
    if (std::regex_search(wdata, match, wexpr, std::regex_constants::match_continuous))
    {
        std::string match_str = to_utf8(match.str(0));
        return Match(data.substr(match_str.size()));
    }

    return PossibleMatch();
}

Whitespace::Whitespace(size_t amt, const std::string_view chars) :
    amount(amt),
    characters(chars)
{
}


PossibleMatch Whitespace::Parse(const std::string_view data, const Progress&) const
{
    if ((data.empty()) && (amount))
        return PossibleMatch();
    auto p = data.find_first_not_of(characters);
    if (p == data.npos)
        return Match{ data.substr(data.size()) };
    else if (p>=amount)
        return Match{ data.substr(p) };
    else
        return PossibleMatch();
}

PossibleMatch Disjunction::Parse(const std::string_view data , const Progress& progress) const
{
    for (const auto& e : expressions)
    {
        auto m = e->Parse(data, progress);
        if (m)
            return m;
    }
    return PossibleMatch();
}

PossibleMatch Sequence::Parse(const std::string_view data , const Progress& progress) const
{
    auto remaining = data;
    std::vector<std::any> tokens;
    Match::Rules lastRules;
    for (const auto& e : expressions)
    {
        auto m = e->Parse(remaining, progress);
        if (!m)
            return PossibleMatch();
        if (m->tokens.has_value())
        {
            // flatten vectors of any TODO use MergeFn
            if (m->tokens.type() == typeid(std::vector<std::any>))
            {
                auto mergeVector = std::any_cast<std::vector<std::any>>(m->tokens);
                tokens.insert(tokens.end(), mergeVector.begin(), mergeVector.end());
            }
            else
            {
                tokens.push_back(m->tokens);
            }
        }
                
        remaining = m->remaining;
        if (!m->rules.empty())
            lastRules = m->rules;
    }
    if (tokens.empty())
        return Match{ remaining };
    else if (tokens.size() == 1)
        return Match{ remaining, tokens.front(), lastRules };
    else
        return Match{ remaining, tokens, lastRules };
}

PossibleMatch Loop::Parse(const std::string_view data , const Progress& progress) const
{
    auto remaining = data;
    std::any tokens;
    Match::Rules lastRules;
    int index = 0;
    while (!remaining.empty())
    {
        auto m = expression->Parse(remaining, progress);
        if (!m)
            break;
        if (m->remaining == remaining)
            break;

        auto len = m->remaining.data() - remaining.data();
        auto parsed = remaining.substr(0,len);
        std::string indexKey = std::string("[") + std::to_string(index++) + "]";
        tokens = merge(tokens, m->tokens);
        remaining = m->remaining;
        if (!m->rules.empty())
            lastRules = m->rules;
    }
    return Match{ remaining, tokens, lastRules };
}

Rule::Rule(const std::string_view n, const Expression& e) :
    name(n),
    expression(e),
    construct(PassToken)
{
    Declare::Define(*this);
}

Rule::Rule(const std::string_view n, const Expression& e, ParseFn p) :
    Rule(n, e)
{
    parse = p;
}

Rule::Rule(const std::string_view n, const Expression& e, ConstructFn c) :
    Rule(n, e)
{
    construct = c;
}

std::any Rule::Tokenize(std::any tokens, const std::string_view data) const
{
    // Rules are currently initialized to either parse the string data into a token 
    //  or to construct the tokens returned by referenced sub rules into a higher level structure
    //  it's not yet supported to do both yet, because what would be the use case? 


    std::string inType, outType;
    std::any result;
    if (parse)
    {
        inType = data;
        result = parse(data);
    }
    else 
    {
        // convert input type to string 
        if (tokens.type() == typeid(Logic::Object))
        {
            const auto& obj = std::any_cast<Logic::Object>(tokens);
            inType = obj->String();
        }
        else
        {
            inType = tokens.type().name();
        }
        if (construct)
            result = construct(tokens);
        else
            result = tokens;
    }
    if (result.has_value())
    {
        outType = result.type().name();
        if (result.type() == typeid(Logic::Object))
        {
            const auto& obj = std::any_cast<Logic::Object>(result);
            outType = obj->String();
        }
    }
    else
    {
        outType = "none";
    }
    return result;  // Tokenize {name}({data}) with { inType } as { outType }
}

std::any Rule::VoidParse(const std::string_view) 
{ 
    return std::any(); 
}


std::any Rule::PassToken(std::any tokens)
{
    return tokens;
}


Recursive::Recursive(const std::string_view n, const Expression& e, ConstructFn c) :
    Rule(n, e, c)
{
    recursive = true;
}

PossibleMatch Rule::Parse(const std::string_view data , const Progress& progress) const
{
    Progress newProgress = progress;
    if (!recursive)
    {
        // infinite recursion protection
        auto previousProgress = progress.find(this);
        if (previousProgress != progress.end() && previousProgress->second == data.size())
            return PossibleMatch();     // Recursion stop { name } @ { data} 
        newProgress[this] = data.size();
    }
    auto m = expression->Parse(data, newProgress);
    if (!m)
        return m;               // Failed to lex { data } as { name }
    else
    {
        auto len = m->remaining.data() - data.data();
        std::string_view parsed = data.substr(0, len);
        auto newRules = m->rules;   // Lexed { parsed } as { name }
        newRules.push_back(this);

        return Match{ m->remaining, Tokenize(m->tokens, parsed), newRules };
    }
}

Declare::Declare(const std::string_view rule) :
    rule(rule)
{
}

std::map<const std::string_view, const Rule*> Declare::definitions;

void Declare::Define(const Rule& rule)
{
    definitions.insert(std::make_pair(rule.name, &rule));
}

const Rule& Declare::Get() const
{
    auto it = definitions.find(rule);
    if (it == definitions.end())
        throw std::runtime_error(std::string("Undefined rule ") + std::string(rule));
    return *it->second;
}


PossibleMatch Declare::Parse(const std::string_view data , const Progress& progress) const
{
    auto& rule = Get();
    return rule.Parse(data, progress);
}


PossibleMatch Ref::Parse(const std::string_view data, const Progress& progress) const
{
    return rule->Parse(data, progress);
}

Match Parse(const Rule& root, const std::string_view data)
{
    Progress start;
    auto possible = root.Parse(data, start);
    if (!possible)
        throw SyntaxError(std::string("Couldn't match ") + std::string(root.name) +" at:" + std::string(data));
    else
        return *possible;
}

}
