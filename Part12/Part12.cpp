// implement simple parscal statement

#include <iostream>
#include <cstring>
#include <map>
#include <fstream>
#include "Part12.h"

const std::map<TokenType, std::string> tokenTypeToStr {
    {TokenType::INTEGER,    "INTEGER"   },    
    {TokenType::PLUS,       "PLUS"      },
    {TokenType::MINUS,      "MINUS"     },
    {TokenType::MUL,        "MUL"       },
    {TokenType::IntegerDiv, "IntegerDiv"},
    {TokenType::LParen,     "LParen"    },
    {TokenType::RParen,     "RParen"    },
    {TokenType::Assign,     "Assign"    },
    {TokenType::Semi,       "Semi"      },
    {TokenType::Dot,        "Dot"       },
    {TokenType::ID,         "ID"        },
    {TokenType::Begin,      "Begin"     },
    {TokenType::End,        "End"       },
    {TokenType::Program,    "Program"   },
    {TokenType::Var,        "Var"       },
    {TokenType::IntegerDiv, "IntegerDiv"},
    {TokenType::Integer,    "Integer"   },
    {TokenType::Real,       "Real"      },
    {TokenType::TYPE_EOF,   "TYPE_EOF"}
};

std::ostream& operator<<(std::ostream& os, const TokenType& tk) {
    os << tokenTypeToStr.at(tk);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Token& tk) {
    os << "Token (" << tk.type_ << ", " << tk.value_ << ")";
    return os;
}

Lexer::Lexer(std::string&& text) {
    textStart_ = (char*)malloc(text.length());
    memcpy(textStart_, text.data(), text.length());
    textEnd_ = textStart_ + text.length() - 1;
    currentPtr_ = textStart_;
    RESERVED_KEYWORDS = {
        {"PROGRAM", Token(TokenType::Program, "PROGRAM")},
        {"VAR", Token(TokenType::Var, "VAR")},
        {"DIV", Token(TokenType::IntegerDiv, "DIV")},
        {"INTEGER", Token(TokenType::Integer, "INTEGER")},
        {"REAL", Token(TokenType::Real, "REAL")},
        {"BEGIN", Token(TokenType::Begin, "BEGIN")},
        {"END", Token(TokenType::End, "End")},
        {"PROCEDURE", Token(TokenType::Procedure, "Procedure")}
    };
}

void Lexer::advance() {
    currentPtr_++;
    if (currentPtr_ > textEnd_) {
        currentPtr_ = nullptr;
    }
}

char* Lexer::peek() {
    char* peekPtr = currentPtr_ + 1;
    if (peekPtr > textEnd_) {
        return nullptr;
    } else {
        return peekPtr;
    }
}

void Lexer::skipWhiteSpace() {
    while (currentPtr_ != nullptr && std::isspace(*currentPtr_))
    {
        advance();
    }
}

/*
* Return a (multidigit) integer or float consumed from the input.
*/
Token Lexer::number() {
    // Return a (multidigit) integer consumed from the input.
    std::string result;
    while (currentPtr_ != nullptr && std::isdigit(*currentPtr_)) {
        result += *currentPtr_;
        advance();
    }

    if (*currentPtr_ == '.') {
        result += *currentPtr_;
        advance();

        while (currentPtr_ != nullptr && std::isdigit(*currentPtr_)) {
            result += *currentPtr_;
            advance();
        }
        
        return Token{TokenType::RealConst, result};
    } else {
        return Token{TokenType::IntegerConst, result};
    }
}

void Lexer::error() {
    throw std::runtime_error("Invalid character");
}

Token Lexer::_id() {
    std::string result;
    while (currentPtr_ != nullptr && std::isalnum(*currentPtr_)) {
        result += *currentPtr_;
        advance();
    }

    auto iter = RESERVED_KEYWORDS.insert(std::make_pair(result, Token(TokenType::ID, result)));
    return iter.first->second;
}

void Lexer::skipComment() {
    while (*currentPtr_ != '}') {
        advance();
    }
    advance(); // the closing curly brace
}

