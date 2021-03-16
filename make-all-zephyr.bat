
REM WORK IN PROGRESS - DO NOT USE YET - See Ubuntu sh version for inspiration

cd herald-wearable
mkdir build-5340
mkdir build-52832
mkdir build-52840
mkdir build-52833

set ARCH=arm
set CXX=C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2020-q4-major\bin\arm-none-eabi-g++.exe
set CC=C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2020-q4-major\bin\arm-none-eabi-gcc.exe

cd build-5340
set BOARD=nrf5340dk_nrf5340_cpuapp
cmake ..
make
cd ..

cd build-52840
set BOARD=nrf52840dongle_nrf52840
cmake ..
make
cd ..

cd build-52832
set BOARD=nrf52dk_nrf52832
cmake ..
make
cd ..

cd build-52833
set BOARD=nrf52833dk_nrf52833
cmake ..
make
cd ..

echo "Done"
