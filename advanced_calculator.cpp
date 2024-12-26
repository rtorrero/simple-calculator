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

#include "matrix.hpp"
#include "vector.hpp"
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>

using namespace std;

#define DEBUG_FUNC false

inline void error(const string &s) { throw runtime_error(s); }

inline void error(const string &s, const string &s2) { error(s + s2); }

void print_help() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  cout << "This is a simple calculator." << endl;
  cout << "It can do addition (+), subtraction (-), multiplication (*), "
          "division (/), and modulus (%)."
       << endl;
  cout << "Note: All expressions should be terminated with a semicolon." << endl
       << endl;
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
  cout << "show myenv; - displays all variables stored in file 'myenv'" << endl
       << endl;
  cout << "To exit the calculator, type 'quit' and press enter." << endl;
}

enum class TokenKind {
  let,
  constant,
  set,
  help,
  quit,
  print,
  number,
  name,
  save,
  load,
  show,
  left_paren,
  right_paren,
  plus,
  minus,
  times,
  divide,
  mod,
  assign,
  comma,
  unary_math_func,
  binary_math_func,
  vector_start,
  vector_end,
  matrix_start,
  matrix_end,
  transpose
};

// Mapping of math functions that use a single argument
std::unordered_map<std::string, std::function<double(double)>> unary_funcs = {
    {"sin", static_cast<double (*)(double)>(std::sin)},
    {"cos", static_cast<double (*)(double)>(std::cos)},
    {"tan", static_cast<double (*)(double)>(std::tan)},
    {"asin", static_cast<double (*)(double)>(std::asin)},
    {"acos", static_cast<double (*)(double)>(std::acos)},
    {"atan", static_cast<double (*)(double)>(std::atan)},
    {"exp", static_cast<double (*)(double)>(std::exp)},
    {"ln", static_cast<double (*)(double)>(std::log)},
    {"log2", static_cast<double (*)(double)>(std::log2)},
    {"log10", static_cast<double (*)(double)>(std::log10)},
};

// Mapping of math functions that use two arguments
std::unordered_map<std::string, std::function<double(double, double)>>
    binary_funcs = {{"pow", [](double x, double y) { return std::pow(x, y); }}};

struct Token {
  TokenKind kind;
  double value;
  string name;
  mat_lib::vector<double> vec_value;
  mat_lib::matrix<double> mat_value;
  bool is_vector;
  bool is_matrix;

  Token(TokenKind k) : kind(k), value(0), is_vector(false), is_matrix(false) {}
  Token(TokenKind k, double val)
      : kind(k), value(val), is_vector(false), is_matrix(false) {}
  Token(TokenKind k, string val)
      : kind(k), name(val), is_vector(false), is_matrix(false) {}
  Token(TokenKind k, mat_lib::vector<double> vec)
      : kind(k), vec_value(vec), is_vector(true), is_matrix(false) {}
  Token(TokenKind k, mat_lib::matrix<double> mat)
      : kind(k), mat_value(mat), is_vector(false), is_matrix(true) {}
};

class Token_stream {
private:
  bool full;
  Token buffer;

public:
  Token_stream() : full(false), buffer(TokenKind::quit) {}
  Token get();
  void unget(Token t) {
    buffer = t;
    full = true;
  }
  void ignore(TokenKind);
};

Token Token_stream::get() {
  if (full) {
    full = false;
    return buffer;
  }
  char ch;
  // cin >> ch;
  do {
    cin.get(ch);
  } while (isspace(ch));
  switch (ch) {
  case '[': {
    char next = cin.get();
    if (next == '[') {
      return Token(TokenKind::matrix_start);
    }
    cin.unget();
    return Token(TokenKind::vector_start);
  }
  case ']': {
    char next = cin.get();
    if (next == ']') {
      return Token(TokenKind::matrix_end);
    }
    cin.unget();
    return Token(TokenKind::vector_end);
  }
  case '~':
    return Token(TokenKind::transpose);
  case '(':
    return Token(TokenKind::left_paren);
  case ')':
    return Token(TokenKind::right_paren);
  case '+':
    return Token(TokenKind::plus);
  case '-':
    return Token(TokenKind::minus);
  case '*':
    return Token(TokenKind::times);
  case '/':
    return Token(TokenKind::divide);
  case ';':
    return Token(TokenKind::print);
  case '=':
    return Token(TokenKind::assign);
  case '%':
    return Token(TokenKind::mod);
  case ',':
    return Token(TokenKind::comma);

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
  case '9': {
    cin.unget();
    double val;
    cin >> val;
    return Token(TokenKind::number, val);
  }
  default:
    if (isalpha(ch)) {
      string s;
      s += ch;
      while (cin.get(ch) && (isalpha(ch) || isdigit(ch)))
        s += ch;
      cin.unget();
      if (s == "let")
        return Token(TokenKind::let);
      if (s == "const")
        return Token(TokenKind::constant);
      if (s == "set")
        return Token(TokenKind::set);
      if (s == "quit")
        return Token(TokenKind::quit);
      if (s == "help")
        return Token(TokenKind::help);
      if (s == "save")
        return Token(TokenKind::save);
      if (s == "load")
        return Token(TokenKind::load);
      if (s == "show")
        return Token(TokenKind::show);
      if (unary_funcs.contains(s))
        return Token(TokenKind::unary_math_func, s);
      if (binary_funcs.contains(s))
        return Token(TokenKind::binary_math_func, s);
      return Token(TokenKind::name, s);
    }
    error("Bad token");
  }
}

