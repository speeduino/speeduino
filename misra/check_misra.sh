#!/bin/bash

script_folder="$(dirname $(readlink -f $0))"

# Initialize variables with defaults
source_folder="$script_folder/../speeduino" # -s, --source
file_exts="ino"                             # -e, --exts
out_folder="$script_folder/.results"        # -o, --out
cppcheck_path=""                            # -c, --cppcheck

function parse_command_line() {
   while [ $# -gt 0 ] ; do
    case "$1" in
      -s | --source) source_folder="$2" ;;
      -e | --exts) file_exts="$2" ;;
      -o | --out) out_folder="$2" ;;
      -c | --cppcheck) cppcheck_path="$2" ;;
      -*) 
        echo "Unknown option: " $1
        exit 1
        ;;
    esac
    shift
  done
}

parse_command_line "$@"

cppcheck_bin="${cppcheck_path}/cppcheck"
cppcheck_misra="${cppcheck_path}/addons/misra.py"
cpp_result_file="$out_folder/cpp_results.txt"
result_file="$out_folder/results.txt"

mkdir -p "$out_folder"
rm -f "$cpp_result_file"
rm -f "$result_file"

shopt -s nullglob nocaseglob
for i in "$source_folder"/*.{"$file_exts",}; do
  "$cppcheck_bin" --addon="$script_folder/misra.json" --suppress=syntaxError:"$source_folder"/src/PID_v1/PID_v1.h -DCORE_AVR=1 -D__AVR_ATmega2560__ $i 2>> "$cpp_result_file"
done
shopt -u nullglob nocaseglob

# Process cppcheck results file: 
# It's grouped into chunks of 3 lines: fold those into one line
sed '$!N;$!N;s/\n/~/g' < "$cpp_result_file" |\
 # Remove duplicate lines
 sort | uniq > "$out_folder/tmp.txt"
# Number of lines == unique errors
error_count=`wc -l < "$out_folder/tmp.txt"`
# Unfold the line groups for readability
tr '~' '\n' < "$out_folder/tmp.txt" > "$result_file"
rm -f "$out_folder/tmp.txt"

cat "$result_file"
echo $error_count MISRA violations

if [ $error_count -gt 0 ]; then
	exit 1
else
	exit 0
fi
