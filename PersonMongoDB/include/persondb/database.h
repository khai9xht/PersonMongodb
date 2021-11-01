#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
// #include <opencv2/core.hpp>
#include <string>
#include <vector>

#include "bsoncxx/builder/basic/document.hpp"
#include "bsoncxx/builder/basic/kvp.hpp"
#include "bsoncxx/builder/list.hpp"
#include "bsoncxx/document/value.hpp"
#include "bsoncxx/document/view.hpp"
#include "persondb/db_structure.h"
#include "mongocxx/collection.hpp"
#include "mongocxx/database.hpp"


class Database {
 public:
  Database();
  void Initialize(const std::string &str_uri);
  Status insertPerson(const Person &person) const;
  Status InsertDocFromJson(const std::string_view &json) const;
  Status SelectAll(std::vector<bsoncxx::document::view> &people) const;
  Status DeleteDocByPersonID(const std::string &id) const;
  Status DeleteDocByPersonName(const std::string &name) const;
  Status SellectAllVectors(std::vector<EmbeddingVector> &embeddingvectors,
                           const std::string &engine) const;
  Status SelectPersonFromVector(Person &person, const EmbeddingVector &embedding_vector,
                                const std::string &engine) const;
  ~Database(){}

 private:
  bool exists(std::string tableName);
  mongocxx::client client;
  mongocxx::database PersonDB; 
  std::vector<std::string> PersonFields;
  std::vector<std::string> FaceFields;
  std::vector<std::string> FaceEmbeddingFields;
};