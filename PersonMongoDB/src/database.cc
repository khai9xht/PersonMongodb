#include "persondb/database.h"

#include <bsoncxx/builder/list.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <iostream>
#include <string>

#include "bsoncxx/array/element.hpp"
#include "bsoncxx/array/view.hpp"
#include "bsoncxx/builder/basic/array.hpp"
#include "bsoncxx/builder/basic/document.hpp"
#include "bsoncxx/builder/basic/kvp.hpp"
#include "bsoncxx/builder/basic/sub_array.hpp"
#include "bsoncxx/builder/basic/sub_document.hpp"
#include "bsoncxx/builder/stream/helpers.hpp"
#include "bsoncxx/document/element.hpp"
#include "bsoncxx/document/value.hpp"
#include "bsoncxx/document/view.hpp"
#include "bsoncxx/stdx/optional.hpp"
#include "bsoncxx/stdx/string_view.hpp"
#include "mongocxx/client.hpp"
#include "mongocxx/cursor.hpp"
#include "mongocxx/result/insert_one.hpp"
#include "mongocxx/uri.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;


Database::Database() {}

void Database::Initialize(const std::string &str_uri) {
  mongocxx::instance instance;
  mongocxx::uri uri(str_uri);
  client = mongocxx::client{uri};
  PersonDB = client["Person_representation"];
}

Status Database::InsertDocFromJson(const std::string_view &json) const {
  bsoncxx::stdx::string_view jsondoc(json.data());
  auto doc = bsoncxx::from_json(jsondoc);
  bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
      PersonDB["person"].insert_one(doc.view());
  return Status::OK;
}

Status Database::insertPerson(const Person &person) const {
  auto doc = person.getDoc();
  auto res = PersonDB["person"].insert_one(doc.view());
  return Status::OK;
}

Status Database::SelectAll(std::vector<bsoncxx::document::view> &people) const {
  mongocxx::cursor cursor = PersonDB["person"].find({});
  for (auto doc : cursor) {
    people.push_back(doc);
  }
  return Status::OK;
}

Status Database::DeleteDocByPersonID(const std::string &id) const {
  bsoncxx::stdx::optional<mongocxx::result::delete_result> result =
      PersonDB["person"].delete_many(document{} << "id" << id << finalize);
  return Status::OK;
}

Status Database::DeleteDocByPersonName(const std::string &name) const {
  bsoncxx::stdx::optional<mongocxx::result::delete_result> result =
      PersonDB["person"].delete_many(document{} << "name" << name << finalize);
  return Status::OK;
}

Status Database::SellectAllVectors(
              std::vector<EmbeddingVector> &embeddingvectors, 
              const std::string &engine) const {
  mongocxx::cursor cursor = PersonDB["person"].find({});
  for (auto doc : cursor) {
    bsoncxx::array::view faces = doc["faces"].get_array().value;
    auto face_it = faces.begin();
    while (face_it != faces.end()) {
      bsoncxx::document::element vectors = (*face_it)["vectors"];
      auto vector_it = vectors.get_array().value.begin();
      while (vector_it != vectors.get_array().value.end()) {
        bsoncxx::document::element eng_element = (*vector_it)["engine"];
        std::string eng = eng_element.get_string().value.to_string();
        if (eng == engine) {
          bsoncxx::array::view embed_vector = ((*vector_it))["value"].get_array();
          bsoncxx::array::view::const_iterator it = embed_vector.begin();
          std::vector<double> value;
          while (it != embed_vector.end()) {
            double x = (*it).get_double().value;
            value.push_back(x);
            it++;
          }
          EmbeddingVector emb_vector(eng, value);
          embeddingvectors.push_back(emb_vector);
        }
        vector_it ++;
      }
      face_it++;
    }
  }
  return Status::OK;
}

Status Database::SelectPersonFromVector(Person &person,
                              const EmbeddingVector &embedding_vector,
                              const std::string &engine) const
{
  bsoncxx::builder::basic::array emb_arr;
  auto it = embedding_vector.value.begin();
  while (it != embedding_vector.value.end()) {
    emb_arr.append(*it);
    it++;
  }
  auto cursor = PersonDB["person"].find(make_document(
    kvp("faces", make_document(
      kvp("vectors", make_document(
        kvp("$elemMatch", make_document(
          kvp("engine", engine), kvp("value", emb_arr)
        ))
      ))
    ))
  ));
  if (cursor.begin() == cursor.end()){
    return Status::NOT_FOUND;
  }
  for (auto doc : cursor){

  }
  return Status::OK;
}