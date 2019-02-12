/*******************************************************************
    Author: Charlie Murphy
    Email:  tcm3@cs.princeton.edu

    Date:   February 12, 2019

    Description: Driver program for reading in pairs of structures
    and running MatchEmbeds algorithm.
 *******************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include "structure.h"
#include "embedding.h"
#include "signature.h"
#include "match_embeds.h"
#include "formats.h"

using namespace std;

int main(int argc, char ** argv) {

  for (size_t i = 1; i < argc; ++i) {
    Structure<string, string, MultiSetSignature> s1, s2;

    ifstream ins(argv[i]); // assumes pair of structs in struct format
    bool valid = true;
    s1 = read_struct_file<MultiSetSignature>(ins, valid);
    if (!valid) {
      cerr << "Structure 1 in " << argv[i] << " is not a valid structure!" << endl;
      continue;
    }
    s2 = read_struct_file<MultiSetSignature>(ins, valid);
    if (!valid) {
      cerr << "Structure 2 in " << argv[i] << " is not a valid structure!" << endl;
      continue;
    }
    Embedding<string, string, MultiSetSignature> emb(s1, s2);
    cout << (MatchEmbeds(emb) ? "True" : "False") << endl;
  }
}
