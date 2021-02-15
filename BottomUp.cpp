// James Vu Assignment3
// This program is a bottom up parser
#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <cerrno>
#include <ctype.h>
#include <stack>
#include <queue>
using namespace std;

string keywords[] = { "int", "float", "bool", "if", "else", "then", "endif",
                      "while", "whileend", "do", "doend", "for", "forend",
                      "input", "output", "and", "or", "function", "begin", "end" };
char sep[] = { '\'', '(', ')', '{', '}', ',', ':', ';' };
char op[]  = { '*', '+', '-', '=', '/', '>', '<', '%'  };
queue<pair<string,string>> tok_lex;
stack<int> stk;
queue<string> syn_str;

string get_file_contents(const string filename) {
  ifstream in(filename, ios::in | ios::binary);
  if (in) {
    string contents;
    in.seekg(0, in.end);
    contents.resize(in.tellg());
    in.seekg(0, in.beg);
    in.read(&contents[0], contents.size());
    in.close();
    return contents;
  }
  throw(errno);
}

bool isseperator(const char ch) {
  for (unsigned int i = 0; i < sizeof(sep); ++i) {
    if (ch == sep[i]) return true;
  }
  return false;
}

bool isoperator(const char ch) {
  for (unsigned int i = 0; i < sizeof(op); ++i) {
    if (ch == op[i]) return true;
  }
  return false;
}

int cur_char(const char ch) {
  if (isalpha(ch))       { return 1; }
  if (isdigit(ch))       { return 2; }
  if (ch == '.')         { return 3; }
  if (ch == '$')         { return 4; }
  if (isseperator(ch))   { return 5; }
  if (isoperator(ch))    { return 6; }
  if (ch == '!')         { return 7; }
  return 0;
}

void add_token_lexeme(const string token, const string lexeme) {
  pair <string, string> token_lexeme(token, lexeme);
  tok_lex.push(token_lexeme);
}

bool check_keyword(const string buf) {
  for (unsigned int i = 0; i < sizeof(keywords); ++i) {
    if (keywords[i] == buf) { return true; }
  }
  return false;
}

void add_keyword_or_identifier(const string buf, const bool flag) {
  if (flag) { add_token_lexeme("Keyword", buf); return; }
  add_token_lexeme("Identifier", buf);
}

void add_realfloat_or_int(const string buf, const bool flag) {
  if (flag) { add_token_lexeme("Real Float", buf); return; }
  add_token_lexeme("Integer", buf);
}

void lexer(const string con) {
  string char_buffer; //initialize character buffer
  int lex_state = 1, char_state = 0; // initial states
  bool backup = 0, float_flag = 0, end_flag = 0; //set flags
  auto it = con.begin();
  while (it != con.end() || !end_flag) { //iterate through string and make sure to process last character
    char_state = cur_char(*it); //get current character state
    switch (lex_state) {
      case 1: // starting state
        switch (char_state) {
          case 1:  lex_state = 2; break; // alphabet
          case 2:  lex_state = 4; break; // digits
          case 3:  lex_state = 7; break; // period
          case 4:  lex_state = 1; break; // $
          case 5:  lex_state = 7; break; // separators
          case 6:  lex_state = 8; break; // operators
          case 7:  lex_state = 9; break; // ! comments
          default: lex_state = 1;        // other
        }
        char_buffer = *it; // sets character buffer to current character
        backup = 0; // resets backup
        break;
      case 2: // in identifier or possible keyword
        if (char_state == 1 || char_state == 2 || char_state == 4) {
          lex_state = 2;      // alpha, digit, or $
          char_buffer += *it; // add to buffer
          break;
        }
        lex_state = 3;      // end of identifier
        backup = 1;         // do not increment it
        break;
      case 3: // end of identifier or keyword
        lex_state = 1; // reset lex state
        add_keyword_or_identifier(char_buffer, check_keyword(char_buffer));
        break;
      case 4: // in number
        if (char_state == 2) {        // digits
          lex_state = 4;              // in number
          char_buffer += *it;         // add to buffer
        } else if (char_state == 3) { // period
          lex_state = 5;              // in number after decimal
          char_buffer += *it;         // add to buffer
          float_flag = 1;               // sets flag
        } else {
          lex_state = 6;              // end of number
          backup = 1;                 // do not increment to next char
        }
        break;
      case 5: // in number after decimal
        if (char_state == 2) { // digits
          lex_state = 5;       // stay in same state
          char_buffer += *it;  // add to buffer
          break;
        }
        lex_state = 6;       // not digit, end of number
        backup = 1;          // do not increment to next char
        break;
      case 6: // end of number
        lex_state = 1; // reset lex state
        add_realfloat_or_int(char_buffer, float_flag);
        float_flag = 0;
        break;
      case 7: // separator
        lex_state = 1; // reset lex state
        add_token_lexeme("Separator", char_buffer);
        backup = 1; // do not increment to next char
        break;
      case 8: // operator
        lex_state = 1; // reset lex state
        add_token_lexeme("Operator", char_buffer);
        backup = 1; // set flag
        break;
      case 9: // in commment
        if (char_state == 7) {
          lex_state = 10; // end comment
          break;
        }
        lex_state = 9; // stay in comment
        break;
      case 10: // end comment
        lex_state = 1; // reset lex state
        break;
      default:
        cout << "Invalid lex state\n";
    }
    if (it == con.end()) { end_flag = 1; } // set end flag to stop loop
    if (!backup && it != con.end()) { ++it; } // increment to next char if not end or backup
  }
  switch (lex_state) {
    case 2: case 4: case 5: case 9:
      cout << "Did not end in final state!";
  }
}

