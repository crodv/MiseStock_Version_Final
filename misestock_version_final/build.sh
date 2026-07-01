#!/bin/bash
set -e

cd "$(dirname "$0")"

mkdir -p build
cd build

if command -v qmake6 >/dev/null 2>&1; then
    QMAKE=qmake6
elif command -v qmake >/dev/null 2>&1; then
    QMAKE=qmake
else
    echo "Error: no se encontró qmake ni qmake6."
    exit 1
fi

$QMAKE ../misestock.pro CONFIG+=release
make -j"$(nproc)"

echo "Compilación finalizada."
echo "Ejecutable generado en: build/misestock"
