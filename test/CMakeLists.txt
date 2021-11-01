set(APP_NAME persondb)
file(GLOB HEADER_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB SOURCE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")


find_package(faiss)
find_package(OpenMP REQUIRED)
find_package(CUDA REQUIRED)


add_executable(faiss_test faiss_test.cc)
target_compile_features(faiss_test PUBLIC cxx_std_20)
target_include_directories(faiss_test PUBLIC ${CPLUS_INCLUDE_PATH}/include INTERFACE ${CUDA_INCLUDE_DIRS})
target_link_libraries(faiss_test PUBLIC faiss ${CPLUS_INCLUDE_PATH}/lib persondb INTERFACE ${CUDA_LIBRARIES})

add_executable(Insert2DB Insert2DB.cc)
target_compile_features(Insert2DB PUBLIC cxx_std_20)
target_link_libraries(Insert2DB PUBLIC persondb)

add_executable(Query2DB Query2DB.cc)
target_compile_features(Query2DB PUBLIC cxx_std_20)
target_link_libraries(Query2DB PUBLIC persondb)