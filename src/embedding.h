/*****************************************************************************
  Author: Charlie Murphy
  Email:  tcm3@cs.princeton.edu

  Date:   January 23, 2019

  Description: An embedding instance
 *****************************************************************************/

#include <vector>
#include "structure.h"
#include "definitions.h"
#include "graph.h"

#ifndef CM_EMBEDDING_H
#define CM_EMBEDDING_H

template <class Element, class Predicate, class Signature>
class Embedding{
  public:
    typedef Structure<Element, Predicate, Signature> Str;

    Embedding(const Str& a, const Str& b) : u_graph_(a.universe_size(), b.universe_size()), valid_(true) {
      fill_u_graph(a, b);

      std::vector<prop> propsA, propsB;
      for (auto mit = a.relations.begin(); mit != a.relations.end(); ++mit) {
        for (auto sit = mit->second.begin(); sit != mit->second.end(); ++sit) {
          propsA.emplace_back(mit->first, (*sit));
        }
      }

      for (auto mit = b.relations.begin(); mit != b.relations.end(); ++mit) {
        for (auto sit = mit->second.begin(); sit != mit->second.end(); ++sit) {
          propsB.emplace_back(mit->first, (*sit));
        }
      }

      p_graph_ = std::move(LabeledGraph<prop, prop>(std::move(propsA), std::move(propsB)));
      fill_p_graph();
      fill_inv_label();
    }

    /* Get the underlying representation of the universe and predicate matchings */
    Graph& get_universe_graph() { return u_graph_; }
    const Graph& get_universe_graph() const { return u_graph_; }
    LabeledGraph<prop, prop>& get_predicate_graph() { return p_graph_; }
    const LabeledGraph<prop, prop>& get_predicate_graph() const { return p_graph_; }
    bool is_valid() const { return valid_; }

    /* Commit to a decision and ensure arc consistency. */
    void decide(decision& d) {
      if (!u_graph_.commit_edge(d.u, d.v, d.remove_u)) {
        valid_ = false;
      } else {
        const std::vector<Graph::Edge>& preds = u_inv_label_[d.u];
        // start filtering likely candidates to avoid expensive filter rounds
        for (size_t i = 0; i < preds.size(); ++i) {
          size_t p = preds[i].vertex;
          filter_one(p, d.remove_u, d.remove_p);
          if (!valid_) return;
        }
        filter(d.remove_u, d.remove_p);
      }
    }

    /* Filter the graph to achieve arc consistency */
    bool filter(std::vector<Graph::VertexPair>& remove_u, std::vector<Graph::VertexPair>& remove_p) {
      bool filtered = true; // more filtering to do?
      while (valid_ && filtered) {
        filtered = false;
        for (size_t p = 0; p < p_graph_.uSize(); ++p) {
          if (filter_one(p, remove_u, remove_p)) {
            filtered = true;
          }
          if (!valid_) {
            return false;
          }
        }
      }
      return true;
    }

    /* Add edges back to the predicate and universe graph (and assume the graph is valid) */
    void add_back(const std::vector<Graph::VertexPair>& p_edges, const std::vector<Graph::VertexPair>& u_edges) {
      valid_ = true;
      for (size_t i = 0; i < p_edges.size(); ++i) {
        p_graph_.add_edge(p_edges[i].u, p_edges[i].v);
      }
      for (size_t i = 0; i < u_edges.size(); ++i) {
        u_graph_.add_edge(u_edges[i].u, u_edges[i].v);
      }
    }

  private:
    Graph u_graph_;
    LabeledGraph<prop, prop> p_graph_;
    /* (vert, pos) \in u_inv_label_[u] -> p_graph_.getULabel(vert).vars[pos] = u */
    std::vector<std::vector<Graph::Edge>> u_inv_label_;
    /* (vert, pos) \in u_inv_label_[v] -> p_graph_.getULabel(vert).vars[pos] = v */
    std::vector<std::vector<Graph::Edge>> v_inv_label_;
    bool valid_;

    /* Takes 2 structures and constructs universe graph */
    void fill_u_graph(const Str& a, const Str& b) {
      std::vector<std::vector<size_t>> adj;
      adj.resize(a.universe_size());

      /* use adj as placeholder in order to safely parallelize */
      #pragma omp parallel for schedule(guided)
      for (size_t i = 0; i < a.universe_size(); ++i) {
        for (size_t j = 0; j < b.universe_size(); ++j) {
          if (a.get_signature(i) <= b.get_signature(j)) {
            adj[i].push_back(j);
          }
        }
      }
      /* Add (undirected) edges to universe graph */
      for (size_t i = 0; i < adj.size(); ++i) {
        for (size_t j = 0; j < adj[i].size(); ++j) {
          u_graph_.add_edge(i, adj[i][j]);
        }
      }

      for (size_t i = 0; i < u_graph_.uSize(); ++i) {
        if (u_graph_.uAdj(i).size() == 0) {
          valid_ = false;
        }
      }
    }

