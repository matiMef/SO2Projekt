#!/bin/bash

# Nazwa pliku źródłowego
SOURCE_FILE="Problem5Filozofów.cpp"

# Nazwa pliku wykonywalnego
EXECUTABLE="Problem5Filozofów"

# Kompilacja z użyciem standardu C++20
g++ -std=c++20 "$SOURCE_FILE" -o "$EXECUTABLE"

# Sprawdzenie, czy kompilacja zakończyła się sukcesem
if [ $? -eq 0 ]; then
    echo "Kompilacja zakończona sukcesem."

    # Nadanie praw do wykonania
    chmod +x "$EXECUTABLE"

    # Uruchomienie programu
    ./"$EXECUTABLE"
else
    echo "Błąd podczas kompilacji."
fi

