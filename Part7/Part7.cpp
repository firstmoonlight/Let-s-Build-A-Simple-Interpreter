// this part implement AST

/*********************************************************************************************************************
 * 
 * LEXER
 * 
**********************************************************************************************************************/


/*
* Token types
* EOF (end-of-file) token is used to indicate that
* there is no more input left for lexical analysis
*/
#include <iostream>
#include <string>
#include <memory>
#include <cassert>

enum class TokenType {
    INTEGER,
    PLUS,
    MINUS,
    MUL,
    DIV,
    LParen,
    RParen,
    TYPE_EOF,
};

class BinOp;
class Num;

class Token {
 public:
    Token(TokenType type, std::string value) : type_(type), value_(value) {}

    TokenType type_;
    std::string value_;
};

class Lexer {
 public:
    explicit Lexer(std::string&& text) : text_(text), currentChar_(text[0]) {}
    void advance() {
        pos_++;
        if (pos_ > text_.length() - 1) {
            currentChar_ = '\0';
        } else {
            currentChar_ = text_[pos_];
        }
    }
    void skipWhiteSpace() {
        while (currentChar_ != '\0' && std::isspace(currentChar_))
        {
            advance();
        }
    }
    std::string integer() {
        // Return a (multidigit) integer consumed from the input.
        std::string result;
        while (currentChar_ != '\0' && std::isdigit(currentChar_)) {
            result += currentChar_;
            advance();
        }

        return result;
    }

    void error() {
        throw std::runtime_error("Invalid character");
    }

    /*
    * Lexical analyzer (also known as scanner or tokenizer)
    * This method is responsible for breaking a sentence
    * apart into tokens. One token at a time.
    * */
    Token getNextToken() {
        while (currentChar_ != '\0') {
            if (std::isspace(currentChar_)) {
                skipWhiteSpace();
                continue;
            }

            if (std::isdigit(currentChar_)) {
                return Token(TokenType::INTEGER, integer());
            }

            if (currentChar_ == '+') {
                advance();
                return Token(TokenType::PLUS, "+");
            }

            if (currentChar_ == '-') {
                advance();
                return Token(TokenType::MINUS, "-");
            }

            if (currentChar_ == '*') {
                advance();
                return Token(TokenType::MUL, "*");
            }

            if (currentChar_ == '/') {
                advance();
                return Token(TokenType::DIV, "/");
            }

            if (currentChar_ == '(') {
                advance();
                return Token(TokenType::LParen, "(");
            }

            if (currentChar_ == ')') {
                advance();
                return Token(TokenType::RParen, ")");
            }

            error();
        }

        return Token(TokenType::TYPE_EOF, "\0");
    }

 private:   
    std::string text_;
    char currentChar_;
    int pos_ = 0; 
};

/*********************************************************************************************************************
 * 
 * PARSER
 * 
**********************************************************************************************************************/
class NodeVisitor {
 public:
    virtual int visit(BinOp& bo) { assert(0); }
    virtual int visit(Num& num) { assert(0); }
};

class AST {
 public:
    virtual int accept(NodeVisitor& visitor) = 0;
};

class BinOp : public AST {
 public:
    BinOp(AST* left, Token op, AST* right) : left_(left), op_(op), right_(right) {}

    int accept(NodeVisitor& visitor) override {
        return visitor.visit(*this);
    }
 
    Token op_;
    AST* left_ = nullptr;
    AST* right_ = nullptr;
};

class Num : public AST {
 public:
    Num(Token& token) : token_(token), value_(token.value_) {}
    int accept(NodeVisitor& visitor) override {
        return visitor.visit(*this);
    }

    Token token_;
    std::string value_;
};




class Parser {
 public:
    Parser(std::unique_ptr<Lexer>&& lexer) : 
                    lexer_(std::move(lexer)), 
                    currentToken_(lexer_->getNextToken()) {}

    void error() {
        throw std::runtime_error("Invalid syntax");
    }

    /*
    * compare the current token type with the passed token
    * type and if they match then "eat" the current token
    * and assign the next token to the self.current_token,
    * otherwise raise an exception.
    */
    void eat(TokenType tktype) {
        if (currentToken_.type_ == tktype) {
            currentToken_ = lexer_->getNextToken();
        } else {
            error();
        }
    }

    /*
    * Return an INTEGER token value.
    * factor : INTEGER | LParen expr RParen
    */
    AST* factor() { 
        if (currentToken_.type_ == TokenType::INTEGER) {
            Token token = currentToken_;
            eat(TokenType::INTEGER);
            return new Num(token);
        } else if (currentToken_.type_ == TokenType::LParen) {
            eat(TokenType::LParen);
            AST* node = expr();
            eat(TokenType::RParen);
            return node;
        }
    }

    /*
    * term : factor ((MUL | DIV) factor)*
    */
    AST* term() {
        AST* result = factor();

        while (currentToken_.type_ == TokenType::MUL || currentToken_.type_ == TokenType::DIV) {
            Token tk = currentToken_;
            if (currentToken_.type_ == TokenType::MUL) {
                eat(TokenType::MUL);
                
            } else if (currentToken_.type_ == TokenType::DIV) {
                eat(TokenType::DIV);
                
            }

            result = new BinOp(result, tk, factor());
        }

        return result;
    }

    /*
    * Arithmetic expression parser / interpreter.
    *    expr   : term ((PLUS | MINUS) term)*
    *    term   : factor ((MUL | DIV) factor)*
    *    factor : INTEGER
    */
    AST* expr() {
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

    AST* parse() {
        return expr();
    }

 private:
    std::unique_ptr<Lexer> lexer_;
    Token currentToken_;
};

/*********************************************************************************************************************
 * 
 * INTERPRETER
 * 
**********************************************************************************************************************/


class Interpreter : public NodeVisitor {
 public: 
    explicit Interpreter(std::unique_ptr<Parser> parser) : parser_(std::move(parser)) {}
    int visit(BinOp& bo) override {
        if (bo.op_.type_ == TokenType::PLUS) {
            return bo.left_->accept(*this) + bo.right_->accept(*this);
        }
        if (bo.op_.type_ == TokenType::MINUS) {
            return bo.left_->accept(*this) - bo.right_->accept(*this);
        }
        if (bo.op_.type_ == TokenType::MUL) {
            return bo.left_->accept(*this) * bo.right_->accept(*this);
        }
        if (bo.op_.type_ == TokenType::DIV) {
            return bo.left_->accept(*this) / bo.right_->accept(*this);
        }
    }

    int visit(Num& num) override {
        return stoi(num.value_);
    }

    int interpret() {
        AST* tree = parser_->expr();
        return tree->accept(*this);
    }

 private:
    std::unique_ptr<Parser> parser_;
};

int main() {
    while (1) {
        std::string inputStr;
        std::cout << "cal> ";
        getline(std::cin, inputStr);

        std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(std::move(inputStr));
        std::unique_ptr<Parser> parser = std::make_unique<Parser>(std::move(lexer));
        Interpreter interp(std::move(parser));
        int result = interp.interpret();

        std::cout << result << std::endl;
    }
}