void print_token_lexeme(const string filename) {
  string output = "lexer_output_of_";
  output += filename;
  ofstream outfile;
  outfile.open(output);
  outfile << "Tokens\t\tLexemes\n";
  queue<pair<string,string>> copy = tok_lex;
  while(!copy.empty()) {
    outfile << copy.front().first << " \t" << copy.front().second << "\n";
    copy.pop();
  }
  outfile.close();
}

// converts tok_lex to an integer, so I can use switch in syntax
int cvtTokLex() {
  string tok = tok_lex.front().first;
  if (tok == "Identifier") {
    return 22;
  } else if (tok == "Real Float" || tok == "Integer") {
    return 23;
  }
  string lex = tok_lex.front().second;
  if      (lex == "=") { return 24; }
  else if (lex == "+") { return 25; }
  else if (lex == "-") { return 26; }
  else if (lex == "*") { return 27; }
  else if (lex == "/") { return 28; }
  else if (lex == "(") { return 29; }
  else if (lex == ")") { return 30; }
  else if (lex == ";") { return 31; }
  else if (lex == "$") { return 32; }
  return 0;
}
// 33 = S, 34 = A, 35 = E, 36 = T, 37 = F
void syntax() {
  stk.push(0);
  pair <string, string> token_lexeme("$", "$");
  tok_lex.push(token_lexeme);
  int topStack, incomingToken;
  string intersect = "";
  while (intersect != "Accept" && intersect != "Error") {
    topStack = stk.top();
    incomingToken = cvtTokLex();
    switch (topStack) {
      case 0:
        if (incomingToken == 22) { // Identifier
          stk.push(22); // Identifier
          stk.push(3); // State 3
          tok_lex.pop();
          syn_str.push("S3");
        } else {
          intersect = "Error";
        }
        break;
      case 1:
        if (incomingToken == 32) { // $
          intersect = "Accept";
          syn_str.push("Finish Parsing");
        } else {
          intersect = "Error";
        }
        break;
      case 2:
        if (incomingToken == 32) { // $ R1
          stk.pop();
          stk.pop();
          if (stk.top() == 0) { // State 0
            stk.push(33); // S
            stk.push(1); // State 1
            syn_str.push("R1: S -> A");
          } else {
            intersect = "Error";
          }
        } else {
          intersect = "Error";
        }
        break;
      case 3:
        if (incomingToken == 24) { // =
          stk.push(24); // =
          stk.push(4); // state 4
          tok_lex.pop();
          syn_str.push("S4");
        } else {
          intersect = "Error";
        }
        break;
      case 4:
      case 8:
      case 12:
      case 13:
      case 14:
      case 15:
        switch (incomingToken) {
          case 22: // id
            stk.push(22); // id
            stk.push(9); // state 9
            tok_lex.pop();
            syn_str.push("S9");
            break;
          case 23: // num
            stk.push(23); // num
            stk.push(10); // state 10
            tok_lex.pop();
            syn_str.push("S10");
            break;
          case 29: // (
            stk.push(29); // (
            stk.push(8); // state 8
            tok_lex.pop();
            syn_str.push("S8");
            break;
          default: intersect = "Error";
        }
        break;
      case 5:
        switch (incomingToken) {
          case 25: // +
            stk.push(25); // +
            stk.push(12); // state 12
            tok_lex.pop();
            syn_str.push("S12");
            break;
          case 26: // -
            stk.push(26); // -
            stk.push(13); // state 13
            tok_lex.pop();
            syn_str.push("S13");
            break;
          case 31: // ;
            stk.push(31); // ;
            stk.push(11); // state 11
            tok_lex.pop();
            syn_str.push("S11");
            break;
          default: intersect = "Error";
        }
        break;
      case 6:
        switch (incomingToken) {
          case 27: // *
            stk.push(27); // *
            stk.push(14); // state 14
            tok_lex.pop();
            syn_str.push("S14");
            break;
          case 28: // /
            stk.push(28); // /
            stk.push(15); // state 15
            tok_lex.pop();
            syn_str.push("S15");
            break;
          case 25: // + R5
          case 26: // - R5
          case 30: // ) R5
          case 31: // ; R5
            stk.pop();
            stk.pop();
            if (stk.top() == 4) { // state 4
              stk.push(35); // E
              stk.push(5); // state 5
            } else if (stk.top() == 8) { // state 8
              stk.push(35); // E
              stk.push(16); // state 16
            } else {
              intersect = "Error";
            }
            syn_str.push("R5: E -> T");
            break;
          default: intersect = "Error";
        }
        break;
      case 7:
        switch (incomingToken) {
          case 25: // + R8
          case 26: // - R8
          case 27: // * R8
          case 28: // / R8
          case 30: // ) R8
          case 31: // ; R8
            stk.pop();
            stk.pop();
            switch (stk.top()) {
              case 4: // state 4
              case 8: // state 8
                stk.push(36); // T
                stk.push(6); // state 6
                break;
              case 12: // state 12
                stk.push(36); // T
                stk.push(17); // state 6
                break;
              case 13: // state 13
                stk.push(36); // T
                stk.push(18); // state 6
                break;
              default: intersect = "Error";
            }
            break;
          default: intersect = "Error";
        }
        syn_str.push("R8: T -> F");
        break;
      case 9:
        switch (incomingToken) {
          case 25: // + R10
          case 26: // - R10
          case 27: // * R10
          case 28: // / R10
          case 30: // ) R10
          case 31: // ; R10
            stk.pop();
            stk.pop();
            switch (stk.top()) {
              case 4: // state 4
              case 8: // state 8
              case 12: // state 12
              case 13: // state 13
                stk.push(37); // F
                stk.push(7); // state 7
                break;
              case 14: // state 13
                stk.push(37); // F
                stk.push(19); // state 19
                break;
              case 15: // state 13
                stk.push(37); // F
                stk.push(20); // state 20
                break;
              default: intersect = "Error";
            }
            break;
          default: intersect = "Error";
        }
        syn_str.push("R10: F -> id");
        break;
      case 10:
        switch (incomingToken) {
          case 25: // + R11
          case 26: // - R11
          case 27: // * R11
          case 28: // / R11
          case 30: // ) R11
          case 31: // ; R11
            stk.pop();
            stk.pop();
            switch (stk.top()) {
              case 4: // state 4
              case 8: // state 8
              case 12: // state 12
              case 13: // state 13
                stk.push(37); // F
                stk.push(7); // state 7
                break;
              case 14: // state 13
                stk.push(37); // F
                stk.push(19); // state 19
                break;
              case 15: // state 13
                stk.push(37); // F
                stk.push(20); // state 20
                break;
              default: intersect = "Error";
            }
            break;
          default: intersect = "Error";
        }
        syn_str.push("R11: F -> num");
        break;
      case 11:
        if (incomingToken == 32) { // $ R2
          for (int i = 0; i < 8; ++i) {
            stk.pop();
          }
          if (stk.top() == 0) { // State 0
            stk.push(34); // S
            stk.push(2); // State 1
            syn_str.push("R2: A -> id = E;");
          } else {
            intersect = "Error";
          }
        } else {
          intersect = "Error";
        }
        break;
      case 16:
        switch (incomingToken) {
          case 25: // +
            stk.push(25); // +
            stk.push(12); // state 12
            tok_lex.pop();
            syn_str.push("S12");
            break;
          case 26: // -
            stk.push(26); // -
            stk.push(13); // state 13
            tok_lex.pop();
            syn_str.push("S13");
            break;
          case 30: // )
            stk.push(30); // )
            stk.push(21); // state 21
            tok_lex.pop();
            syn_str.push("S21");
            break;
          default: intersect = "Error";
        }
        break;
      case 17:
        switch (incomingToken) {
          case 27: // *
            stk.push(27); // *
            stk.push(14); // state 14
            tok_lex.pop();
            syn_str.push("S14");
            break;
          case 28: // /
            stk.push(28); // /
            stk.push(15); // state 15
            tok_lex.pop();
            syn_str.push("S15");
            break;
          case 25: // + R3
          case 26: // - R3
          case 30: // ) R3
          case 31: // ; R3
            for (int i = 0; i < 6; ++i) {
              stk.pop();
            }
            if (stk.top() == 4) { // state 4
              stk.push(35); // E
              stk.push(5); // state 5
              syn_str.push("R3: E -> E + T");
            } else if (stk.top() == 8) { // state 8
              stk.push(35); // E
              stk.push(16); // state 16
              syn_str.push("R3: E -> E + T");
            } else {
              intersect = "Error";
            }
            break;
          default: intersect = "Error";
        }
        break;
      case 18:
        switch (incomingToken) {
          case 27: // *
            stk.push(27); // *
            stk.push(14); // state 14
            tok_lex.pop();
            syn_str.push("S14");
            break;
          case 28: // /
            stk.push(28); // /
            stk.push(15); // state 15
            tok_lex.pop();
            syn_str.push("S15");
            break;
          case 25: // + R4
          case 26: // - R4
          case 30: // ) R4
          case 31: // ; R4
            for (int i = 0; i < 6; ++i) {
              stk.pop();
            }
            if (stk.top() == 4) { // state 4
              stk.push(35); // E
              stk.push(5); // state 5
              syn_str.push("R4: E -> E - T");
            } else if (stk.top() == 8) { // state 8
              stk.push(35); // E
              stk.push(16); // state 16
              syn_str.push("R4: E -> E - T");
            } else {
              intersect = "Error";
            }
            break;
          default: intersect = "Error";
        }
        break;
      case 19:
        switch (incomingToken) {
          case 25: // + R6
          case 26: // - R6
          case 27: // * R6
          case 28: // / R6
          case 30: // ) R6
          case 31: // ; R6
            for (int i = 0; i < 6; ++i) {
              stk.pop();
            }
            switch (stk.top()) {
              case 4: // state 4
              case 8: // state 8
                stk.push(36); // T
                stk.push(6); // state 6
                break;
              case 12: // state 12
                stk.push(36); // T
                stk.push(17); // state 6
                break;
              case 13: // state 13
                stk.push(36); // T
                stk.push(18); // state 6
                break;
              default: intersect = "Error";
            }
          break;
          default: intersect = "Error";
        }
        syn_str.push("R6: T -> T * F");
        break;
      case 20:
        switch (incomingToken) {
          case 25: // + R7
          case 26: // - R7
          case 27: // * R7
          case 28: // / R7
          case 30: // ) R7
          case 31: // ; R7
            for (int i = 0; i < 6; ++i) {
              stk.pop();
            }
            switch (stk.top()) {
              case 4: // state 4
              case 8: // state 8
                stk.push(36); // T
                stk.push(6); // state 6
                break;
              case 12: // state 12
                stk.push(36); // T
                stk.push(17); // state 6
                break;
              case 13: // state 13
                stk.push(36); // T
                stk.push(18); // state 6
                break;
              default: intersect = "Error";
            }
            break;
          default: intersect = "Error";
        }
        syn_str.push("R7: T -> T / F");
        break;
      case 21:
        switch (incomingToken) {
          case 25: // + R9
          case 26: // - R9
          case 27: // * R9
          case 28: // / R9
          case 30: // ) R9
          case 31: // ; R9
            for (int i = 0; i < 6; ++i) {
              stk.pop();
            }
            switch (stk.top()) {
              case 4: // state 4
              case 8: // state 8
              case 12: // state 12
              case 13: // state 13
                stk.push(37); // F
                stk.push(7); // state 7
                break;
              case 14: // state 13
                stk.push(37); // F
                stk.push(19); // state 19
                break;
              case 15: // state 13
                stk.push(37); // F
                stk.push(20); // state 20
                break;
              default: intersect = "Error";
            }
            break;
          default: intersect = "Error";
        }
        syn_str.push("R9: F -> ( E )");
        break;
      default: intersect = "Error";
    }
  }
  syn_str.push(intersect);
}

void print_syntax(string filename) {
  string output = "syntax_output_of_";
  output += filename;
  ofstream outfile;
  outfile.open(output);
  outfile << "Production/Actions\n";
  while(!syn_str.empty()) {
    outfile << syn_str.front() << "\n";
    syn_str.pop();
  }
  outfile.close();
}

int main(int argc, char* argv[]) {
  string content, filename;
  cin >> filename;
  content = get_file_contents(filename);
  lexer(content);
  print_token_lexeme(filename);
  syntax();
  print_syntax(filename);
  return 0;
}
