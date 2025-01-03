/*
	simple_calculator_v2.cpp - Simple calculator (second version)

  This program implements a basic expression calculator.
  Input from cin, output from cout.
  The grammar for input is:

  Command:
    Help
    Save
    Load
    Show
    Quit

  Save:
    save Name

  Load:
    load Name

  Show:
    show Name

  Help:
    help
    
  Quit:
    quit

  Statement:
    Declaration
    Assignment
    Expression
    Print

  Print:
    ;

  Declaration:
    let Name = Expression
    const Name = Expression

  Assignment:
    set Name = Expression

  Expression:
    Term
    Term + Expression
    Term - Expression

  Term:
    Primary
    Primary * Term
    Primary / Term
    Primary % Term

  Primary:
    Function
    Number
    Name
    ( Expression )
    - Primary
    + Primary

  Function:
    FunctionName ( Expression )
    pow ( Expression , Expression )
  
  FunctionName :
    sin
    cos
    tan
    asin
    acos
    atan
    exp
    ln
    log10
    log2

  Number:
    floating-point-literal

  Name:
    a string of letters and numbers
  
  Input comes from cin through the Token_stream called ts.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <map>
#include <unordered_map>
#include <functional>
#include <cmath>
#include "vector.hpp"
#include "matrix.hpp"

using namespace std;
using matrix_t=mat_lib::matrix<double>;
using vector_t=mat_lib::vector<double>;

#define DEBUG_FUNC false

inline void error(const string& s)
{
	throw runtime_error(s);
}

inline void error(const string& s, const string& s2) { error(s+s2); }

struct Value {
    enum class Type { Number, Vector, Matrix };
    Type type;
    double number;
    vector_t vector;
    matrix_t matrix;
    
    Value(double d) : type(Type::Number), number(d) {}
    Value(vector_t v) : type(Type::Vector), vector(v) {}
    Value(matrix_t m) : type(Type::Matrix), matrix(m) {}
};

Value operator+(const Value& a, const Value& b) {
    if (a.type == Value::Type::Number && b.type == Value::Type::Number)
        return Value(a.number + b.number);
    if (a.type == Value::Type::Vector && b.type == Value::Type::Vector)
        return Value(a.vector + b.vector);
    if (a.type == Value::Type::Matrix && b.type == Value::Type::Matrix)
        return Value(a.matrix + b.matrix);
    error("invalid operand types for +");
}

Value operator-(const Value& a, const Value& b) {
    if (a.type == Value::Type::Number && b.type == Value::Type::Number)
        return Value(a.number - b.number);
    if (a.type == Value::Type::Vector && b.type == Value::Type::Vector)
        return Value(a.vector - b.vector);
    if (a.type == Value::Type::Matrix && b.type == Value::Type::Matrix)
        return Value(a.matrix - b.matrix);
    error("invalid operand types for -");
}

Value operator*(const Value& a, const Value& b) {
    if (a.type == Value::Type::Number && b.type == Value::Type::Number)
        return Value(a.number * b.number);
    if (a.type == Value::Type::Vector && b.type == Value::Type::Number)
        return Value(a.vector * b.number);
    if (a.type == Value::Type::Number && b.type == Value::Type::Vector)
        return Value(b.vector * a.number);
    if (a.type == Value::Type::Matrix && b.type == Value::Type::Number)
        return Value(a.matrix * b.number);
    if (a.type == Value::Type::Number && b.type == Value::Type::Matrix)
        return Value(b.matrix * a.number);
    if (a.type == Value::Type::Matrix && b.type == Value::Type::Vector)
        return Value(a.matrix * b.vector);
    if (a.type == Value::Type::Matrix && b.type == Value::Type::Matrix)
        return Value(a.matrix * b.matrix);
    if (a.type == Value::Type::Vector && b.type == Value::Type::Vector)
        return Value(dot(a.vector, b.vector));
    error("invalid operand types for *");
}

Value operator/(const Value& a, const Value& b) {
    if (a.type == Value::Type::Number && b.type == Value::Type::Number) {
        if (b.number == 0) error("divide by zero");
        return Value(a.number / b.number);
    }
    if (a.type == Value::Type::Vector && b.type == Value::Type::Number) {
        if (b.number == 0) error("divide by zero");
        return Value(a.vector / b.number);
    }
    error("invalid operand types for /");
}

Value operator%(const Value& a, const Value& b) {
    if (a.type == Value::Type::Number && b.type == Value::Type::Number) {
        if (b.number == 0) error("divide by zero");
        return Value(fmod(a.number, b.number));
    }
    error("invalid operand types for %");
}

// Unary operators
Value operator-(const Value& a) {
    switch(a.type) {
        case Value::Type::Number:
            return Value(-a.number);
        case Value::Type::Vector:
            return Value(-a.vector);
        case Value::Type::Matrix:
            return Value(-a.matrix);
    }
    error("invalid operand type for unary -");
}

Value operator+(const Value& a) {
    return a; // Unary plus just returns the value
}

// Comparison operators
bool operator==(const Value& a, const Value& b) {
    if (a.type != b.type) return false;
    
    switch(a.type) {
        case Value::Type::Number:
            return a.number == b.number;
        case Value::Type::Vector:
            return a.vector == b.vector;
        case Value::Type::Matrix:
            return a.matrix == b.matrix;
    }
    return false;
}

bool operator!=(const Value& a, const Value& b) {
    return !(a == b);
}

// << operator for Value
std::ostream& operator<<(std::ostream& os, const Value& v) {
    switch(v.type) {
        case Value::Type::Number:
            os << v.number;
            break;
        case Value::Type::Vector:
            os << v.vector;
            break;
        case Value::Type::Matrix:
            os << v.matrix;
            break;
    }
    return os;
}

void print_help()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC

  cout << "This is a simple calculator." << endl;
  cout << "It can do addition (+), subtraction (-), multiplication (*), division (/), and modulus (%)." << endl;
  cout << "Note: All expressions should be terminated with a semicolon." << endl << endl;
  cout << "Supported syntax:" << endl;
  cout << "1+2;" << endl;
  cout << "= 3" << endl << endl;
  cout << "Longer expressions are supported:" << endl;
  cout << "1+2*3;" << endl;
  cout << "1+2*3/4;" << endl << endl;
  cout << "Variables can be declared and updated:" << endl;
  cout << "let a = 1;" << endl;
  cout << "= 1" << endl;
  cout << "set a = 2;" << endl;
  cout << "= 2" << endl << endl;
  cout << "Constants can be declared:" << endl;
  cout << "const b = 2;" << endl;
  cout << "= 2" << endl << endl;
  cout << "Constants cannot be updated:" << endl;
  cout << "set b = 3;" << endl;
  cout << "set: cannot update constant b" << endl << endl;
  cout << "Trigonometric and other math functions supported:" << endl;
  cout << "sin(x) - sine of x (x in radians)" << endl;
  cout << "cos(x) - cosine of x (x in radians)" << endl;
  cout << "tan(x) - tangent of x (x in radians)" << endl;
  cout << "asin(x) - arcsine of x (result in radians)" << endl;
  cout << "acos(x) - arccosine of x (result in radians)" << endl;
  cout << "atan(x) - arctangent of x (result in radians)" << endl;
  cout << "exp(x) - e^x" << endl;
  cout << "ln(x) - natural logarithm of x" << endl;
  cout << "log10(x) - base 10 logarithm of x" << endl;
  cout << "log2(x) - base 2 logarithm of x" << endl;
  cout << "pow(x, y) - x raised to the power of y" << endl << endl;
  cout << "Environment management:" << endl;
  cout << "save myenv; - saves all variables to file 'myenv'" << endl;
  cout << "load myenv; - loads all variables from file 'myenv'" << endl;
  cout << "show myenv; - displays all variables stored in file 'myenv'" << endl << endl;
  cout << "To exit the calculator, type 'quit' and press enter." << endl;
}

enum class TokenKind {
  let, constant, set, help, quit, print, number, name, save, load, show,
  left_paren, right_paren, plus, minus, times, divide, mod, assign,
  comma, unary_math_func, binary_math_func, bracket_open, bracket_close,
    braces_open, braces_close, vector_decl, matrix_decl
};

// Update the function maps to work with Value types
std::unordered_map<std::string, std::function<Value(Value)>> unary_funcs = {
    {"sin", [](Value v) { return Value(std::sin(v.number)); }},
    {"cos", [](Value v) { return Value(std::cos(v.number)); }},
    {"tan", [](Value v) { return Value(std::tan(v.number)); }},
    {"asin", [](Value v) { return Value(std::asin(v.number)); }},
    {"acos", [](Value v) { return Value(std::acos(v.number)); }},
    {"atan", [](Value v) { return Value(std::atan(v.number)); }},
    {"exp", [](Value v) { return Value(std::exp(v.number)); }},
    {"ln", [](Value v) { return Value(std::log(v.number)); }},
    {"log2", [](Value v) { return Value(std::log2(v.number)); }},
    {"log10", [](Value v) { return Value(std::log10(v.number)); }}
};

std::unordered_map<std::string, std::function<Value(Value, Value)>> binary_funcs = {
    {"pow", [](Value x, Value y) { return Value(std::pow(x.number, y.number)); }}
};

struct Token 
{
  TokenKind kind;
  double value;
  string name;
  Token(TokenKind k) :kind(k), value(0) { }
  Token(TokenKind k, double val) :kind(k), value(val) { }
  Token(TokenKind k, string val) :kind(k), name(val) { }
};

class Token_stream 
{ 
  private:

    bool full; 
    Token buffer; 
    
  public: 
    
    Token_stream() :full(false), buffer(TokenKind::quit) { } 
    Token get(); 
    void unget(Token t) { buffer=t; full=true; } 
    void ignore(TokenKind);
};

Token Token_stream::get()
{
  if (full) { full=false; return buffer; }
  char ch;
  //cin >> ch;
  do { cin.get(ch); } while(isspace(ch));
  switch (ch) 
  {
    case '(': return Token(TokenKind::left_paren);
    case ')': return Token(TokenKind::right_paren);
    case '+': return Token(TokenKind::plus);
    case '-': return Token(TokenKind::minus);
    case '*': return Token(TokenKind::times);
    case '/': return Token(TokenKind::divide);
    case ';': return Token(TokenKind::print);
    case '=': return Token(TokenKind::assign);
    case '%': return Token(TokenKind::mod);
    case ',': return Token(TokenKind::comma);
    case '[': return Token(TokenKind::bracket_open);
    case ']': return Token(TokenKind::bracket_close);
    case '{': return Token(TokenKind::braces_open);
    case '}': return Token(TokenKind::braces_close);

    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {	
      cin.unget();
      double val;
      cin>>val;
      return Token(TokenKind::number,val);
    }
    default:
    	if (isalpha(ch)) 
      {
        string s;
        s+=ch;
        while(cin.get(ch) && (isalpha(ch) || isdigit(ch))) s+=ch;
        cin.unget();
        if (s == "let") return Token(TokenKind::let);
        if (s == "const") return Token(TokenKind::constant);
        if (s == "set") return Token(TokenKind::set);
        if (s == "quit") return Token(TokenKind::quit);
        if (s == "help") return Token(TokenKind::help);
        if (s == "save") return Token(TokenKind::save);
        if (s == "load") return Token(TokenKind::load);
        if (s == "show") return Token(TokenKind::show);
        if (s == "vector") return Token(TokenKind::vector_decl);
        if (s == "matrix") return Token(TokenKind::matrix_decl);
        if (unary_funcs.contains(s)) return Token(TokenKind::unary_math_func, s);
        if (binary_funcs.contains(s)) return Token(TokenKind::binary_math_func, s);
        return Token(TokenKind::name,s);
    	}
    	error("Bad token");
  }
}

void Token_stream::ignore(TokenKind kind)
{
  if (full && kind == buffer.kind) 
  {
    full = false;
    return;
  }
  full = false;

  while (true) {
      Token t = get();  // This already handles char-to-Token conversion
      if (t.kind == kind) return;
  }
}

struct Variable {
    Value value;
    bool is_const;
    Variable() : value(0.0), is_const(false) {}
    Variable(Value v, bool c=false) : value(v), is_const(c) {}
};

map<string, Variable> names;

Value get_value(string s)
{
    auto it = names.find(s);
    if (it == names.end()) {
        error("get: undefined name ", s);
    }
    return it->second.value;
}

// set_value assumes the key exists
void set_value(string s, Value d)
{
  Variable& var = names.at(s);
  if (var.is_const) {
    error("set: cannot update constant ", s);
  }
  var.value = d;
}


bool is_declared(string s)
{
  return names.contains(s); // C++20
}

// define_name will overwrite a variable if it already exists
void define_name(string s, Value d, bool is_const)
{
  names[s] = Variable(d, is_const);
}

Token_stream ts;

Value expression();

Value parse_vector() {
    Token t = ts.get();
    if (t.kind != TokenKind::bracket_open) error("[ expected after vector");
    size_t dim = expression().number;
    t = ts.get();
    if (t.kind != TokenKind::bracket_close) error("] expected after dimension");
    t = ts.get();
    if (t.kind != TokenKind::braces_open) error("{ expected");
    
    vector_t result(dim);
    for (size_t i = 0; i < dim; ++i) {
        result[i] = expression().number;
        t = ts.get();
        if (i < dim-1 && t.kind != TokenKind::comma) error(", expected");
    }
    if (t.kind != TokenKind::braces_close) error("} expected");
    return Value(result);
}

Value parse_matrix() {
    Token t = ts.get();
    if (t.kind != TokenKind::bracket_open) error("[ expected after matrix");

    // Get first number directly as rows
    t = ts.get();
    if (t.kind != TokenKind::number) error("number expected for rows");
    size_t rows = t.value;
    
    // Check for dimension separator
    t = ts.get();
    if (t.kind != TokenKind::comma) error(", expected between dimensions");
    
    // Get second number directly as columns
    t = ts.get();
    if (t.kind != TokenKind::number) error("number expected for columns");
    size_t cols = t.value;

    t = ts.get();
    if (t.kind != TokenKind::bracket_close) error("] expected after dimensions");
    t = ts.get();
    if (t.kind != TokenKind::braces_open) error("{ expected");
    
    matrix_t result(rows, cols);
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[i][j] = expression().number;
            t = ts.get();
            if (j < cols-1 && t.kind != TokenKind::comma) error(", expected");
        }
        if (i < rows-1 && t.kind != TokenKind::print) {
            error("; expected between rows");
        }
    }
    if (t.kind != TokenKind::braces_close) error("} expected");
    return Value(result);
}

Value primary()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t = ts.get();
  switch (t.kind) 
  {
    case TokenKind::left_paren:
    {	
      Value d = expression();
      t = ts.get();
      if (t.kind != TokenKind::right_paren) error("'(' expected");
      return d;
    }
    case TokenKind::minus:
      return Value(-primary().number);
    case TokenKind::plus:
      return primary();
    case TokenKind::number:
      return t.value;
    case TokenKind::name:
      return get_value(t.name);
    case TokenKind::vector_decl:
      return parse_vector();
    case TokenKind::matrix_decl:
      return parse_matrix();
    case TokenKind::unary_math_func:
        {
          string func_name = t.name;
          Token next = ts.get();
          if (next.kind != TokenKind::left_paren) 
              error("'(' expected after function name");
          
          Value arg = expression();
          
          t = ts.get();
          if (t.kind != TokenKind::right_paren) 
              error("')' expected");
          
          return unary_funcs.at(func_name)(arg);
        }
    case TokenKind::binary_math_func:
        {
          string func_name = t.name;
          Token next = ts.get();
          if (next.kind != TokenKind::left_paren) 
              error("'(' expected after function name");
          
          Value arg1 = expression();
          
          next = ts.get();
          if (next.kind != TokenKind::comma) 
              error("',' expected between arguments");
              
          Value arg2 = expression();
          
          next = ts.get();
          if (next.kind != TokenKind::right_paren) 
              error("')' expected");
          
          return binary_funcs.at(func_name)(arg1, arg2);
        }              
    default:
      error("primary expected");
  }
}

Value term()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Value left = primary();
  while(true) 
  {
    Token t = ts.get();
    switch(t.kind) 
    {
      case TokenKind::times:
        left = left * primary();
        break;
      case TokenKind::divide:
        {
          Value d = primary();
          if (d == 0) error("divide by zero");
          left = left / primary();
          break;
        }
      case TokenKind::mod:
        {
          Value d = primary();
          if (d == 0) error("divide by zero");
          left = left % primary();
          break;
        }
      default:
        ts.unget(t);
        return left;
    }
  }
}

Value expression()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Value left = term();
  while(true) 
  {
    Token t = ts.get();
    switch(t.kind) 
    {
      case TokenKind::plus:
        left = left + term();
        break;
      case TokenKind::minus:
        left = left - term();
        break;
      default:
        ts.unget(t);
        return left;
    }
  }
}

Value declaration(bool is_const=false)
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t = ts.get();
  if (t.kind != TokenKind::name) error ("name expected in declaration");
  string name = t.name;
  if (is_declared(name)) error(name, " declared twice");
  Token t2 = ts.get();
  if (t2.kind != TokenKind::assign) error("= missing in declaration of " ,name);
  Value d = expression();
  define_name(name,d,is_const);
  return d;
}

Value assignment()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != TokenKind::name) error ("name expected in assignment");
  string name = t.name;
  if (!is_declared(name)) error(name, " undeclared");
  Token t2 = ts.get();
  if (t2.kind != TokenKind::assign) error("= missing in declaration of " ,name);
  Value d = expression();
  set_value(name,d);
  return d;
}

// It wasn't entirely clear if env was a name for a particular env-file or a subcommand
// In this implementation we have assumed it's a name.
void save_state()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != TokenKind::name) error ("env filename expected");
  string name = t.name;
  ofstream file(name);
  
  for(const auto& [var_name, var] : names) {
    file << var_name << " " << var.value << " " << var.is_const << "\n";
  }
  
  file.close();
}

void load_state()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != TokenKind::name) error ("env filename expected");
  string name = t.name;
  ifstream file(name);
  
  if (!file) error("cannot open file ", name);
  
  string var_name;
  double value;
  bool is_const;
  
  while (file >> var_name >> value >> is_const) {
    define_name(var_name, value, is_const);
  }
  
  file.close();
}

void show_state()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != TokenKind::name) error ("env filename expected");
  string name = t.name;
  ifstream file(name);
  
  if (!file) error("cannot open file ", name);
  
  string var_name;
  double value;
  bool is_const;
  
  cout << "Variables in environment '" << name << "':\n";
  cout << "----------------------------------------\n";
  while (file >> var_name >> value >> is_const) {
    cout << (is_const ? "const " : "let ") << var_name 
         << " = " << value << "\n";
  }
  cout << "----------------------------------------\n";
  
  file.close();
}

Value statement()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t = ts.get();

  switch(t.kind) 
  {
    case TokenKind::let:    
      return declaration();

    case TokenKind::constant:
      return declaration(true);
    
    case TokenKind::set:
      return assignment();
      
    default:
      ts.unget(t);
      return expression();
  }
}

void clean_up_mess()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
	ts.ignore(TokenKind::print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  while(true) 
  try 
  {
    cout << prompt;
    Token t = ts.get();
    while (t.kind == TokenKind::print) t=ts.get();

    if (t.kind == TokenKind::save) {
      save_state();
      continue;
    }
    if (t.kind == TokenKind::show) {
      show_state();
      continue;
    }
    if (t.kind == TokenKind::load) {
      load_state();
      continue;
    }
    if (t.kind == TokenKind::quit) return;
    if (t.kind == TokenKind::help) {
      print_help();
      continue;
    }
    ts.unget(t);

    auto the_result=statement();
    cout << result << the_result << endl;
  }
  catch(runtime_error& e) 
  {
    cerr << e.what() << endl;
    clean_up_mess();
  }
}

int main()
try 
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC

  calculate();
  return 0;
}
catch (exception& e) {
  cerr << "exception: " << e.what() << endl;
  char c;
  while (cin >>c && c!=';') ;
  return 1;
}
catch (...) {
  cerr << "exception\n";
  char c;
  while (cin>>c && c!=';');
  return 2;
}
