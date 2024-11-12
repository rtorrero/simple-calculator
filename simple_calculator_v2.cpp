/*
	simple_calculator_v2.cpp - Simple calculator (second version)

  This program implements a basic expression calculator.
  Input from cin, output from cout.
  The gramman for input is:

  Statement:
    Declaration
    Assignment
    Expression
    Print
    Help
    Quit

  Print:
    ;

  Help:
    help
    
  Quit:
    quit

  Declaration:
    let Name = Expression

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
    Number
    Name
    ( Expression )
    - Primary
    + Primary

  Number:
    floating-point-literal

  Name:
    a string of letters and numbers
  
  Input comes from cin through the Token_stream called ts.
*/

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <cmath>

using namespace std;

#define DEBUG_FUNC false

inline void error(const string& s)
{
	throw runtime_error(s);
}

inline void error(const string& s, const string& s2) { error(s+s2); }

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
  cout << "= 3" << endl;
  cout << "Longer expressions are supported:" << endl;
  cout << "1+2*3;" << endl;
  cout << "1+2*3/4;" << endl;
  cout << "Variables can be declared and updated:" << endl;
  cout << "let a = 1;" << endl;
  cout << "= 1" << endl;
  cout << "set a = 2;" << endl;
  cout << "= 2" << endl;
  cout << "To exit the calculator, type 'quit' and press enter." << endl;
}

struct Token 
{
  char kind;
  double value;
  string name;
  Token(char ch) :kind(ch), value(0) { }
  Token(char ch, double val) :kind(ch), value(val) { }
  Token(char ch, string val) :kind(ch), name(val) { }
};

class Token_stream 
{ 
  private:

    bool full; 
    Token buffer; 
    
  public: 
    
    Token_stream() :full(false), buffer(0) { } 
    Token get(); 
    void unget(Token t) { buffer=t; full=true; } 
    void ignore(char);
};

const char let = 'L';
const char set = 'S';
const char help = 'H';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';

Token Token_stream::get()
{
  if (full) { full=false; return buffer; }
  char ch;
  //cin >> ch;
  do { cin.get(ch); } while(isspace(ch));
  switch (ch) 
  {
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/':
    case ';':
    case '=': 
    case '%':
      return Token(ch);

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
      return Token(number,val);
    }
    default:
    	if (isalpha(ch)) 
      {
        string s;
        s+=ch;
        while(cin.get(ch) && (isalpha(ch) || isdigit(ch))) s+=ch;
        cin.unget();
        if (s == "let") return Token(let);	
        if (s == "set") return Token(set);
        if (s == "quit") return Token(quit);
        if (s == "help") return Token(help);
        return Token(name,s);
    	}
    	error("Bad token");
  }
}

void Token_stream::ignore(char c)
{
  if (full && c==buffer.kind) 
  {
    full = false;
    return;
  }
  full = false;

  char ch;
  while (cin>>ch)
    if (ch==c) return;
}

struct Variable 
{
  string name;
  double value;
  Variable(string n, double v) :name(n), value(v) { }
};

vector<Variable> names;	

double get_value(string s)
{
  for (int i = 0; i<names.size(); ++i)
    if (names[i].name == s) return names[i].value;
  error("get: undefined name ",s);
}

void set_value(string s, double d)
{
  for (int i = 0; i<=names.size(); ++i)
    if (names[i].name == s) 
    {
      names[i].value = d;
      return;
    }
  error("set: undefined name ",s);
}

bool is_declared(string s)
{
  for (int i = 0; i<names.size(); ++i)
    if (names[i].name == s) return true;
  return false;
}

void define_name(string s, double d)
{
  names.push_back(Variable(s,d));
}

Token_stream ts;

double expression();

double primary()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t = ts.get();
  switch (t.kind) 
  {
    case '(':
    {	
      double d = expression();
      t = ts.get();
      if (t.kind != ')') error("'(' expected");
      return d;
    }
    case '-':
      return - primary();
    case '+':
      return primary();
    case number:
      return t.value;
    case name:
      return get_value(t.name);
    default:
      error("primary expected");
  }
}

double term()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  double left = primary();
  while(true) 
  {
    Token t = ts.get();
    switch(t.kind) 
    {
      case '*':
        left *= primary();
        break;
      case '/':
        {
          double d = primary();
          if (d == 0) error("divide by zero");
          left /= d;
          break;
        }
      case '%':
        {
          double d = primary();
          if (d == 0) error("divide by zero");
          left = fmod(left, d);
          break;
        }
      default:
        ts.unget(t);
        return left;
    }
  }
}

double expression()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  double left = term();
  while(true) 
  {
    Token t = ts.get();
    switch(t.kind) 
    {
      case '+':
        left += term();
        break;
      case '-':
        left -= term();
        break;
      default:
        ts.unget(t);
        return left;
    }
  }
}

double declaration()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t = ts.get();
  if (t.kind != name) error ("name expected in declaration");
  string name = t.name;
  if (is_declared(name)) error(name, " declared twice");
  Token t2 = ts.get();
  if (t2.kind != '=') error("= missing in declaration of " ,name);
  double d = expression();
  names.push_back(Variable(name,d));
  return d;
}

double assignment()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC

  Token t = ts.get();
  if (t.kind != name) error ("name expected in assignment");
  string name = t.name;
  if (!is_declared(name)) error(name, " undeclared");
  Token t2 = ts.get();
  if (t2.kind != '=') error("= missing in declaration of " ,name);
  double d = expression();
  set_value(name,d);
  return d;
}

double statement()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t = ts.get();

  switch(t.kind) 
  {
    case let:
      return declaration();
    
    case set:
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
         
	ts.ignore(print);
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
    while (t.kind == print) t=ts.get();
    if (t.kind == quit) return;
    if (t.kind == help) {
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
