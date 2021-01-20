#!/bin/bash

# Initialize variables with defaults
source_folder="speeduino/speeduino" # -s, --source
file_exts="ino"                     # -e, --exts
out_folder="."                      # -o, --out
cppcheck_path=""                    # -c, --cppcheck

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
script_folder="$(dirname $(readlink -f $0))"

if [ -f "$out_folder"/results.txt ]; then
	rm "$out_folder"/results.txt
fi

shopt -s nullglob nocaseglob
for i in "$source_folder"/*.{"$file_exts",}; do
  #cppcheck --xml --include=${i%.*}.h --include=speeduino/speeduino/globals.h $i > /dev/null
  #cppcheck --force --dump --suppress=syntaxError:speeduino/speeduino/src/PID_v1/PID_v1.h --include=${i%.*}.h --include=speeduino/speeduino/globals.h -DCORE_AVR=1 -USTM32F4 $i > /dev/null
  "$cppcheck_bin" --dump --suppress=syntaxError:"$source_folder"/src/PID_v1/PID_v1.h -DCORE_AVR=1 -D__AVR_ATmega2560__ $i > /dev/null
done
shopt -u nullglob nocaseglob

mv "$source_folder"/*.dump "$out_folder"/
#rm ./utils.*.dump

python "$cppcheck_misra" --rule-texts="$script_folder"/misra_2012_text.txt "$out_folder"/*.dump 2> "$out_folder"/results.txt
#rm "$out_folder"/*.dump

cat "$out_folder"/results.txt
# wc -l "$out_folder"/results.txt

errors=`wc -l < "$out_folder"/results.txt | tr -d ' '`
echo $errors MISRA violations

if [ $errors -gt 0 ]; then
	exit 1
else
	exit 0
fi
