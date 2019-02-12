if [ -f ./results.txt ]; then
	rm results.txt
fi

cd speeduino/speeduino
#cppcheck --dump --inline-suppr --suppress=syntaxError:src/PID_v1/PID_v1.h --suppressions-list=../misra/suppressions.txt --include=./*.h -DCORE_AVR=1 -D__AVR_ATmega2560__ -U__STM32F1__ -USTM32F4 ./*.ino > /dev/null
cppcheck --dump --inline-suppr --suppress=syntaxError:src/PID_v1/PID_v1.h --suppressions-list=../misra/suppressions.txt --include=./*.h -DCORE_AVR=1 -D__AVR_ATmega2560__ -U__STM32F1__ -USTM32F4 -UCORE_STM32 -UCORE_TEENSY ./storage.ino > /dev/null
cd ../..
mv speeduino/speeduino/*.dump ./
rm ./utils.*.dump

python cppcheck/addons/misra.py --rule-texts=speeduino/misra/misra_2012_text.txt *.dump 2> results.txt
#python cppcheck/addons/misra.py --rule-texts=speeduino/misra/misra_2012_text.txt board_avr2560.ino.dump 2> results.txt
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
