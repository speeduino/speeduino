if ls *.dump 1> /dev/null 2>&1; then
	rm *.dump
fi
if [ -f ./results.txt ]; then
	rm results.txt
fi

for i in speeduino/speeduino/*.ino; do
  #cppcheck --xml --include=${i%.*}.h --include=speeduino/speeduino/globals.h $i > /dev/null
  cppcheck --quiet --dump --suppress=syntaxError:speeduino/speeduino/src/PID_v1/PID_v1.h --include=${i%.*}.h --include=speeduino/speeduino/globals.h $i > /dev/null
done
mv speeduino/speeduino/*.dump ./

python cppcheck/addons/misra.py *.dump 2> results.txt

# wc -l results.txt

errors=`wc -l < results.txt | tr -d ' '`
echo $errors
exit $errors
