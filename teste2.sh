#!/bin/sh

FILE=file1.txt

time ./simulator 1 < files/$FILE > mt1
time ./simulator 2 < files/$FILE > mt2
diff -u mt1 mt2
