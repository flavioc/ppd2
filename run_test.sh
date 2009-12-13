#!/bin/sh

DIR=results

run_prog()
{
  PROGRAM="$1"
  TEST_FILE="$2"
  RESULT="$3"

  echo "$PROGRAM < $TEST_FILE > $DIR/$RESULT"

  { time $PROGRAM < $TEST_FILE; } &> $DIR/$RESULT
}

run_test()
{
  FILE=$1
  PATH="files/$FILE"

  run_prog "./seq" "$PATH" "sequential" || exit 1
  run_prog "./simulator 1" "$PATH" "1thread" || exit 1
  run_prog "./simulator 2" "$PATH" "2thread" || exit 1
  run_prog "./simulator 4" "$PATH" "4thread" || exit 1
}

mkdir -p $DIR
run_test teste1 || exit 1
run_test teste2 || exit 1
run_test teste3 || exit 1

