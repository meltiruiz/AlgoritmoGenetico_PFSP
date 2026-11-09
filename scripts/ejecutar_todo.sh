#!/usr/bin/env bash
# Compila el programa y ejecuta las 3 instancias con la configuracion base (Linux/macOS)
set -e
g++ -std=c++17 -O2 -Iinclude src/*.cpp -o ga_pfsp
for f in instancias/*.txt; do
  ./ga_pfsp "$f"
done
