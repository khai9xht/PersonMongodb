#include "persondb/db_structure.h"

#include "bsoncxx/builder/basic/kvp.hpp"
#include "bsoncxx/builder/basic/sub_array.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::sub_array;

EmbeddingVector::EmbeddingVector(const std::string &_engine,
                                 const std::vector<double> &_value) {
  engine = _engine;
  value = _value;
}
EmbeddingVector::EmbeddingVector(const bsoncxx::builder::basic::document &doc) {
  auto doc_view = doc.view();
  bsoncxx::document::element eng_element = doc_view["engine"];
  engine = eng_element.get_string().value.to_string();

  bsoncxx::array::view value_arr = doc_view["value"].get_array();
  auto value_it = value_arr.begin();
  while (value_it != value_arr.end()) {
    double x = (*value_it).get_double().value;
    value.push_back(x);
    value_it++;
  }
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