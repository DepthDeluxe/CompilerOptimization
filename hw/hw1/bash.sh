folder=folder
mkdir -p $folder

cd $folder
cd ..

echo "abc123" > testing123.txt
echo "something else" >> testing123.txt

first="testing123.txt"
second="another.txt"

cp "$first" "$second"
rm -f $first

ls -1
ls -1 | grep .txt

echo "python script"
python python.py $second