    /* finish constructing the predicate graph */
    void fill_p_graph() {
      if (!valid_) return;
      for (size_t i = 0; i < p_graph_.uSize(); ++i) {
        const prop& u_label = p_graph_.getULabel(i);
        for (size_t j = 0; j < p_graph_.vSize(); ++j) {
          const prop& v_label = p_graph_.getVLabel(j);
          bool mem(u_label.pred == v_label.pred);
          for (size_t k = 0; mem && k < u_label.vars.size(); ++k) {
            mem = u_graph_.has_edge(u_label.vars[k], v_label.vars[k]);
          }
          if (mem) p_graph_.add_edge(i, j);
        }
      }

      for (size_t i = 0; i < p_graph_.uSize(); ++i) {
        if (p_graph_.uAdj(i).size() == 0) {
          valid_ = false;
          break;
        }
      }
    }

    /* construct inverse labels */
    void fill_inv_label() {
      if (!valid_) return;
      u_inv_label_.resize(u_graph_.uSize());
      for (size_t i = 0; i < p_graph_.uSize(); ++i){
        const std::vector<size_t>& vars = p_graph_.getULabel(i).vars;
        for (size_t k = 0; k < vars.size(); ++k){
          u_inv_label_[vars[k]].emplace_back(i, k);
        }
      }

      v_inv_label_.resize(u_graph_.vSize());
      for (size_t i = 0; i < p_graph_.vSize(); ++i){
        const std::vector<size_t>& vars = p_graph_.getVLabel(i).vars;
        for (size_t k = 0; k < vars.size(); ++k){
          v_inv_label_[vars[k]].emplace_back(i, k);
        }
      }
    }

    /* Filter one predicate p(x0, ..., xn) one iteration */
    bool filter_one(size_t p, std::vector<Graph::VertexPair>& remove_u, std::vector<Graph::VertexPair>& remove_p) {
      const std::vector<Graph::Edge>& p_adj = p_graph_.uAdj(p);
      const std::vector<size_t>& p_vars = p_graph_.getULabel(p).vars;
      /* For each edge p(x_1,...,x_n) -> q(y_1, ..., y_n) in the
         predicate graph, ensure that each of x_1 -> y_1, ..., x_n ->
         y_n is the universe graph. */
      size_t q = 0;
      bool filtered = false;
      while (q < p_adj.size()) {
        const std::vector<size_t>& q_vars = p_graph_.getVLabel(p_adj[q].vertex).vars;
        bool remove_pq = false;
        for (size_t i = 0; !remove_pq && i < p_vars.size(); ++i) {
          const std::vector<Graph::Edge>& u_adj = u_graph_.uAdj(p_vars[i]);
          size_t v;
          for (v = 0; v < u_adj.size() && u_adj[v].vertex != q_vars[i]; ++v);
          if (v == u_adj.size()) {
            remove_pq = true;
          }
        }
        if (remove_pq) {
          remove_p.emplace_back(p, p_adj[q].vertex);
          p_graph_.remove_edge(p, q);
          filtered = true;
        } else {
          ++q;
        }
      }
      if (q == 0) {
        valid_ = false;
        return true;
      } else if (q == 1) { // unit prop
        if (!p_graph_.commit_edge(p, p_adj[0].vertex, remove_p)) {
          valid_ = false;
          return true;
        }
        const std::vector<size_t>& q_vars = p_graph_.getVLabel(p_adj[0].vertex).vars;
        for (size_t i = 0; i < p_vars.size(); ++i) {
          if (!u_graph_.commit_edge(p_vars[i], q_vars[i], remove_u)) {
            valid_ = false;
            return true;
          }
        }
      } else {
        /* Suppose that x_i -> y.  Then there must be some p(x_1,...,x_n) ->
           q(y_1, ..., y_n) in the predicate graph with y = y_i */
        for (size_t i = 0; i < p_vars.size(); ++i) {
          const std::vector<Graph::Edge>& xi_adj = u_graph_.uAdj(p_vars[i]);
          size_t y = 0;
          while (y < xi_adj.size()) {
            bool remove_xiy = true;
            for (size_t q = 0; remove_xiy && q < p_adj.size(); ++q) {
              const std::vector<size_t>& q_vars = p_graph_.getVLabel(p_adj[q].vertex).vars;
              if (xi_adj[y].vertex == q_vars[i]) {
                remove_xiy = false;
              }
            }
            if (remove_xiy) {
              remove_u.emplace_back(p_vars[i], xi_adj[y].vertex);
              u_graph_.remove_edge(p_vars[i], y);
              filtered = true;
            } else {
              ++y;
            }
          }
          if (y == 0) {
            valid_ = false;
            return true;
          } else if (y == 1) { // unit prop
            if (!u_graph_.commit_edge(p_vars[i], xi_adj[0].vertex, remove_u)) {
              valid_ = false;
              return true;
            }
          }
        }
      }
      return filtered;
    }

};

#endif
