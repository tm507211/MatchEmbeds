/*****************************************************************************
  Author: Charlie Murphy
  Email:  tcm3@cs.princeton.edu

  Date:   February 11, 2019

  Description: match embeds algorithm
 *****************************************************************************/

#include <vector>
#include <stack>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include "definitions.h"
#include "graph.h"
#include "embedding.h"
#include "selection.h"

#ifndef CM_MATCH_EMBEDS_H
#define CM_MATCH_EMBEDS_H

template <class Element, class Predicate, class Signature>
void find_conflicts(const Embedding<Element, Predicate, Signature>& e, const std::vector<int>& matching, std::vector<size_t>& confs);

template <class Element, class Predicate, class Signature>
void backtrack(Embedding<Element, Predicate, Signature>& e, std::stack<decision>& decisions);

template <class Element, class Predicate, class Signature>
bool MatchEmbeds(Embedding<Element, Predicate, Signature>& e, Var_selection sel = MIN_REMAINING_VALUES) { /* default selection heuristic is minimum remaining values */
  /* Remove any edges inconsistent without needing to make a decision */
  {
    std::vector<Graph::VertexPair> p_removed, u_removed;
    std::vector<size_t> junk;
    if (!e.get_universe_graph().unit_prop(u_removed, junk, junk)) return false;
    e.filter(p_removed, u_removed);
  }
  if (!e.is_valid()) return false;
  Graph& u_graph = e.get_universe_graph();

  srand(time(NULL));
  std::vector<size_t> conflict_history, conflicts;
  conflict_history.resize(u_graph.uSize(), 0);

  std::vector<int> match1, match2, vis;

  match1.resize(u_graph.uSize(), -1);
  match2.resize(u_graph.vSize(), -1);
  vis.resize(u_graph.uSize(), 0);
  size_t ans;

  std::stack<decision> decisions;

  while (true) {
    /* unmatch any edges that no longer belong to the universe graph */
    for (size_t i = 0; i < match1.size(); ++i) {
      if (match1[i] != -1 && !u_graph.has_edge(i, match1[i])) {
        match2[match1[i]] = -1;
        match1[i] = -1;
      }
    }

    std::fill(vis.begin(), vis.end(), 0);            /* reset variables for matching problem */
    ans = u_graph.max_matching(match1, match2, vis); /* compute maximum cardinality matching */

    /* no total matching exists => backtrack */
    if (ans != u_graph.uSize()) {
      if(decisions.size() >= 1) {
        backtrack(e, decisions);
        continue;
      } else {
        return false;
      }
    }
    /* find any predicates p(x0, ..., xn) that are not satisfied by candidate embedding match1 */
    find_conflicts(e, match1, conflicts);
    /* if all predicates are satisfied then the candidate is a valid embedding */
    if (conflicts.size() == 0) {
      return true;
    }
    size_t d_edge; /* edge in match1 selected using sel heuristic */
    bool valid = select_variable(e, conflicts, sel, conflict_history, d_edge); /* valid <==> some edge can be selected <==> embedding instance is consistent */
    if (!valid) {
      if (decisions.size() >= 1) {
        backtrack(e, decisions);
        continue;
      } else {
        return false;
      }
    }

    /* make the decision that d_edge |-> match1[d_edge] */
    decisions.emplace(d_edge, match1[d_edge]);
    e.decide(decisions.top());

    /* if this decision was inconsistent backtrack */
    if (!e.is_valid()) {
      backtrack(e, decisions);
    }
  } /* continue until we find an embedding or there are no more candidate embeddings are left to explore */
}

template <class Element, class Predicate, class Signature>
void find_conflicts(const Embedding<Element, Predicate, Signature>& e, const std::vector<int>& matching, std::vector<size_t>& confs) {
  const LabeledGraph<prop, prop>& p_graph = e.get_predicate_graph();
  confs.clear();
  /* for each p(x0, ..., xn) */
  for (size_t i = 0, j, k; i < p_graph.uSize(); ++i) {
    const std::vector<Graph::Edge>& adj = p_graph.uAdj(i);
    const std::vector<size_t>& u_vars = p_graph.getULabel(i).vars;
    /* is there some q(y0, ..., yn) such that not (for each (xi, yi), matching[xi] = yi) */
    for (j = 0; j < adj.size(); ++j) {
      const std::vector<size_t>& v_vars = p_graph.getVLabel(adj[j].vertex).vars;
      for (k = 0; k < u_vars.size() && matching[u_vars[k]] == v_vars[k]; ++k);
      if (k == u_vars.size()) break;
    }
    /* if there is add p(x0, ..., xn) to the set of conflicts */
    if (j == adj.size()) {
      confs.push_back(i);
    }
  }
}

template <class Element, class Predicate, class Signature>
void backtrack(Embedding<Element, Predicate, Signature>& e, std::stack<decision>& decisions) {
  Graph& u_graph = e.get_universe_graph();
  decision& d = decisions.top();

  e.add_back(d.remove_p, d.remove_u);

  // blame and remove (d.u |-> d.v) edge
  size_t pos;
  const std::vector<Graph::Edge>& adj = u_graph.uAdj(d.u);
  for (pos = 0; pos < adj.size() && adj[pos].vertex != d.v; ++pos);
  assert (pos < adj.size());
  //  bool check = u_graph.check();
  u_graph.remove_edge(d.u, pos);
  //  assert (!check || u_graph.check()); /* ensure proper operation of edge removal */

  decisions.pop();

  /* if this isn't the root decision it is possible for the edge (d.u, d.v) to belong to an embedding */
  if (decisions.size() > 0) {
    decision& prev = decisions.top();
    prev.remove_u.emplace_back(d.u, d.v);
  }
}

#endif
