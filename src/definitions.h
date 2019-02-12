/********************************************************************************
    Author: Charlie Murphy
    Email:  tcm3@cs.princeton.edu

    Date:   January 14, 2019

    Description: Header file for various types used in embedding algorithm.
 ********************************************************************************/

#include <vector>
#include <cstdint>
#include "graph.h"

#ifndef CM_EMBEDDING_DEF_H
#define CM_EMBEDDING_DEF_H

/* The label for the proposition graph predicate symbol and list of arguments */
struct prop{
  prop (size_t p = 0) : pred(p) {}
  /* copy semantics / cast to const if you want to copy a non-const vector*/
  prop (size_t p, const std::vector<size_t>& v) : pred(p), vars(v) {}
  /* move semantics (default) */
  prop (size_t p, std::vector<size_t>& v) : pred(p), vars(std::move(v)) {}
  size_t pred;
  std::vector<size_t> vars;
};

/* The type of decisions:
     (u, v) : an edge in the maximum matching on the universe graph
     remove_u : edges in universe graph removed because of this decision
     remove_p : edges in predicate graph removed because of this decision */
struct decision{
    size_t u;
    size_t v;
    std::vector<Graph::VertexPair> remove_u;
    std::vector<Graph::VertexPair> remove_p;
    decision(size_t _u, size_t _v) : u(_u), v(_v) { }
    decision(size_t _u, size_t _v, std::vector<Graph::VertexPair>& _remove_u, std::vector<Graph::VertexPair>& _remove_p) : u(_u), v(_v), remove_u(_remove_u), remove_p(_remove_u) { }
};

#endif
