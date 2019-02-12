match-embeds: src/match_embeds.cc src/definitions.h src/embedding.h src/formats.h src/graph.h src/match_embeds.h src/selection.h src/signature.h src/structure.h
	g++ -std=c++11 src/match_embeds.cc -o match-embeds -fopenmp
