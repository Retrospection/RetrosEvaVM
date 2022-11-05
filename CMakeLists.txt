cmake_minimum_required(VERSION 3.20)
project(RetrosEvaVM)

set(CMAKE_CXX_STANDARD 17)

add_executable(RetrosEvaVM main.cpp EvaVM.h OpCode.h Logger.h EvaValue.h parser/EvaParser.h EvaCompiler.h)