mat_lib::vector<double> parse_vector() {
  vector<double> values;
  Token t = ts.get();
  while (t.kind != TokenKind::vector_end) {
    if (t.kind == TokenKind::number) {
      values.push_back(t.value);
    }
    t = ts.get();
    if (t.kind != TokenKind::comma && t.kind != TokenKind::vector_end) {
      error("Expected ',' or ']' in vector");
    }
  }
  return mat_lib::vector<double>(values);
}

mat_lib::matrix<double> parse_matrix() {
  vector<vector<double>> rows;
  Token t = ts.get();
  while (t.kind != TokenKind::matrix_end) {
    if (t.kind == TokenKind::vector_start) {
      vector<double> row;
      t = ts.get();
      while (t.kind != TokenKind::vector_end) {
        if (t.kind == TokenKind::number) {
          row.push_back(t.value);
        }
        t = ts.get();
        if (t.kind != TokenKind::comma && t.kind != TokenKind::vector_end) {
          error("Expected ',' or ']' in matrix row");
        }
      }
      rows.push_back(row);
    }
    t = ts.get();
    if (t.kind != TokenKind::comma && t.kind != TokenKind::matrix_end) {
      error("Expected ',' or ']]' in matrix");
    }
  }
  return mat_lib::matrix<double>(rows);
}

void Token_stream::ignore(TokenKind kind) {
  if (full && kind == buffer.kind) {
    full = false;
    return;
  }
  full = false;

  while (true) {
    Token t = get(); // This already handles char-to-Token conversion
    if (t.kind == kind)
      return;
  }
}

struct Variable {
  double scalar_value;
  mat_lib::vector<double> vec_value;
  mat_lib::matrix<double> mat_value;
  bool is_const;
  bool is_vector;
  bool is_matrix;

  Variable()
      : scalar_value(0), is_const(false), is_vector(false), is_matrix(false) {}
  Variable(double v, bool c = false)
      : scalar_value(v), is_const(c), is_vector(false), is_matrix(false) {}
  Variable(mat_lib::vector<double> v, bool c = false)
      : vec_value(v), is_const(c), is_vector(true), is_matrix(false) {}
  Variable(mat_lib::matrix<double> m, bool c = false)
      : mat_value(m), is_const(c), is_vector(false), is_matrix(true) {}
};

map<string, Variable> names;

double get_value(string s) {
  auto it = names.find(s);
  if (it == names.end()) {
    error("get: undefined name ", s);
  }
  return it->second.value;
}

// set_value assumes the key exists
void set_value(string s, double d) {
  Variable &var = names.at(s);
  if (var.is_const) {
    error("set: cannot update constant ", s);
  }
  var.value = d;
}

bool is_declared(string s) {
  return names.contains(s); // C++20
}

// define_name will overwrite a variable if it already exists
void define_name(string s, Token t, bool is_const) {
  if (t.is_vector) {
    names[s] = Variable(t.vec_value, is_const);
  } else if (t.is_matrix) {
    names[s] = Variable(t.mat_value, is_const);
  } else {
    names[s] = Variable(t.value, is_const);
  }
}

Token_stream ts;

double expression();

