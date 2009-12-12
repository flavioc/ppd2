#!/bin/sh

FILE=file2.txt

time ./seq < files/$FILE 2> st
time ./simulator 4 < files/$FILE 2> mt
diff -u st mt
