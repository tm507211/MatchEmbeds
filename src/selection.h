/*****************************************************************************
  Author: Charlie Murphy
  Email:  tcm3@cs.princeton.edu

  Date:   February 11, 2019

  Description: selection heuristics
 *****************************************************************************/

#include <vector>
#include <set>
#include <map>
#include <cstdlib>
#include "embedding.h"
#include "definitions.h"
#include "graph.h"

#ifndef CM_SELECTION_H
#define CM_SELECTION_H

/* Variable Selection */
enum Var_selection {
  MIN_REMAINING_VALUES = 0,  // min = even
  MAX_REMAINING_VALUES,  // = 1 max = odd
  MIN_CONFLICTS,
  MAX_CONFLICTS,
  MIN_CONFLICT_HISTORY,
  MAX_CONFLICT_HISTORY,
  FIRST_VAR,
  WEIGHTED_RANDOM_VAR, // weighted by # conflicts
  UNIFORM_RANDOM_VAR,
};

/* Select a variable (edge) in conflicts using the sel heuristic */
template <class Element, class Predicate, class Signature>
bool select_variable(const Embedding<Element, Predicate, Signature>& e, const std::vector<size_t>& conflicts, Var_selection sel, std::vector<size_t>& conflict_history, size_t& d_edge) {
  const Graph& u_graph = e.get_universe_graph();
  const LabeledGraph<prop, prop>& p_graph = e.get_predicate_graph();

  /* select the first valid decision edge */
  if (sel == FIRST_VAR) {
    const std::vector<size_t> & cvars = p_graph.getULabel(conflicts[0]).vars;
    for (size_t i = 0; i < cvars.size(); ++i) {
      if (u_graph.uAdj(cvars[i]).size() > 1) {
        d_edge = cvars[i];
        return true;
      }
    }
    return false;
  /* select a valid decision edge weighted by the # of conflicts it's involved int */
  } else if (sel == WEIGHTED_RANDOM_VAR) {
    std::vector<size_t> vars;
    for (size_t i = 0; i < conflicts.size(); ++i) {
      const std::vector<size_t>& cvars = p_graph.getULabel(conflicts[i]).vars;
      bool valid(false);
      for (size_t j = 0; j < cvars.size(); ++j) {
        if (u_graph.uAdj(cvars[j]).size() > 1) {
          vars.push_back(cvars[j]);
          valid = true;
        }
      }
      if (!valid) return false;
    }
    d_edge = vars[rand()%vars.size()];
    return true;
    /* selects a valid decision edge with uniform weight */
  } else if (sel == UNIFORM_RANDOM_VAR) {
    std::set<size_t> vars;
    for (size_t i = 0; i < conflicts.size(); ++i) {
      const std::vector<size_t>& cvars = p_graph.getULabel(conflicts[i]).vars;
      bool valid(false);
      for (size_t j = 0; j < cvars.size(); ++j) {
        if (u_graph.uAdj(cvars[j]).size() > 1) {
          vars.insert(cvars[j]);
          valid = true;
        }
      }
      if (!valid) return false;
    }
    std::set<size_t>::iterator it = vars.begin();
    size_t var = rand()%vars.size();
    for (size_t i = 0; i < var; ++i) ++it;
    d_edge = *it;
    return true;
  }

  std::map<size_t, size_t> vars; /* maps valid decisions to thier heuristic value */

  /* Compute heuristic value for Remaining Values heuristic */
  if (sel == MIN_REMAINING_VALUES || sel == MAX_REMAINING_VALUES) {
    for (size_t i = 0; i < conflicts.size(); ++i) {
      const std::vector<size_t>& cvars = p_graph.getULabel(conflicts[i]).vars;
      bool valid(false);
      for (size_t j = 0; j < cvars.size(); ++j) {
        if (u_graph.uAdj(cvars[j]).size() > 1) {
          vars[cvars[j]] = u_graph.uAdj(cvars[j]).size();
          valid = true;
        }
      }
      if (!valid) return false;
    }
  /* Compute number of conflicts each decision variable is involved in */
  } else if (sel == MIN_CONFLICTS || sel == MAX_CONFLICTS) {
    for (size_t i = 0; i < conflicts.size(); ++i) {
      const std::vector<size_t>& cvars = p_graph.getULabel(conflicts[i]).vars;
      bool valid(false);
      for (size_t j = 0; j < cvars.size(); ++j) {
        if (u_graph.uAdj(cvars[j]).size() > 1) {
          ++vars[cvars[j]];
          valid = true;
        }
      }
      if (!valid) return false;
    }
  /* Update conflict history of each decision variable and use as heuristic value */
  } else { // sel == MIN_CONFLICT_HISTORY || sel == MAX_CONFLICT_HISTORY
    for (size_t i = 0; i < conflicts.size(); ++i) {
      const std::vector<size_t>& cvars = p_graph.getULabel(conflicts[i]).vars;
      bool valid(false);
      for (size_t j = 0; j < cvars.size(); ++j) {
        if (u_graph.uAdj(cvars[j]).size() > 1) {
          vars[cvars[j]] = ++conflict_history[cvars[j]];
          valid = true;
        }
      }
      if (!valid) return false;
    }
  }

  /* find the argument that (min/max)imizes the heuristic value */
  d_edge = vars.begin()->first;
  if (sel & 1) { // MAX
    size_t max = vars.begin()->second;
    for (std::map<size_t, size_t>::iterator it = vars.begin(); it != vars.end(); ++it) {
      if (max < it->second) {
        d_edge = it->first;
        max = it->second;
      }
    }
  } else { // MIN
    size_t min = vars.begin()->second;
    for (std::map<size_t, size_t>::iterator it = vars.begin(); it != vars.end(); ++it) {
      if (min > it->second) {
        d_edge = it->first;
        min = it->second;
      }
    }
  }
  return true;
}

#endif
