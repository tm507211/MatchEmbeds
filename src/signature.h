/*********************************************************************
  Author: Charlie Murphy
  Email:  tcm3@cs.princeton.edu

  Date:   January 23, 2019

  Description: This file contains descriptions of signature types for
    structure universe elements

    All signatures are required to implement the following publically
    available API:

    Signature(size_t self);
    void update_signature(size_t predicate, std::vector<size_t> vars, size_t position);
    bool operator < (const Signature& other) const;
 *********************************************************************/

#include <vector>

/* This signature records the multiset of positions within a relation
   this element appears in optimized for densely packed structures */
class MultiSetSignature {
 public:
  MultiSetSignature(size_t self) {}

  void update_signature(size_t predicate, std::vector<size_t> vars, size_t pos) {
    if (occurences.size() < predicate + 1) {
      occurences.resize(predicate + 1);
      occurences[predicate].resize(vars.size(), 0);
    }
    ++occurences[predicate][pos];
  }

  bool operator <= (const MultiSetSignature& other) const {
    bool subset = occurences.size() <= other.occurences.size();
    for (size_t i = 0; subset && i < occurences.size(); ++i) {
      subset = occurences[i].size() <= other.occurences[i].size();
      for (size_t j = 0; subset && j < occurences[i].size(); ++j) {
        subset = occurences[i][j] <= other.occurences[i][j];
      }
    }
    return subset;
  }

 private:
  std::vector<std::vector<size_t>> occurences;
};
