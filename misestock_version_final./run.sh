#!/bin/bash
cd "$(dirname "$0")"

if [ ! -x build/misestock ]; then
    echo "No existe el ejecutable build/misestock."
    echo "Ejecutar primero: ./build.sh"
    exit 1
fi

./build/misestock
