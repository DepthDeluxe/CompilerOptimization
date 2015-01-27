# build the compiler
pushd compiler
make

if [ $? -ne 0 ]; then
  echo "Error: failed to build"
  exit 1
fi

# build test programs
popd
pushd handoutPrograms
CC="../compiler/cm"

# blank the build directiory
rm -rf build
mkdir -p build

test_files=$(ls -1 *.cm)
for file in $test_files; do
  name=$(echo "$file" | cut -d'.' -f 1)
  $CC < "$file" > "build/${name}.tm"

  if [ $? -ne 0 ]; then
    echo "Error: failed to build $file!"
    exit 2
  fi
done

# run the built programs, remove existing output
pushd build
test_files=$(ls -1 *.tm)
rm -rf raw-output.txt

for file in $test_files; do
  # get the name of the file
  name=$(echo "$file" | cut -d'.' -f 1)

  # if there is a text file with the same name, pipe it into the input of the
  # program when we are running
  prefix=""
  if [ -a "../${name}.txt" ]; then
    prefix="echo \"../${name}.txt\" |"
  fi
  $prefix ftm "./$file" >> raw-output.txt

  if [ $? -ne 0 ]; then
    echo "Error: failed to run $file!"
    exit 3
  fi
done
# only save value of text OUT
cat raw-output.txt | grep "OUT instruction prints" > output.txt

# compare to expected output
diff ../expected-output.txt output.txt
