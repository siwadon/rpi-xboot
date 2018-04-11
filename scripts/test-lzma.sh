#!/usr/bin/env sh

script_name=$(basename "$0")

if [[ $# != 1 ]]; then
  echo "Usage: $script_name FILE" 
  exit
fi

make

output_file="$1.tmp"
rm $output_file

# compress
python3 compress.py $1 > $1.lzma

# decompress
./lzma $1.lzma $output_file

# compare
diff $1 $output_file
echo

if [[ $? != 0 ]]; then
  echo "FAILED" 
else
  echo "PASSED"
fi
