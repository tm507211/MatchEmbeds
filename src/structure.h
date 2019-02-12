/********************************************************************************
    Author: Charlie Murphy
    Email:  tcm3@cs.princeton.edu

    Date:   January 22, 2018

    Description: Class representation of first order relational structures
 ********************************************************************************/

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <utility>

#ifndef CM_STRUCTURE_H
#define CM_STRUCTURE_H

template <class Element, class Predicate, class Signature>
class Embedding;

/* Definition of Structure */
template <class Element, class Predicate, class Signature>
class Structure {
 public:
  void add_element(const Element& e) {
    if (universe.find(e) == universe.end()) {
      universe.emplace(e, universe.size());
      elements.push_back(e);
      signatures.push_back(Signature(universe[e]));
    }
  }

  static void add_relation(const Predicate& p) {
    if (rel_symbols.find(p) == rel_symbols.end()) {
      rel_symbols.emplace(p, rel_symbols.size());
      predicates.push_back(p);
    }
  }

  void add_proposition(const Predicate& p, const std::vector<Element>& vars) {
    std::vector<size_t> uvars;
    for (size_t i = 0; i < vars.size(); ++i) {
      if (universe.find(vars[i]) == universe.end()) {
        add_element(vars[i]);
      }
      uvars.push_back(universe[vars[i]]);
    }
    if (rel_symbols.find(p) == rel_symbols.end()) {
      add_relation(p);
    }
    size_t q = rel_symbols[p];
    if (relations[q].find(uvars) == relations[q].end()) { /* relies on fact that [] notation initializes to empty set if not already in map */
      relations[q].insert(uvars);
      for (size_t i = 0; i < uvars.size(); ++i) {
        signatures[uvars[i]].update_signature(q, uvars, i);
      }
    }
  }

  size_t universe_size() const {
    return universe.size();
  }

  const Signature& get_signature(size_t u) const {
    return signatures[u];
  }

  friend std::ostream& operator << (std::ostream& outs, const Structure& s) {
    outs << "Universe: {";
    for (size_t i = 0; i < s.elements.size(); ++i) {
      if (i != 0) {
        outs << ", ";
      }
      outs << s.elements[i];
    }
    outs << "}" << std::endl;

    for (auto it = s.relations.begin(); it != s.relations.end(); ++it) {
      for (auto sit = it->second.begin(); sit != it->second.end(); ++sit) {
        outs << predicates[it->first] << "(";
        for (size_t i = 0; i < sit->size(); ++i) {
          if (i != 0) {
            outs << ", ";
          }
          outs << s.elements[sit->at(i)];
        }
        outs << ")" << std::endl;
      }
    }
    return outs;
  }

  friend  Embedding<Element, Predicate, Signature>;

 private:
  std::map<Element, size_t> universe;
  std::vector<Element> elements;       /* reverse map of universe */
  std::vector<Signature> signatures;   /* signature of each universe element */

  static std::map<Predicate, size_t> rel_symbols;   /* Shared across all instances of this structure type */
  static std::vector<Predicate> predicates;         /* reverse map of rel_symbols */
  std::map<size_t, std::set<std::vector<size_t>>> relations;
};

template <class Element, class Predicate, class Signature>
std::map<Predicate, size_t> Structure<Element, Predicate, Signature>::rel_symbols;

template <class Element, class Predicate, class Signature>
std::vector<Predicate> Structure<Element, Predicate, Signature>::predicates;

#endif
