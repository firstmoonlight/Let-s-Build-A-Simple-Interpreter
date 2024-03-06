/*
* Token types
* EOF (end-of-file) token is used to indicate that
* there is no more input left for lexical analysis
*/
#include <iostream>
#include <string>
#include <memory>

enum class TokenType {
    INTEGER,
    MUL,
    DIV,
    TYPE_EOF,
};

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

            if (currentChar_ == '*') {
                advance();
                return Token(TokenType::MUL, "*");
            }

            if (currentChar_ == '/') {
                advance();
                return Token(TokenType::DIV, "/");
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

class Interpreter {
 public:
    Interpreter(std::unique_ptr<Lexer>&& lexer) : 
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
    * factor : INTEGER
    */
    std::string factor() {
        Token token = currentToken_;
        eat(TokenType::INTEGER);
        return token.value_;
    }

    /*
    * Arithmetic expression parser / interpreter.
    * expr   : factor ((MUL | DIV) factor)*
    * factor : INTEGER
    */
    int expr() {
        int result = stoi(factor());

        while (currentToken_.type_ == TokenType::MUL || currentToken_.type_ == TokenType::DIV) {
            if (currentToken_.type_ == TokenType::MUL) {
                eat(TokenType::MUL);
                result = result * stoi(factor());
            } else if (currentToken_.type_ == TokenType::DIV) {
                eat(TokenType::DIV);
                result = result / stoi(factor());
            }
        }

        return result;
    }

 private:
    std::unique_ptr<Lexer> lexer_;
    Token currentToken_;
};

int main() {
    std::string inputStr;
    std::cout << "cal> ";
    getline(std::cin, inputStr);

    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(std::move(inputStr));
    Interpreter interp(std::move(lexer));
    int result = interp.expr();

    std::cout << result << std::endl;
}