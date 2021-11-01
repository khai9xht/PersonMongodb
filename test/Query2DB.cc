#include <iostream>
#include <string>
#include <vector>

#include "mongocxx/cursor.hpp"
#include "persondb/database.h"

int main(int argc, char *argv[]) {
	// Initialize MongoDb connection
  std::string uri = "mongodb://192.168.50.17:9876";
  auto persondb = Database();
  persondb.Initialize(uri);
	// Select all vector from DB
  std::vector<EmbeddingVector> EmbeddingVectors;
  std::string engine = "ArcFace";
  persondb.SellectAllVectors(EmbeddingVectors, engine);
  // convert all vector from std::vector<EmbeddingVector> to float* (input of faiss)
  int length = EmbeddingVectors.size();
	std::vector<double> EmbedVectorArray; 
	auto it = EmbeddingVectors.begin();
	int dim = (it->value).size();
	float *arr_embeds = new float[length*dim];
	int row_index = 0;
  while ((it != EmbeddingVectors.end()) && (row_index != length)) {
		std::vector<double> embed_vector = it->value;
    auto value_it = embed_vector.begin();
		int column_index = 0; 
		while (value_it != embed_vector.end()) {
			arr_embeds[row_index*dim+column_index] = static_cast<float>(*value_it);
			value_it++;
			length++;
		}	
		it++;
		row_index++;
  }
	delete [] arr_embeds;
  return 0;
}