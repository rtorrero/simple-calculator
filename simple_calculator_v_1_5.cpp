/*
	simple_calculator.cpp - Simple calculator

  This program implements a basic expression calculator.
  Input from cin, output from cout.
  The grammar for input is:

  Statement:
    Expression
    Print
    Quit

  Print:
    ;
    
  Quit:
    quit

  Expression:
    Term
    + Expression
    - Expression
    Term + Expression
    Term - Expression

  Term:
    Primary
    Primary * Term 
    Primary / Term 

  Primary:
    Number
    ( Expression )
  
  Input comes from cin through the Token_stream called ts.
*/

#include <iostream>
#include <sstream>
#include<string>
#include<stdexcept>
using namespace std;

#define DEBUG_FUNC false

inline void error(const string& s)
{
	throw runtime_error(s);
}

struct Token 
{
  char kind;
  double value;
  Token(char ch) :kind(ch), value(0) { }
  Token(char ch, double val) :kind(ch), value(val) { }
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

const char quit = 'Q';
const char print = ';';
const char number = '8';

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
        if (s == "quit") return Token(quit);
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
  //while (cin>>ch)
  while (cin.get(ch))
    if (ch==c) return;
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
      if (t.kind != ')') error("')' expected");
      return d;
    }

    //case '-':
    //  return - primary();
    //case '+':
    //  return primary();
    
    case number:
      return t.value;
      
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
  
  Token t = ts.get();
  double left=(
    (t.kind=='-')? 
      -term(): 
      (
        (t.kind=='+')? 
          term(): 
          (ts.unget(t), term())
      )
  );

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

double statement()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  return expression();
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
