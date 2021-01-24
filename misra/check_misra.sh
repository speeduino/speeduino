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

function run_cppcheck() {
  shopt -s nullglob nocaseglob
  for i in "$source_folder"/*.{"$file_exts",}; do
    "$cppcheck_bin" \
        --inline-suppr \
        --language=c++ \
        --addon="$script_folder/misra.json" \
        --suppressions-list="$script_folder/suppressions.txt" \
        -DCORE_AVR=1 \
        -D__AVR_ATmega2560__ \
        $i 2>> "$cpp_result_file"
  done
  shopt -u nullglob nocaseglob
}

function process_cpp_results() {
  local intermediate_file="$out_folder/tmp.txt"

  # It's grouped into chunks of 3 lines: fold those into one line
  sed '$!N;$!N;s/\n/~/g' < "$cpp_result_file" |\
    # Remove duplicate lines
    sort | uniq > "$intermediate_file"
  # Count error lines
  local __error_count=`grep ": error:" < "$intermediate_file" | wc -l`
  # Unfold the line groups for readability
  tr '~' '\n' < "$intermediate_file" > "$result_file"
  rm -f "$intermediate_file"

  echo "$__error_count"
}

parse_command_line "$@"

cppcheck_bin="${cppcheck_path}/cppcheck"
cppcheck_misra="${cppcheck_path}/addons/misra.py"
cpp_result_file="$out_folder/cpp_results.txt"
result_file="$out_folder/results.txt"

mkdir -p "$out_folder"
rm -f "$cpp_result_file"
rm -f "$result_file"

run_cppcheck
error_count="$(process_cpp_results)"

cat "$result_file"
echo $error_count MISRA violations

if [ $error_count -gt 0 ]; then
	exit 1
else
	exit 0
fi
