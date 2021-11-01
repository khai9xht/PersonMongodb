#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "persondb/database.h"

int main(int argc, char *argv[]) {
  std::string str_uri = "mongodb://192.168.50.17:9876";
  int dim = 10;

  std::vector<double> value;
  value.reserve(dim);
  for (int i = 0; i < dim; i++) {
    value.push_back(drand48());
  }
  std::string engine = "ArcFace";
  EmbeddingVector embed(engine, value);

  std::vector<double> value1;
  value1.reserve(dim);
  for (int i = 0; i < dim; i++) {
    value1.push_back(drand48());
  }
  std::string engine1 = "CosFace";
  EmbeddingVector embed1(engine1, value1);

  std::string face_id = "qwrqr";
  std::string imgPath = "eqwfe";
  std::vector<EmbeddingVector> vectors;
	vectors.push_back(embed);
  Face face(face_id, imgPath, vectors);

  std::string face_id1 = "qwrqr";
  std::string imgPath1 = "eqwfe";
  std::vector<EmbeddingVector> vectors1;
  vectors1.push_back(embed1);
  Face face1(face_id1, imgPath1, vectors1);

  std::string person_id = "1";
  std::string name = "abc";
  std::string outsideID = "asc";
  std::vector<Face> faces;
	faces.push_back(face);
  faces.push_back(face1);
  Person person(person_id, name, outsideID, faces);

  auto persondb = Database();
  persondb.Initialize(str_uri);
  Status s = persondb.insertPerson(person);
  return EXIT_SUCCESS;
}