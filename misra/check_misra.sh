if [ -f ./results.txt ]; then
	rm results.txt
fi

for i in speeduino/speeduino/*.ino; do
  #cppcheck --xml --include=${i%.*}.h --include=speeduino/speeduino/globals.h $i > /dev/null
  #cppcheck --force --dump --suppress=syntaxError:speeduino/speeduino/src/PID_v1/PID_v1.h --include=${i%.*}.h --include=speeduino/speeduino/globals.h -DCORE_AVR=1 -USTM32F4 $i > /dev/null
  cppcheck --dump --suppress=syntaxError:speeduino/speeduino/src/PID_v1/PID_v1.h --include=${i%.*}.h -DCORE_AVR=1 -D__AVR_ATmega2560__ $i > /dev/null
done
mv speeduino/speeduino/*.dump ./
rm ./utils.*.dump

python cppcheck/addons/misra.py --rule-texts=speeduino/misra/misra_2012_text.txt *.dump 2> results.txt
rm *.dump

cat results.txt
# wc -l results.txt

errors=`wc -l < results.txt | tr -d ' '`
echo $errors MISRA violations

if [ $errors -gt 0 ]; then
	exit 1
else
	exit 0
fi
