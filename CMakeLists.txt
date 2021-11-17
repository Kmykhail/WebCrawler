cmake_minimum_required(VERSION 3.19)
project(WebCrawler)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS -pthread)

add_executable(WebCrawler main.cpp PrimitiveHttpClient.cpp PrimitiveHttpClient.h Crawler.cpp Crawler.h)

target_link_libraries(WebCrawler -lcurl)