double primary() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  Token t = ts.get();
  switch (t.kind) {
  case TokenKind::vector_start:
    return Token(TokenKind::vector, parse_vector());
  case TokenKind::matrix_start:
    return Token(TokenKind::matrix, parse_matrix());
  case TokenKind::transpose: {
    Token operand = primary();
    if (operand.is_matrix) {
      return Token(TokenKind::matrix, operand.mat_value.make_transpose());
    }
    error("Transpose operator requires matrix operand");
  }
  case TokenKind::left_paren: {
    double d = expression();
    t = ts.get();
    if (t.kind != TokenKind::right_paren)
      error("'(' expected");
    return d;
  }
  case TokenKind::minus:
    return -primary();
  case TokenKind::plus:
    return primary();
  case TokenKind::number:
    return t.value;
  case TokenKind::name:
    return get_value(t.name);
  case TokenKind::unary_math_func: {
    Token next = ts.get();
    next = ts.get();
    if (next.kind != TokenKind::left_paren)
      error("'(' expected after function name");

    double arg = expression();

    t = ts.get();
    if (next.kind != TokenKind::right_paren)
      error("')' expected");

    return unary_funcs.at(t.name)(arg);
  }
  case TokenKind::binary_math_func: {
    Token next = ts.get();
    if (next.kind != TokenKind::left_paren)
      error("'(' expected after function name");

    double arg1 = expression();

    next = ts.get();
    if (next.kind != TokenKind::comma)
      error("',' expected between arguments");

    double arg2 = expression();

    next = ts.get();
    if (next.kind != TokenKind::right_paren)
      error("')' expected");

    return binary_funcs.at(t.name)(arg1, arg2);
  }
  default:
    error("primary expected");
  }
}

Token term() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  Token left = primary();
  while (true) {
    Token t = ts.get();
    switch (t.kind) {
    case TokenKind::times: {
      Token right = primary();
      // Matrix * Matrix
      if (left.is_matrix && right.is_matrix) {
        left.mat_value = left.mat_value * right.mat_value;
        continue;
      }
      // Matrix * Vector
      if (left.is_matrix && right.is_vector) {
        left.vec_value = left.mat_value * right.vec_value;
        left.is_matrix = false;
        left.is_vector = true;
        continue;
      }
      // Vector * Matrix
      if (left.is_vector && right.is_matrix) {
        left.vec_value = left.vec_value * right.mat_value;
        continue;
      }
      // Vector * Scalar or Scalar * Vector
      if (left.is_vector && !right.is_vector && !right.is_matrix) {
        left.vec_value = left.vec_value * right.value;
        continue;
      }
      if (!left.is_vector && !left.is_matrix && right.is_vector) {
        left.vec_value = right.vec_value * left.value;
        left.is_vector = true;
        continue;
      }
      // Matrix * Scalar or Scalar * Matrix
      if (left.is_matrix && !right.is_vector && !right.is_matrix) {
        left.mat_value = left.mat_value * right.value;
        continue;
      }
      if (!left.is_vector && !left.is_matrix && right.is_matrix) {
        left.mat_value = right.mat_value * left.value;
        left.is_matrix = true;
        continue;
      }
      // Regular scalar multiplication
      if (!left.is_vector && !left.is_matrix && !right.is_vector &&
          !right.is_matrix) {
        left.value *= right.value;
        continue;
      }
      error("Invalid multiplication operands");
    }
    case TokenKind::divide: {
      Token right = primary();
      if (right.is_vector || right.is_matrix) {
        error("Cannot divide by vector or matrix");
      }
      if (right.value == 0) {
        error("divide by zero");
      }
      if (left.is_vector) {
        left.vec_value = left.vec_value / right.value;
      } else if (left.is_matrix) {
        left.mat_value = left.mat_value / right.value;
      } else {
        left.value /= right.value;
      }
      continue;
    }
    case TokenKind::mod: {
      if (left.is_vector || left.is_matrix) {
        error("Modulo operation not defined for vectors or matrices");
      }
      Token right = primary();
      if (right.is_vector || right.is_matrix) {
        error("Modulo operation not defined for vectors or matrices");
      }
      if (right.value == 0) {
        error("divide by zero");
      }
      left.value = fmod(left.value, right.value);
      continue;
    }
    default:
      ts.unget(t);
      return left;
    }
  }
}

