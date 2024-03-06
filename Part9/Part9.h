// implement simple parscal statement

/*
 * BEGIN
 *     BEGIN
 *         number := 2;
 *         a := number;
 *         b := 10 * a + 10 * number / 4;
 *         c := a - - b
 *     END;
 *     x := 11;
 * END.
 */

/*
 *    program : compound_statement DOT
 *
 *    compound_statement : BEGIN statement_list END
 *
 *    statement_list : statement
 *                   | statement SEMI statement_list
 *
 *    statement : compound_statement
 *              | assignment_statement
 *              | empty
 *
 *    assignment_statement : variable ASSIGN expr
 *
 *    empty :
 *
 *    expr: term ((PLUS | MINUS) term)*
 *
 *    term: factor ((MUL | DIV) factor)*
 *
 *    factor : PLUS factor
 *           | MINUS factor
 *           | INTEGER
 *           | LPAREN expr RPAREN
 *           | variable
 *
 *    variable: ID
 */
#include <string>
#include <unordered_map>
#include <list>

/*
* Token types
* EOF (end-of-file) token is used to indicate that
* there is no more input left for lexical analysis
*/
enum class TokenType {
    INTEGER,
    PLUS,
    MINUS,
    MUL,
    DIV,
    LParen,
    RParen,
    Assign,
    Semi,
    Dot,
    ID,
    Begin,
    End,
    TYPE_EOF,
};

class BinOp;
class UnaryOp;
class Num;
class Compound;
class Assign;
class Var;
class NoOp;

class Token {
 public:
    Token(TokenType type, std::string value) : type_(type), value_(value) {}

    friend std::ostream& operator<<(std::ostream& os, const Token& tk);

    TokenType type_;
    std::string value_;
};

class Lexer {
 public:
    explicit Lexer(std::string&& text);
    void advance();
    char* peek();
    void skipWhiteSpace();
    std::string integer();
    void error();

    /*
    * Handle identifiers and reserved keywords
    */
    Token _id();

    /*
    * Lexical analyzer (also known as scanner or tokenizer)
    * This method is responsible for breaking a sentence
    * apart into tokens. One token at a time.
    * */
    Token getNextToken();

 private:   
    char* textStart_ = nullptr;
    char* textEnd_ = nullptr;
    char* currentPtr_ = nullptr; 

    std::unordered_map<std::string, Token> RESERVED_KEYWORDS;
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
    virtual int visit(UnaryOp& uo) { assert(0); }

    virtual void visit(Compound& cp) { assert(0); }
    virtual void visit(Assign& as) { assert(0); }
    virtual int visit(Var& var) { assert(0); }
    virtual void visit(NoOp& noop) { assert(0); }
};

class AST {
 public:
    virtual int accept(NodeVisitor& visitor) = 0;
};

class Compound : public AST {
public:
    int accept(NodeVisitor& visitor) override {
        visitor.visit(*this);
        return -1;
    }
    std::list<AST*> children_;
};

class Assign : public AST {
 public:
    Assign(Var* left, Token& tk, AST* right)
                : op_(tk), left_(left), right_(right) {}
    int accept(NodeVisitor& visitor) override {
        visitor.visit(*this);
        return -1;
    }
    Token op_;
    Var* left_ = nullptr;
    AST* right_ = nullptr;
};

/*
* The Var node is constructed out of ID token.
*/
class Var : public AST {
 public:
    Var(Token& tk) : token_(tk), value_(tk.value_) {}
    int accept(NodeVisitor& visitor) override {
        return visitor.visit(*this);
    }
    Token token_;
    std::string value_;
};

class NoOp : public AST {
 public:
    int accept(NodeVisitor& visitor) override {
        visitor.visit(*this);
        return -1;
    }
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

class UnaryOp : public AST {
 public:
    UnaryOp(Token& op, AST* expr) : op_(op), expr_(expr) {}
    int accept(NodeVisitor& visitor) override {
        return visitor.visit(*this);
    }
    Token op_;
    AST* expr_;
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
    Parser(std::unique_ptr<Lexer>&& lexer);

    void error() {
        throw std::runtime_error("Invalid syntax");
    }

    /*
    * compare the current token type with the passed token
    * type and if they match then "eat" the current token
    * and assign the next token to the self.current_token,
    * otherwise raise an exception.
    */
    void eat(TokenType tktype);

    /*
    * program : compound_statement DOT
    */
    AST* program();

    /*
    * compound_statement: BEGIN statement_list END
    */
    Compound* compoundStatement();

    /*
    * statement_list : statement
    *               | statement SEMI statement_list
    */
    std::list<AST*> statementList();

    /*
    *     statement : compound_statement
    *          | assignment_statement
    *          | empty
    */
    AST* statement();

    /*
    * assignment_statement : variable ASSIGN expr
    */
    AST* assignmentStatement();

    /*
    * variable : ID
    */
    Var* variable();

   /*
   * An empty production
   */
    AST* empty();

    /*
    * Return an INTEGER token value.
    * factor : PLUS  factor
    *         | MINUS factor
    *         | INTEGER
    *         | LPAREN expr RPAREN
    *         | variable
    */
    AST* factor();

    /*
    * term : factor ((MUL | DIV) factor)*
    */
    AST* term();

    /*
    * Arithmetic expression parser / interpreter.
    *    expr   : term ((PLUS | MINUS) term)*
    *    term   : factor ((MUL | DIV) factor)*
    *    factor : (PLUS | MINUS) factor | INTEGER | LParen expr RParen
    */
    AST* expr();

    AST* parse() {
        AST* node = program();
        if (currentToken_.type_ != TokenType::TYPE_EOF) {
            error();
        }
        return node;
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
    int visit(BinOp& bo) override;
    int visit(UnaryOp& uo) override;
    int visit(Num& num) override;
    void visit(Compound& comp) override;
    void visit(Assign& as) override;
    int visit(Var& var) override;
    void  visit(NoOp& noop) override;


    int interpret() {
        AST* tree = parser_->parse();
        if (tree == nullptr) {
            return -1;
        }
        return tree->accept(*this);
    }

    void printGlobalScope();

 private:
    std::unique_ptr<Parser> parser_;

    std::unordered_map<std::string, int> GLOBAL_SCOPE;
};