Token Lexer::getNextToken() {
    while (currentPtr_ != nullptr) {
        if (std::isspace(*currentPtr_)) {
            skipWhiteSpace();
            continue;
        }

        if (*currentPtr_ == '{') {
            skipComment();
            continue;
        }

        if (std::isdigit(*currentPtr_)) {
            return number();
        }

        if (*currentPtr_ == '+') {
            advance();
            return Token(TokenType::PLUS, "+");
        }

        if (*currentPtr_ == '-') {
            advance();
            return Token(TokenType::MINUS, "-");
        }

        if (*currentPtr_ == '*') {
            advance();
            return Token(TokenType::MUL, "*");
        }

        if (*currentPtr_ == '(') {
            advance();
            return Token(TokenType::LParen, "(");
        }

        if (*currentPtr_ == ')') {
            advance();
            return Token(TokenType::RParen, ")");
        }

        if (std::isalpha(*currentPtr_)) {
            return _id();
        }

        if (*currentPtr_ == ':' && *peek() == '=') {
            advance();
            advance();
            return Token(TokenType::Assign, ":=");
        }

        if (*currentPtr_ == ';') {
            advance();
            return Token(TokenType::Semi, ";");
        }

        if (*currentPtr_ == '.') {
            advance();
            return Token(TokenType::Dot, ".");
        }

        if (*currentPtr_ == ':') {
            advance();
            return Token(TokenType::Colon, ":");
        }

        if (*currentPtr_ == ',') {
            advance();
            return Token(TokenType::Comma, ",");
        }

        if (*currentPtr_ == '/') {
            advance();
            return Token(TokenType::FloatDiv, "/");
        }

        error();
    }

    return Token(TokenType::TYPE_EOF, "\0");
}

Parser::Parser(std::unique_ptr<Lexer>&& lexer) : 
                lexer_(std::move(lexer)), 
                currentToken_(lexer_->getNextToken()) {}

void Parser::eat(TokenType tktype) {
    if (currentToken_.type_ == tktype) {
        currentToken_ = lexer_->getNextToken();
    } else {
        error();
    }
}

AST* Parser::factor() { 
    Token token = currentToken_;
    if (currentToken_.type_ == TokenType::IntegerConst) {
        eat(TokenType::IntegerConst);
        return new Num(token);
    }  else if (currentToken_.type_ == TokenType::RealConst) {
        eat(TokenType::RealConst);
        return new Num(token);
    } else if (currentToken_.type_ == TokenType::LParen) {
        eat(TokenType::LParen);
        AST* node = expr();
        eat(TokenType::RParen);
        return node;
    } else if (currentToken_.type_ == TokenType::PLUS) {
        eat(TokenType::PLUS);
        AST* node = factor();
        return new UnaryOp(token, node);
    } else if (currentToken_.type_ == TokenType::MINUS) {
        eat(TokenType::MINUS);
        AST* node = factor();
        return new UnaryOp(token, node);
    } else if (currentToken_.type_ == TokenType::ID) {
        return variable();
    }
    
    error();
    return nullptr;
}

AST* Parser::term() {
    AST* result = factor();

    while (currentToken_.type_ == TokenType::MUL || currentToken_.type_ == TokenType::FloatDiv 
                            || currentToken_.type_ == TokenType::IntegerDiv) {
        Token op = currentToken_;
        if (currentToken_.type_ == TokenType::MUL) {
            eat(TokenType::MUL);
        } else if (currentToken_.type_ == TokenType::IntegerDiv) {
            eat(TokenType::IntegerDiv);
        } else if (currentToken_.type_ == TokenType::FloatDiv) {
            eat(TokenType::FloatDiv);
        }

        result = new BinOp(result, op, factor());
    }

    return result;
}

AST* Parser::expr() {
    AST* result = term();

    while (currentToken_.type_ == TokenType::PLUS || currentToken_.type_ == TokenType::MINUS) {
        Token tk = currentToken_;
        if (currentToken_.type_ == TokenType::PLUS) {
            eat(TokenType::PLUS);
        } else if (currentToken_.type_ == TokenType::MINUS) {
            eat(TokenType::MINUS);
        }

        result = new BinOp(result, tk, term());
    }

    return result;
}

Program* Parser::program() {
    eat(TokenType::Program);
    Var* varNode = variable();
    std::string progName = varNode->value_;
    eat(TokenType::Semi);
    Block* blk = block();

    Program* prog = new Program(progName, blk);
    eat(TokenType::Dot);
    return prog;
}

Compound* Parser::compoundStatement() {
    eat(TokenType::Begin);
    std::list<AST*> nodes =  statementList();
    eat(TokenType::End);

    Compound* root = new Compound();
    for (AST* node : nodes) {
        root->children_.push_back(node);
    }

    return root;
}

