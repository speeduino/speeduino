#!/bin/bash

script_folder="$(dirname $(readlink -f $0))"

# Initialize variables with defaults
source_folder="$script_folder/../speeduino" # -s, --source
file_exts="cpp,ino"                         # -e, --exts
out_folder="$script_folder/.results"        # -o, --out
cppcheck_path=""                            # -c, --cppcheck
quiet=0                                     # -q, --quiet
output_xml=0                                # -x | --xml

cppcheck_parameters=( --inline-suppr
                      --language=c++
                      --addon="$script_folder/misra.json"
                      --suppressions-list="$script_folder/suppressions.txt"
                      --platform=avr8
                      -DCORE_AVR=1
                      -D__AVR_ATmega2560__
                      # All violations from included libraries (*src* folders) are ignored
                      --suppress="\"*:*src*\"")

function parse_command_line() {
   while [ $# -gt 0 ] ; do
    case "$1" in
      -s | --source) source_folder="$2" ;;
      -e | --exts) file_exts="$2" ;;
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

function get_source_files() {
  local source_files=() 
  for i in ${file_exts//,/ }; do
    local file_list=($source_folder/*.$i)
    source_files+=(${file_list[@]})
  done
  
  echo ${source_files[@]}
}

function run_cppcheck() {
  # There is no way to tell the misra add on to skip certain headers
  # libdivide adds 10+ minutes to each file so rename the folder 
  # before the scan
  mv "$source_folder"/src/libdivide "$source_folder"/src/_libdivide
  shopt -s nullglob nocaseglob
  
  for i in "${@}"; do
    # Must redirect stdout to /dev/null else the cppcheck output is mixed in
    # with the function output (I.e. returned to caller)
    # Print current file to stderr instead.
    >&2 echo "Checking " $i

    out_file="$out_folder/$(basename $i).tmp"
    rm -f $out_file
    echo $out_file
    "$cppcheck_bin" ${cppcheck_parameters[@]} --output-file=$out_file $i > /dev/null
  done

  shopt -u nullglob nocaseglob
  # Restore libdivide folder name after scan
  mv "$source_folder"/src/_libdivide "$source_folder"/src/libdivide
}

function merge_txt_files() {
  local merged_file="$out_folder/merged.txt.tmp"

  rm -f $merged_file
  for i in "${@}"; do
    cat $i >> $merged_file
  done

  echo "$merged_file"
}

function process_txt_results() {

  local merged_file=$(merge_txt_files "${@}")

  local out_file="$out_folder/results.txt"
  rm -f $out_file

  # It's grouped into chunks of 3 lines: fold those into one line
  sed '$!N;$!N;s/\n/~/g' < "$merged_file" |\
    # Remove duplicate lines
    sort | uniq | \
    # Unfold the line groups for readability
    tr '~' '\n' > "$out_file"

  echo $out_file
}

function merge_xml_files() {
  local merged_file="$out_folder/merged.xml.tmp"
  rm -f $merged_file

  echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?><errors>" > $merged_file
  for i in "${@}"; do
      xmllint --xpath "/results/errors/*" $i >> $merged_file
  done
  echo "</errors>" >> $merged_file

  echo "$merged_file"
}


function process_xml_results() {
  local out_file="$out_folder/results.xml"
  rm -f $out_file

  xsltproc "$script_folder/flatten_merged.xsl" $(merge_xml_files "${@}") > $out_file
  
  echo "$out_file"
}

parse_command_line "$@"

cppcheck_bin="${cppcheck_path}/cppcheck"
cppcheck_misra="${cppcheck_path}/addons/misra.py"

mkdir -p "$out_folder"

if [ $output_xml -eq 1 ]; then
  cppcheck_parameters+=(--xml)
fi

source_files=($(get_source_files))
result_files=($(run_cppcheck ${source_files[@]}))
if [ $output_xml -eq 1 ]; then
  out_file=$(process_xml_results ${result_files[@]})
else
  out_file=$(process_txt_results ${result_files[@]})
fi

# Count lines for Mandatory or Required rules
error_count=`grep -i "Mandatory - \|Required - " < "$out_file" | wc -l`

if [ $quiet -eq 0 ]; then
  cat "$out_file"
fi
echo $error_count MISRA violations
echo $error_count > "$out_folder/error_count.txt"

exit 0