echo "START COMPILATION..."

pio_output="../speeduino/speeduino.ino.cpp"

#start platformio compilation
cd ../ && platformio run -e megaatmega2560 >/dev/null &

#wait until the cpp file is created by platformio
while [[ ! -f $pio_output ]] ; do
  sleep 0.5;
done;

echo "CPP FILE FIRST APPARITION"

#try to get the latest file, so it is fully post-processed
while [[ -f $pio_output ]] ; do
  cp $pio_output ./ >/dev/null 2>&1;
  sleep 0.1;
done;

echo "GOT CPP FILE LAST ITERATION"

#copy speeduino/speeduino.cpp
sed -e 's/# [0-9]*/\/\/\0/' speeduino.ino.cpp > speeduino.cpp
rm speeduino.ino.cpp

#copy speeduino/globals.h
cat include/my_globals.h > include/mock_globals.h
echo "" >> include/mock_globals.h
sed -e 's/^int\|^uint\|^bool\|^volatile\|^unsigned\|^byte/extern \0/' -e '/{/!s/^struct*/extern \0/' \
    -e '/static_assert/d' -e '/const/!s/=.*;/;/g' \
    ../speeduino/globals.h >> include/mock_globals.h

echo "WAITING FOR PLATFORMIO TO FINISH"

wait

echo "SUCCESS!"