std::list<AST*> Parser::statementList() {
    AST* node = statement();
    std::list<AST*> result;
    result.push_back(node);

    while (currentToken_.type_ == TokenType::Semi) {
        eat(TokenType::Semi);
        result.push_back(statement());
    }

    // why in the last should we judge ID, is other token not important?
    if (currentToken_.type_ == TokenType::ID) {
        error();
    }

    return result;
}

AST* Parser::statement() {
    // how to judge it is compund or assign or empty
    // just using token type
    if (currentToken_.type_ == TokenType::Begin) {
        return compoundStatement();
    } else if (currentToken_.type_ == TokenType::ID) {
        return assignmentStatement();
    } else {
        return empty();
    }
}

AST* Parser::assignmentStatement() {
    Var* left = variable();
    Token op = currentToken_;
    eat(TokenType::Assign);
    AST* right = expr();
    AST* node = new Assign(left, op, right);

    return node;
}

Var* Parser::variable() {
    Var* node = new Var(currentToken_);
    eat(TokenType::ID);

    return node;
}

AST* Parser::empty() {
    return new NoOp();
}

Block* Parser::block() {
    std::list<AST*> decls = declarations();
    Compound* compState = compoundStatement();

    return new Block(decls, compState);
}

std::list<AST*> Parser::declarations() {
    std::list<AST*> decls;

    if (currentToken_.type_ == TokenType::Var) {
        eat(TokenType::Var);
        while (currentToken_.type_ == TokenType::ID) {
            std::list<VarDecl*> tmpDecls = variableDeclaration();
            decls.insert(decls.end(), tmpDecls.begin(), tmpDecls.end());
            eat(TokenType::Semi);
        }
    }

    while (currentToken_.type_ == TokenType::Procedure) {
        eat(TokenType::Procedure);
        std::string procName = currentToken_.value_;
        eat(TokenType::ID);
        eat(TokenType::Semi);
        Block* blk = block();
        ProcedureDecl* procDecl = new ProcedureDecl(procName, blk);
        decls.push_back(procDecl);
        eat(TokenType::Semi);
    }

    return decls;
}

std::list<VarDecl*> Parser::variableDeclaration() {
    std::list<Var*> varNodes;

    // first ID
    varNodes.push_back(new Var(currentToken_));
    eat(TokenType::ID);

    while (currentToken_.type_ == TokenType::Comma) {
        eat(TokenType::Comma);
        varNodes.push_back(new Var(currentToken_));
        eat(TokenType::ID);
    }
    eat(TokenType::Colon);

    Type* typeNode = typeSpec();

    std::list<VarDecl*> varDeclarations;
    for (auto* varNode : varNodes) {
        varDeclarations.emplace_back(new VarDecl(varNode, typeNode));
    }

    return varDeclarations;
}

Type* Parser::typeSpec() {
    if (currentToken_.type_ == TokenType::Integer) {
        eat(TokenType::Integer);
    } else {
        eat(TokenType::Real);
    }

    return new Type(currentToken_);
}

std::string SymbolTable::getPrettyPrintedString() {
    std::string ans;
    ans = "Symbols: {";
    auto iter = symbols_.begin();
    while (iter != symbols_.end()) {
        ans += iter->second->getPrettyPrintedString();
        iter++;
        if (iter != symbols_.end()) {
            ans += ",";
        }
    }
    ans += "}";
    return ans;
}

void SymbolTable::define(Symbol* symbol) {
    printf("Define: %s", symbol->getPrettyPrintedString().c_str());
    symbols_[symbol->name_] = symbol;
}

Symbol* SymbolTable::lookup(std::string& name) {
    printf("Lookup: %s", name.c_str());
    Symbol* symbol = symbols_[name];
    return symbol;
}

void SymbolTable::initBuiltins() {
    define(new BuiltinTypeSymbol("INTEGER"));
    define(new BuiltinTypeSymbol("REAL"));
}

void Interpreter::visit(Program& prog) {
    prog.block_->accept(*this);
}

void SymbolTableBuilder::visit(Block& blk) {
    for (AST* declaration : blk.declarations_) {
        declaration->accept(*this);
    }
    blk.compoundStatement_->accept(*this);
}

void SymbolTableBuilder::visit(Program& prog) {
    prog.block_->accept(*this);
}

