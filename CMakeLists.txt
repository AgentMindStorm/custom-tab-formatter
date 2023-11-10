cmake_minimum_required(VERSION 3.26)
project(ChangelogFormatter)

set(CMAKE_CXX_STANDARD 11)

configure_file(changelog.txt changelog.txt COPYONLY)

add_executable(ChangelogFormatter main.cpp)
