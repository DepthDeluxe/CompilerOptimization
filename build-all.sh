#!/bin/bash

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
  debug='/tmp/debug.txt'

  # run the built programs, remove existing output
  pushd "handoutPrograms/build"

  test_files=$(ls -1 *.tm)
  rm -rf "$OUTPUT" "$PROFILE" "$debug"

  for file in $test_files; do
    # get the name of the file
    name=$(echo "$file" | cut -d'.' -f 1)

    echo "Running: $file"

    # if there is a text file with the same name, pipe it into the input of the
    # program when we are running
    redirect=""
    if [ -a "../${name}.txt" ]; then
      redirect="echo \"../${name}.txt\" | "
    fi
    thisOutput=$($redirect ftm "./$file")

    echo "$thisOutput" >>$debug

    # filter out needed information from the output
    echo "$thisOutput" | grep "OUT instruction prints" >> "$OUTPUT"
    echo $thisOutput | egrep -o '[0-9]+ were NOPs --> [0-9]+' | sed -e 's/[^0-9 ]//g' -e 's/    /;/g' -e "s/^/$name;/" >> "$PROFILE"
  done

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

run_profile() {
  run_tests

  oldIFS="$IFS"

  # build up the profile
  IFS=$'\n'
  profileData=$(cat "$PROFILE")
  profileDataArray=($profileData)

  # print out header
  printf "%-15s %-10s %-10s %-10s %-10s %-10s\n" 'program' 'before' 'after' 'nops after' 'real after' 'pct gone'
  echo "-----------------------------------------------------------------------------"

  # print out profile data for each program
  for line in ${profileDataArray[@]}; do
    IFS=';'

    # put parameters into array
    lineArgs=($line)

    # print out the information about program
    printf "%-15s %-10s %-10d %-10d %-10d %-10s\n" \
      "${lineArgs[0]}" \
      '--' \
      "$(expr ${lineArgs[1]} + ${lineArgs[2]})" \
      "${lineArgs[1]}" \
      "${lineArgs[2]}" \
      '--'
  done

  # restore the old IFS so we can keep iterating through $profileDataArray
  IFS="$oldIFS"
}

# set the command variable
command=""
if [ $# -eq 0 ]; then
  command="test"
else
  command="$1"
fi

# system constants
EXPECTED_OUTPUT='handoutPrograms/expected-output.txt'
OUTPUT='/tmp/output.txt'
PROFILE='/tmp/profile.txt'

# run different commands depending on option
case $command in
test)
  run_tests
  ;;
rebuild)
  run_rebuild
  ;;
profile)
  run_profile
  ;;
esac

