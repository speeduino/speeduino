if [ -f ./results.txt ]; then
	rm results.txt
fi

for i in speeduino/speeduino/*.ino; do
  #cppcheck --xml --include=${i%.*}.h --include=speeduino/speeduino/globals.h $i > /dev/null
  cppcheck --force --dump --suppress=syntaxError:speeduino/speeduino/src/PID_v1/PID_v1.h --include=${i%.*}.h --include=speeduino/speeduino/globals.h $i > /dev/null
done
mv speeduino/speeduino/*.dump ./
rm ./utils.*.dump

python cppcheck/addons/misra.py *.dump 2> results.txt
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
