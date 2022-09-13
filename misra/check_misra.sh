#!/bin/bash

script_folder="$(dirname $(readlink -f $0))"

# Initialize variables with defaults
source_folder="$script_folder/../speeduino" # -s, --source
file_exts="ino"                             # -e, --exts
out_folder="$script_folder/.results"        # -o, --out
cppcheck_path=""                            # -c, --cppcheck
quiet=0                                     # -q, --quiet

function parse_command_line() {
   while [ $# -gt 0 ] ; do
    case "$1" in
      -s | --source) source_folder="$2" ;;
      -e | --exts) file_exts="$2" ;;
      -o | --out) out_folder="$2" ;;
      -c | --cppcheck) cppcheck_path="$2" ;;
      -q | --quiet) quiet=1 ;;
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
    # cppcheck currently has no way of excluding files that are #include'd. If maths.ino is scanned on versions of cppcheck 2.8+, the scan will run for a significant period of time (15+ mins) due to all the static code from libdivide. 
    # All violations from included libraries (*src* folders) are ignored
    if [[ $i != *"maths.ino"* ]]; then
      "$cppcheck_bin" \
          --inline-suppr \
          --language=c++ \
          --addon="$script_folder/misra.json" \
          --suppressions-list="$script_folder/suppressions.txt" \
          --platform=avr8 \
          -DCORE_AVR=1 \
          -D__AVR_ATmega2560__ \
          --suppress="*:*src*" \
          --report-progress \
          $i 2>> "$cpp_result_file"
    fi
  done
  shopt -u nullglob nocaseglob
}

function process_cpp_results() {
  local intermediate_file="$out_folder/tmp.txt"

  # It's grouped into chunks of 3 lines: fold those into one line
  sed '$!N;$!N;s/\n/~/g' < "$cpp_result_file" |\
    # Remove duplicate lines
    sort | uniq > "$intermediate_file"
  # Count lines for Mandatory or Required rules
  local __error_count=`grep -i "Mandatory\|Required" < "$intermediate_file" | wc -l`
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

if [ $quiet -eq 0 ]; then
  cat "$result_file"
fi
echo $error_count MISRA violations
echo $error_count > ".results/error_count.txt"

exit 0