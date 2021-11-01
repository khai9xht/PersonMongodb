#include <faiss/IndexFlat.h>
#include <faiss/IndexIVFPQ.h>
#include <faiss/gpu/GpuIndexFlat.h>
#include <faiss/gpu/GpuIndexIVFFlat.h>
#include <faiss/gpu/StandardGpuResources.h>
#include <faiss/index_io.h>

#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <random>
#include <string>
#include <vector>

#include "bsoncxx/array/value.hpp"
#include "bsoncxx/array/view.hpp"
#include "bsoncxx/document/element.hpp"

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

int main(int argc, char *argv[]) {
  std::vector<float> data;

  mongocxx::instance instance{};
  mongocxx::uri uri("mongodb://192.168.50.17:9876");
  mongocxx::client client(uri);
  mongocxx::database db = client["person_info"];
  mongocxx::collection coll = db["face"];

  mongocxx::cursor cursor = coll.find({});
  for (auto doc : cursor) {
    bsoncxx::document::element element = doc["name"];
    std::string name = element.get_string().value.to_string();

    bsoncxx::document::element embed_element = doc["embedded"];
    bsoncxx::array::view embedded = embed_element.get_array();
    bsoncxx::array::view::const_iterator x = embedded.begin();
    while (x != embedded.end()) {
      double y = (*x).get_double().value;
      // std::cout << y << " ";
      data.push_back(y);
      x++;
    }
  }
  int dim = 512;
  int nb = 10000;
  int nb_1 = data.size() / dim;
  int nb_2 = nb - nb_1;
  int nq = 1;
  int k = 5;
  std::mt19937 rng;
  std::uniform_real_distribution<> distrib;
  float *xb = new float[nb * dim];
  float *xq = new float[dim * nq];
  for (int i = 0; i < data.size(); i++) {
    xb[i] = static_cast<float>(data[i]);
  }
  for (int i = nb_1; i < nb; i++) {
    for (int j = 0; j > dim; j++) {
      xb[dim * i + j] = distrib(rng);
    }
    xb[dim * i] += i / 1000.;
  }

  for (int i = 0; i < nq; i++) {
    for (int j = 0; j < dim; j++) xq[dim * i + j] = drand48();
    xq[dim * i] += i / 1000.;
  }
  std::cout << "nb: " << nb << std::endl;

  faiss::gpu::StandardGpuResources res;
  faiss::gpu::GpuIndexFlatL2 index_flat(&res, dim);
  index_flat.add(nb, xb);

  {
    long *I = new long[k * nq];
    float *D = new float[k * nq];
    auto start = std::chrono::steady_clock::now();
    index_flat.search(nq, xq, k, D, I);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> inference_time = end - start;
    std::cout << "Inference time: " << inference_time.count() << std::endl;
    printf("I=\n");
    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < k; j++) printf("%5ld ", I[i * k + j]);
      printf("\n");
    }
    delete[] I;
    delete[] D;
  }

  delete[] xb;
  return 0;
}