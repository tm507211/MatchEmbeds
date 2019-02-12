/****************************************************************************
    Author: Charlie Murphy
    Email:  tcm3@cs.princeton.edu

    Date:   February 12, 2019

    Description: Header file for reading in structures from files
 ****************************************************************************/
#include <fstream>
#include <vector>
#include <string>
#include <cctype>

template <class Signature>
Structure<std::string, std::string, Signature> read_struct_file(std::ifstream& ins, bool& valid);

template <class Signature>
Structure<std::string, std::string, Signature> read_structure(const std::string& file_name, bool& valid) {
  std::string ext = file_name.substr(file_name.find_last_of(".") + 1);

  if (ext == "struct") {
    std::ifstream ins(file_name);
    return read_struct_file<Signature>(ins, valid);
  } else {
    valid = false;
    return Structure<std::string, std::string, Signature>();
  }
}

template <class Signature>
Structure<std::string, std::string, Signature> read_struct_file(std::ifstream& ins, bool& valid) {
  Structure<std::string, std::string, Signature> s; // structure to return;

  size_t state(0);
  char c;
  std::string pred;
  std::string var;
  std::vector<std::string> vars;
  char quote(0);
  bool comment(false);

  while (ins && state < 4) {
    ins.get(c);
    if (c == '#') {
      comment = true;
    }
    if (comment) {
      if (c == '\n') comment = false; // end comment on new line
      continue;
    }
    if (c == '\'' || c == '\"') {
      if (quote == c) {
        quote = 0;
        continue;
      } else if (!quote && ((state == 1 && pred.length() == 0) || (state == 2 && var.length() == 0))) { // valid state for begining qouted text
        quote = c;
        continue;
      } else if (!quote) {
        state = 5;
        continue;
      }
    }
    if (quote) {
      if (state == 1) { pred.push_back(c); }
      else { var.push_back(c); }
    } else {
      if (isspace(c)) continue;
      switch (state) {
        case 0:
          if (c == '{') { state = 1; }
          else { state = 5; }
          break;
        case 1:
          if (c == '(') { state = 2; }
          else if (c == ',' || c == '}') {
            if (pred.length() != 0) {  // ignore empty string predicates
              s.add_proposition(pred, vars);
              pred.clear();
            }
            if (c == ',') { state = 1; }
            else { state = 4; }
          } else {
            pred.push_back(c);
          }
          break;
        case 2:
          if (c == ',' || c == ')') {
            if (var.length() != 0) { // ignore 0 length variables
              vars.push_back(var);
              var.clear();
            }
            if (c == ')') {
               s.add_proposition(pred, vars);
               pred.clear(); vars.clear();
               state = 3;
            }
          } else {
            var.push_back(c);
          }
          break;
        case 3:
          if (c == ',') { state = 1; }
          else if (c == '}') { state = 4; }
          else state = 5;
          break;
        default:
          state = 5;
          break;
      }
    }
  }
  if (state != 4) {
    valid = false;
  }
  return s;
}
