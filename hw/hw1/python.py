import sys

if len(sys.argv) != 2:
    print('usage: python.py [filename]')
    exit()

# definitons
filename = sys.argv[1]

# open the file and read lines
file = open(filename)
lines = file.readlines()

# print out line if it equals "abc123"
for line in lines:
    if line == "abc123\n":
        print(line)
