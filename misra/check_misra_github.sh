#!/bin/bash

cppcheck_path=cppcheck_github/
cppcheck_bin="${cppcheck_path}cppcheck"
cppcheck_misra="${cppcheck_path}addons/misra.py"

#Uncomment below to use the Brew installation of cppcheck on Mac
#cppcheck_path_brew=/usr/local/Cellar/cppcheck/2.1/share/cppcheck/
#cppcheck_misra="${cppcheck_path_brew}addons/misra.py"
#cppcheck_bin=/usr/local/Cellar/cppcheck/2.1/bin/cppcheck



if [ -f ./results.txt ]; then
	rm results.txt
fi

for i in speeduino/speeduino/*.ino; do
	$cppcheck_bin --dump --max-configs=1 --inline-suppr --suppressions-list=speeduino/misra/suppressions.txt --language=c++ --include=${i%.*}.h -DCORE_AVR=1 -D__AVR_ATmega2560__=1 $i > /dev/null
done

mv speeduino/speeduino/*.dump ./
rm ./utils.*.dump

python $cppcheck_misra --rule-texts=speeduino/misra/misra_2012_text.txt *.dump 2> results.txt
#rm *.dump

sort results.txt | uniq
# wc -l results.txt

errors=`sort results.txt | uniq | wc -l`
echo $errors MISRA violations

if [ $errors -gt 0 ]; then
	exit 1
else
	exit 0
fi
