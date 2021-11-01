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
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::document; 

EmbeddingVector::EmbeddingVector(
    const std::string &_engine, const std::vector<double> &_value) {
  engine = _engine;
  value = _value;
}

Face::Face(const std::string &_id, const std::string &_imgPath,
           std::vector<EmbeddingVector> &_vectors) {
  id = _id;
  imgPath = _imgPath;
  vectors = _vectors;
}

Person::Person(const std::string &_id, const std::string &_name,
               const std::string &_outsideID, std::vector<Face> &_faces) {
  id = _id;
  name = _name;
  outsideID = _outsideID;
  faces = _faces;
}

bsoncxx::document::value EmbeddingVector::getSubDoc() const {
  auto embed_vector = bsoncxx::builder::basic::document{};
  embed_vector.append(kvp("engine", engine),
                      kvp("value", [this](sub_array subarr) {
                        auto it = value.begin();
                        while (it != value.end()) {
                          subarr.append(bsoncxx::types::b_double{*it});
                          it++;
                        }
                      }));
  return embed_vector.extract();
}

bsoncxx::document::value Face::getSubDoc() const {
  auto doc = bsoncxx::builder::basic::document{};
  doc.append(kvp("id", id), kvp("imgPath", imgPath),
             kvp("vectors", [this](sub_array subarr) {
               auto it = vectors.begin();
               while (it != vectors.end()) {
                 subarr.append(it->getSubDoc());
                 it++;
               }
             }));
  return doc.extract();
}

bsoncxx::document::value Person::getDoc() const {
  auto doc = bsoncxx::builder::basic::document{};
  doc.append(kvp("id", id), kvp("name", name), kvp("outsideID", outsideID),
             kvp("faces", [this](sub_array subarr) {
               auto it = faces.begin();
               while (it != faces.end()) {
                 subarr.append(it->getSubDoc());
                 it++;
               }
             }));
  return doc.extract();
}

Database::Database() {}

void Database::Initialize(std::string &str_uri) {
  mongocxx::instance instance;
  mongocxx::uri uri(str_uri);
  client = mongocxx::client{uri};
  PersonDB = client["Person_representation"];
}

Status Database::InsertDocFromJson(std::string_view &json) const {
  bsoncxx::stdx::string_view jsondoc(json.data());
  auto doc = bsoncxx::from_json(jsondoc);
  bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
      PersonDB["person"].insert_one(doc.view());
  return Status::OK;
}

Status Database::insertPerson(Person &person) const {
  auto doc = person.getDoc();
  auto res = PersonDB["person"].insert_one(doc.view());
  return Status::OK;
}

Status Database::SelectAll(std::vector<bsoncxx::document::view> &people) const {
  mongocxx::cursor cursor = PersonDB["person"].find({});
  for (auto doc : cursor){
    people.push_back(doc);
  }
  return Status::OK;
}

Status Database::DeleteDocByPersonID(std::string &id) const {
  bsoncxx::stdx::optional<mongocxx::result::delete_result> result =
      PersonDB["person"].delete_many(document{} << "id" << id << finalize);
  return Status::OK; 
}

Status Database::DeleteDocByPersonName(std::string &name) const {
  bsoncxx::stdx::optional<mongocxx::result::delete_result> result =
      PersonDB["person"].delete_many(document{} << "name" << name << finalize);
  return Status::OK; 
}

Status Database::SellectAllVectors(std::vector<EmbeddingVector> &embeddingvectors, std::string &engine){
  mongocxx::cursor cursor = PersonDB["person"].find({});
  for (auto doc : cursor){
    bsoncxx::document::element faces = doc["faces"];
    int length = faces.length();
    for(int i = 0; i < length; i++){
      bsoncxx::array::element face = faces[i];
      bsoncxx::document::element vectors = face["vectors"];
      for(int j = 0; j < vectors.length(); j++){
        bsoncxx::array::element vector = vectors[j];

        bsoncxx::document::element eng_element = vector["engine"];
        std::string eng = eng_element.get_string().value.to_string();
        if(eng == engine){
          bsoncxx::array::view embed_vector = vector["value"].get_array();
          bsoncxx::array::view::const_iterator it = embed_vector.begin();
          std::vector<double> value;
          while (it != embed_vector.end()) {
            double x = (*it).get_double().value;
            value.push_back(x);
            it++;
          }
          embeddingvectors.push_back(EmbeddingVector(engine, value));
        }

      }
    }
  }
  return Status::OK;
}