Token expression() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  Token left = term();
  while (true) {
    Token t = ts.get();
    switch (t.kind) {
    case TokenKind::plus: {
      Token right = term();
      if (left.is_vector && right.is_vector) {
        left.vec_value += right.vec_value;
      } else if (left.is_matrix && right.is_matrix) {
        left.mat_value += right.mat_value;
      } else if (!left.is_vector && !left.is_matrix && !right.is_vector &&
                 !right.is_matrix) {
        left.value += right.value;
      } else {
        error("Invalid addition operands");
      }
      break;
    }
    case TokenKind::minus: {
      // Similar to plus case
      Token right = term();
      if (left.is_vector && right.is_vector) {
        left.vec_value -= right.vec_value;
      } else if (left.is_matrix && right.is_matrix) {
        left.mat_value -= right.mat_value;
      } else if (!left.is_vector && !left.is_matrix && !right.is_vector &&
                 !right.is_matrix) {
        left.value -= right.value;
      } else {
        error("Invalid subtraction operands");
      }
      break;
    }
    default:
      ts.unget(t);
      return left;
    }
  }
}

double declaration(bool is_const = false) {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != TokenKind::name)
    error("name expected in declaration");
  string name = t.name;
  if (is_declared(name))
    error(name, " declared twice");
  Token t2 = ts.get();
  if (t2.kind != TokenKind::assign)
    error("= missing in declaration of ", name);
  Token result = expression();
  define_name(name, result, is_const);
  return result;
}

double assignment() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != TokenKind::name)
    error("name expected in assignment");
  string name = t.name;
  if (!is_declared(name))
    error(name, " undeclared");
  Token t2 = ts.get();
  if (t2.kind != TokenKind::assign)
    error("= missing in declaration of ", name);
  double d = expression();
  set_value(name, d);
  return d;
}

// It wasn't entirely clear if env was a name for a particular env-file or a
// subcommand In this implementation we have assumed it's a name.
void save_state() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != TokenKind::name)
    error("env filename expected");
  string name = t.name;
  ofstream file(name);

  for (const auto &[var_name, var] : names) {
    file << var_name << " ";
    if (var.is_vector) {
      file << "vector " << var.vec_value;
    } else if (var.is_matrix) {
      file << "matrix " << var.mat_value;
    } else {
      file << "scalar " << var.scalar_value;
    }
    file << " " << var.is_const << "\n";
  }

  file.close();
}

void load_state() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != TokenKind::name)
    error("env filename expected");
  string name = t.name;
  ifstream file(name);

  if (!file)
    error("cannot open file ", name);

  string var_name, type;
  bool is_const;

  while (file >> var_name >> type) {
    if (type == "scalar") {
      double value;
      file >> value >> is_const;
      define_name(var_name, value, is_const);
    } else if (type == "vector") {
      mat_lib::vector<double> vec;
      file >> vec >> is_const;
      define_name(var_name, vec, is_const);
    } else if (type == "matrix") {
      mat_lib::matrix<double> mat;
      file >> mat >> is_const;
      define_name(var_name, mat, is_const);
    }
  }

  file.close();
}

void show_state() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != TokenKind::name)
    error("env filename expected");
  string name = t.name;
  ifstream file(name);

  if (!file)
    error("cannot open file ", name);

  string var_name;
  double value;
  bool is_const;

  cout << "Variables in environment '" << name << "':\n";
  cout << "----------------------------------------\n";
  while (file >> var_name >> value >> is_const) {
    cout << (is_const ? "const " : "let ") << var_name << " = " << value
         << "\n";
  }
  cout << "----------------------------------------\n";

  file.close();
}

Token statement() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  Token t = ts.get();
  switch (t.kind) {
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

void clean_up_mess() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  ts.ignore(TokenKind::print);
}

const string prompt = "> ";
const string result = "= ";

void calculate() {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  while (true)
    try {
      cout << prompt;
      Token t = ts.get();
      while (t.kind == TokenKind::print)
        t = ts.get();

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
      if (t.kind == TokenKind::quit)
        return;
      if (t.kind == TokenKind::help) {
        print_help();
        continue;
      }
      ts.unget(t);

      auto the_result = statement();
      cout << result << the_result << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
      clean_up_mess();
    }
}

int main() try {
#if DEBUG_FUNC
  cout << __func__ << std::endl;
#endif // DEBUG_FUNC

  calculate();
  return 0;
} catch (exception &e) {
  cerr << "exception: " << e.what() << endl;
  char c;
  while (cin >> c && c != ';')
    ;
  return 1;
} catch (...) {
  cerr << "exception\n";
  char c;
  while (cin >> c && c != ';')
    ;
  return 2;
}
