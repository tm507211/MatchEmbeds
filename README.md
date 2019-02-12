# MatchEmbeds: A practical algorithm for the structure embedding problem

This repo contains a proof of concept implementation of MatchEmbeds availabe under the MIT lisence that addresses the structure embedding problem.

Structures are a generalization of graphs. Each structure contains a finite universe of elements and a finite set of relations over those elements. Graphs are structures where vertices comprise the universe and has a single relation (the edge relation) determining which vertices are adjacent to each other.

The structure embedding is then the generalization of subgraph isomorphism to structures. The structure embedding problem then asks, given two first order relational structures, `A` and `B`, is there an injective homomorphism from `A` to `B` or in other words is there a copy of `A` within `B` that preserves the structure of `A` but not necessarily the names of its elements.

MatchEmbeds starts by constructing a bipartite graph from the universe elements of structure `A` to the universe of structure `B`. An edge connects an element `a` to an element `b` if `a` may map to `b`. The algorithm then searches over the space of total matchings (which correspond to injective functions from `A` to `B`) of this bipartite graph to find an embedding.

For more detailed description of the structures, structure embedding, or MatchEmbeds please refer to this [paper](http://www.cs.princeton.edu/~tcm3/docs/vmcai19.pdf) or these [slides](http://www.cs.princeton.edu/~tcm3/docs/vmcai_2019_slides.pdf).

## How to Compile

Download the git repository.

Ensure that your default c++ compiler allows compiling C++11 and openMP and then run the makefile.

```Bash
make
```

## How to Use

A very simple driver program is provided to quickly test the structure embedding problem by reading in `N` structure embedding problems and outputting "True" or "False" weather an embedding exists from structure `A` to `B` for each instance. An example file containing two structures is shown below.

```Bash
./match-embeds file1.struct ... fileN.struct
```

```Bash
======================== file1.struct ========================
{p(1, 2), q(3), r(3, 2, 4)}
{p(a, b), q(c), r(c, b, d)}
```

Alternatively, you can use the header files and incorporate MatchEmbeds into your own project!

```C++
#include "signature.h"
#include "structure.h"
#include "embedding.h"
#include "match_embeds.h"

void my_function() {
  Structure<std::string, size_t, MultiSetSignature> A, B;
  
  for (size_t i = 0; i < 10; ++i) {
    A.add_element(i);
  }
  A.add_proposition(p, {0, 1}); // adds p(0, 1) to structure
  A.add_proposition(q, {});     // adds q() to structure
  
  for (size_t i = 0; i < 15; ++i) {
    B.add_element(i);
  }
  
  B.add_proposition(p, {4, 7});
  B.add_proposition(q, {});
  B.add_proposition(r, {4, 1, 13});
  
  Embedding<std::string, size_t, MultiSetSignature> emb(A, B);
  if (MatchEmbeds(emb)) {
    // An embedding exists
  } else {
    // No embedding exists
  }
}
```

## Other

This repo contains an implementation of MatchEmbeds available under the MIT lisence.

If you use this in academic work, please cite the paper detailing MatchEmbeds:

```
Murphy C., Kincaid Z. (2019) A Practical Algorithm for Structure Embedding. In: Enea C., Piskac R. (eds)
    Verification, Model  Checking, and Abstract Interpretation. VMCAI 2019.
    Lecture Notes in Computer Science, vol 11388. Springer, Cham.
```
