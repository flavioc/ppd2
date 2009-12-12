#!/bin/sh

FILE=file1.txt

time ./simulator 1 < files/$FILE 2> mt1
time ./simulator 4 < files/$FILE 2> mt2
diff -u mt1 mt2
