clang++ -g -std=c++17 -stdlib=libc++ ../src/lexer.cpp ../src/parser.cpp ../src/codegen.cpp ./codegen_test.cpp `/usr/local/opt/llvm/bin/llvm-config --cppflags --ldflags --system-libs --libs core mcjit native orcjit` -o codegen.test