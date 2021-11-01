#pragma once

#include <string>
#include <vector>

#include "bsoncxx/builder/basic/document.hpp"

struct EmbeddingVector {
  std::string engine;
  std::vector<double> value;
  EmbeddingVector();
  EmbeddingVector(const std::string &_engine,
                  const std::vector<double> &_value);
  explicit EmbeddingVector(const bsoncxx::builder::basic::document &doc);

  bsoncxx::document::value getSubDoc() const;
};

struct Face {
  std::string id;
  std::string imgPath;
  std::vector<EmbeddingVector> vectors;
  Face();
  Face(const std::string &_id, const std::string &_imgPath,
       std::vector<EmbeddingVector> &_vectors);

  bsoncxx::document::value getSubDoc() const;
};

struct Person {
  std::string id;
  std::string name;
  std::string outsideID;
  std::vector<Face> faces;
  Person(const std::string &_id, const std::string &_name,
         const std::string &_outsideID, std::vector<Face> &_faces);
  Person();
  bsoncxx::document::value getDoc() const;
};

enum class Status { OK, NOT_FOUND, CONFLICT, FAIL };