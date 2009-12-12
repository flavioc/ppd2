#!/bin/sh

FILE=file2.txt

time ./seq < files/$FILE &> st >
time ./simulator 4 < files/$FILE &> mt
diff -u st mt
