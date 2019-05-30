echo "START COMPILATION..."

cd ../
~/.platformio/penv/bin/pio run -e megaatmega2560 >/dev/null &

cp speeduino/speeduino.ino.cpp tests/ >/dev/null 2>&1
while [[ $? -ne 0 ]] ; do
  sleep 0.1;
  cp speeduino/speeduino.ino.cpp tests/ >/dev/null 2>&1
done;

cd tests

#copy speeduino/speeduino.cpp
sed -e 's/# [0-9]*/\/\/\0/' speeduino.ino.cpp > speeduino.cpp
rm speeduino.ino.cpp

#copy speeduino/globals
cat include/my_globals.h > include/mock_globals.h
echo "" >> include/mock_globals.h
sed -e 's/^int\|^uint\|^bool\|^volatile\|^unsigned\|^byte/extern \0/' -e '/{/!s/^struct*/extern \0/' \
    -e '/static_assert/d' -e '/const/!s/=.*;/;/g' \
    ../speeduino/globals.h >> include/mock_globals.h

echo "WAITING..."

wait

echo "SUCCESS!"

