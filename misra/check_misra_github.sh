#!/bin/bash

cppcheck_path=cppcheck_github/
cppcheck_bin="${cppcheck_path}cppcheck"
#cppcheck_bin="cppcheck"
cppcheck_misra="${cppcheck_path}addons/misra.py"

if [ -f ./results.txt ]; then
	rm results.txt
fi

for i in speeduino/speeduino/*.ino; do
	$cppcheck_bin --dump --max-configs=1 --inline-suppr --suppressions-list=speeduino/misra/suppressions.txt --suppress=syntaxError:speeduino/speeduino/src/PID_v1/PID_v1.h --include=${i%.*}.h -DCORE_AVR=1 -D__AVR_ATmega2560__=1 $i > /dev/null
done

mv speeduino/speeduino/*.dump ./
rm ./utils.*.dump

python $cppcheck_misra --rule-texts=speeduino/misra/misra_2012_text.txt *.dump 2> results.txt
#rm *.dump

cat results.txt
# wc -l results.txt

errors=`wc -l < results.txt | tr -d ' '`
echo $errors MISRA violations

if [ $errors -gt 0 ]; then
	exit 1
else
	exit 0
fi
