# builds the compiler and check
build_compiler() {
  # build the compiler with GCC since this is
  # what the final thing will be built with
  pushd compiler
  make clean
  make gcc

  if [ $? -ne 0 ]; then
    echo "Error: failed to build"
    exit 1
  fi

  popd
}

build_programs() {
  # build test programs
  pushd handoutPrograms
  CC="../compiler/cm"

  # blank the build directiory
  rm -rf build
  mkdir -p build

  test_files=$(ls -1 *.cm)
  for file in $test_files; do
    echo "Building: $file"

    name=$(echo "$file" | cut -d'.' -f 1)
    $CC < "$file" > "build/${name}.tm"

    if [ $? -ne 0 ]; then
      echo "Error: failed to build $file!"
      exit 2
    fi
  done

  # go back out
  popd
}

run_programs() {
  # run the built programs, remove existing output
  pushd "handoutPrograms/build"

  test_files=$(ls -1 *.tm)
  rm -rf "$RAW_OUTPUT"

  for file in $test_files; do
    # get the name of the file
    name=$(echo "$file" | cut -d'.' -f 1)

    echo "Running: $file"

    # if there is a text file with the same name, pipe it into the input of the
    # program when we are running
    prefix=""
    if [ -a "../${name}.txt" ]; then
      prefix="echo \"../${name}.txt\" |"
    fi
    $prefix ftm "./$file" >> "$RAW_OUTPUT"

    if [ $? -ne 0 ]; then
      echo "Error: failed to run $file!"
      exit 3
    fi
  done

  # only save value of text OUT
  cat "$RAW_OUTPUT" | grep "OUT instruction prints" > "$OUTPUT"

  # go back out
  popd
}

run_tests() {
  build_compiler
  build_programs
  run_programs

  # compare to expected output
  diff "$EXPECTED_OUTPUT" "$OUTPUT"
}

run_rebuild() {
  build_compiler
  build_programs
  run_programs

  # write the expected output
  echo "Copying output to \"$EXPECTED_OUTPUT\""
  cp "$OUTPUT" "$EXPECTED_OUTPUT"
}

# set the command variable
command=""
if [ $# -eq 0 ]; then
  command="test"
else
  command="$1"
fi

# system constants
RAW_OUTPUT="/tmp/raw-output.txt"
OUTPUT="/tmp/output.txt"
EXPECTED_OUTPUT="handoutPrograms/expected-output.txt"

# run different commands depending on option
case $command in
test)
  run_tests
  ;;
rebuild)
  run_rebuild
  ;;
esac

