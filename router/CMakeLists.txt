cmake_minimum_required(VERSION 3.16)
project(ai_router)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.70 REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

add_executable(ai_router src/main.cpp src/proxy.cpp src/metrics.cpp)

target_include_directories(ai_router PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(ai_router PRIVATE
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)
