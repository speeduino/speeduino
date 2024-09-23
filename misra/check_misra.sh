#!/bin/bash
# uncomment to echo *fully* expanded script commands to terminal
# set -x


get_abs_filename() {
  # $1 : relative filename
  echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
}

script_folder=$(get_abs_filename "$(dirname $(readlink -f $0))")

# Initialize variables with defaults
source_folder="$script_folder/../speeduino" # -s, --source
out_folder="$script_folder/.results"        # -o, --out
cppcheck_path=""                            # -c, --cppcheck
quiet=0                                     # -q, --quiet
output_xml=0                                # -x, --xml

function parse_command_line() {
   while [ $# -gt 0 ] ; do
    case "$1" in
      -s | --source) source_folder="$2" ;;
      -o | --out) out_folder="$2" ;;
      -c | --cppcheck) cppcheck_path="$2" ;;
      -q | --quiet) quiet=1 ;;
      -x | --xml) output_xml=1 ;;
      -*) 
        echo "Unknown option: " $1
        exit 1
        ;;
    esac
    shift
  done
}

parse_command_line "$@"

# Have to use absolute paths for source:
# 1. CPPCheck (or the shell) expands globs to absolute paths.
# 2. CPPCheck matches paths from the command line using simple string comparisons
#   2.1. E.g. exclusion folders
source_folder=$(get_abs_filename "$source_folder")

cppcheck_bin="${cppcheck_path}/cppcheck"
cppcheck_misra="${cppcheck_path}/addons/misra.py"

num_cores=`getconf _NPROCESSORS_ONLN`
let num_cores--

mkdir -p "$out_folder"

cppcheck_parameters=( --inline-suppr
                      --language=c++
                      --enable=warning
                      --enable=information
                      --enable=performance
                      --enable=portability
                      --enable=style
                      --addon="$script_folder/misra.json"
                      --suppressions-list="$script_folder/suppressions.txt"
                      --suppress=unusedFunction:*
                      --suppress=missingInclude:*
                      --suppress=missingIncludeSystem:*
                      --suppress=unmatchedSuppression:*
                      --suppress=cstyleCast:*
                      --platform=avr8
                      --cppcheck-build-dir="$out_folder"
                      -j "$num_cores"
                      -DCORE_AVR=1
                      -D__AVR_ATmega2560__
                      -DARDUINO_AVR_MEGA2560 
                      -DF_CPU=16000000L 
                      -DARDUINO_ARCH_AVR 
                      -DARDUINO=10808 
                      -DAVR=1
                      # This is defined in the AVR headers, which aren't included.
                      # cppcheck will not do type checking on unknown types.
                      # It's used a lot and it's unsigned, which can trigger a lot
                      # of type mismatch violations.
                      -Dbyte=uint8_t
                      # All violations from included libraries (*src* folders) are ignored
                      --suppress="*:$source_folder/src/*"
                      # No libdivide - analysis takes too long
                      -UUSE_LIBDIVIDE
                      # Don't parse the /src folder
                      -i "$source_folder/src"
                      "$source_folder/**.ino"
                      "$source_folder/**.cpp")

cppcheck_out_file="$out_folder/results.txt"
if [ $output_xml -eq 1 ]; then
  cppcheck_out_file="$out_folder/results.xml"
  cppcheck_parameters+=(--xml)
fi

# There is no way to tell the misra add on to skip certain headers
# libdivide adds 10+ minutes to each file so rename the folder 
# before the scan
mv "$source_folder"/src/libdivide "$source_folder"/src/_libdivide

"$cppcheck_bin" ${cppcheck_parameters[@]} 2> $cppcheck_out_file

# Restore libdivide folder name after scan
mv "$source_folder"/src/_libdivide "$source_folder"/src/libdivide

# Count lines for Mandatory or Required rules
error_count=`grep -i "Mandatory - \|Required - " < "$cppcheck_out_file" | wc -l`

if [ $quiet -eq 0 ]; then
  cat "$cppcheck_out_file"
fi
echo $error_count MISRA violations
echo $error_count > "$out_folder/error_count.txt"

exit 0