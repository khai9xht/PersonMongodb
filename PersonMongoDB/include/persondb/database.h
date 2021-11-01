#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
// #include <opencv2/core.hpp>
#include <string>
#include <vector>

#include "bsoncxx/builder/basic/kvp.hpp"
#include "bsoncxx/builder/list.hpp"
#include "bsoncxx/document/value.hpp"
#include "bsoncxx/document/view.hpp"
#include "mongocxx/collection.hpp"
#include "mongocxx/database.hpp"

struct EmbeddingVector {
  std::string engine;
  std::vector<double> value;
  EmbeddingVector();
  EmbeddingVector(const std::string &_engine,
                   const std::vector<double> &_value);

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

class Database {
 public:
  Database();
  void Initialize(std::string &str_uri);
  Status insertPerson(Person &person) const;
  Status InsertDocFromJson(std::string_view &json) const;
  Status SelectAll(std::vector<bsoncxx::document::view> &people) const;
  Status DeleteDocByPersonID(std::string &id) const;
  Status DeleteDocByPersonName(std::string &name) const;
  Status SellectAllVectors(std::vector<EmbeddingVector> &embeddingvectors,
                           std::string &engine);
  ~Database() {
    delete[] PersonFields;
    delete[] FaceFields;
    delete[] FaceEmbeddingFields;
  }

 private:
  bool exists(std::string tableName);
  mongocxx::client client;
  mongocxx::database PersonDB; 
  char *PersonFields;
  char *FaceFields;
  char *FaceEmbeddingFields;
};