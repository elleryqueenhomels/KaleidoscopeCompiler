clang++ -g -std=c++17 -stdlib=libc++ src/lexer.cpp src/parser.cpp src/codegen.cpp src/main.cpp `/usr/local/opt/llvm/bin/llvm-config --cppflags --ldflags --system-libs --libs core executionengine interpreter mc support nativecodegen` -o main.test