void SymbolTableBuilder::visit(BinOp& bo) {
    bo.left_->accept(*this);
    bo.right_->accept(*this);
}

void SymbolTableBuilder::visit(UnaryOp& uo) {
    uo.expr_->accept(*this);
}

void SymbolTableBuilder::visit(Compound& comp) {
    for (AST* child : comp.children_) {
        child->accept(*this);
    }
}

void SymbolTableBuilder::visit(VarDecl& vDecl) {
    std::string& name = vDecl.typeNode_->value_;
    Symbol* typeSymbol = symtab.lookup(name);
    std::string& varName = vDecl.varNode_->value_;
    // 下面的强转只是基于当前的type只有builtin的情况下成立
    VarSymbol* varSymbol = new VarSymbol(varName, static_cast<BuiltinTypeSymbol*>(typeSymbol));
    symtab.define(varSymbol);
}

void SymbolTableBuilder::visit(Assign& as) {
    std::string& name = as.left_->value_;
    Symbol* varSymbol = symtab.lookup(name);
    if (varSymbol == nullptr) {
        std::string str = "variable " + name + "not declared";
        throw std::runtime_error(str);
    }
    as.right_->accept(*this);
}

void SymbolTableBuilder::visit(Var& var) {
    std::string& name = var.value_;
    Symbol* varSymbol = symtab.lookup(name);
    if (varSymbol == nullptr) {
        std::string str = "variable " + name + "not declared";
        throw std::runtime_error(str);
    }
}

void Interpreter::visit(Block& blk) {
    for (AST* decl : blk.declarations_) {
        decl->accept(*this);
    }
    blk.compoundStatement_->accept(*this);
}

void Interpreter::visit(VarDecl& vDecl) {
    // Do nothig
}

void Interpreter::visit(Type& tp) {
    // Do nothig
}

int Interpreter::visit(BinOp& bo) {
    if (bo.op_.type_ == TokenType::PLUS) {
        return bo.left_->accept(*this) + bo.right_->accept(*this);
    }
    if (bo.op_.type_ == TokenType::MINUS) {
        return bo.left_->accept(*this) - bo.right_->accept(*this);
    }
    if (bo.op_.type_ == TokenType::MUL) {
        return bo.left_->accept(*this) * bo.right_->accept(*this);
    }
    if (bo.op_.type_ == TokenType::IntegerDiv) {
        return bo.left_->accept(*this) / bo.right_->accept(*this);
    }
    if (bo.op_.type_ == TokenType::FloatDiv) {
        return (float)(bo.left_->accept(*this)) / (float)(bo.right_->accept(*this));
    }
}

int Interpreter::visit(UnaryOp& uo) {
    if (uo.op_.type_ == TokenType::PLUS) {
        return uo.expr_->accept(*this);
    } else if (uo.op_.type_ == TokenType::MINUS) {
        return -uo.expr_->accept(*this);
    }
}

int Interpreter::visit(Num& num) {
    return stoi(num.value_);
}

void Interpreter::visit(Compound& comp) {
    for (AST* child : comp.children_) {
        child->accept(*this);
    }
}

void Interpreter::visit(NoOp& noop) {

}

void Interpreter::visit(Assign& as) {
    std::string varName = as.left_->value_;
    GLOBAL_SCOPE[varName] = as.right_->accept(*this);
}

int Interpreter::visit(Var& var) {
    std::string varName = var.value_;
    auto it = GLOBAL_SCOPE.find(varName);
    if (it == GLOBAL_SCOPE.end()) {
        throw std::runtime_error("variable not defined");
    }

    return it->second;
}

void Interpreter::printGlobalScope() {
    std::cout << "{";
    auto iter = GLOBAL_SCOPE.begin();
    while (iter != GLOBAL_SCOPE.end()) {
        std::cout << iter->first << ": " << iter->second;
        iter++;
        if (iter != GLOBAL_SCOPE.end()) {
            std::cout << ", ";
        }
    }
    std::cout << "}";
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "please input your file" << std::endl;
    }
    const std::string filepath(argv[1]);
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return 1;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(std::move(content));
    std::unique_ptr<Parser> parser = std::make_unique<Parser>(std::move(lexer));
    Interpreter interp(std::move(parser));
    interp.interpret();
    interp.printGlobalScope();

    file.close();

    return 0;
}