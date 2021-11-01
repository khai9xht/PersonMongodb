#include <iostream>
#include <string>
#include <vector>

#include "mongocxx/cursor.hpp"
#include "persondb/database.h"

int main(int argc, char *argv[]) {
  std::string uri = "mongodb://192.168.50.17:9876";
  auto persondb = Database();
  persondb.Initialize(uri);
  std::vector<EmbeddingVector> EmbeddingVectors;
  std::string engine = "ArcFace";
  persondb.SellectAllVectors(EmbeddingVectors, engine);
  return 0;
}