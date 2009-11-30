#!/bin/sh

FILE=file1.txt

time ./seq < files/$FILE > st
time ./simulator 1 < files/$FILE > mt
diff -u st mt
