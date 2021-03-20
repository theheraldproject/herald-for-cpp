#!/bin/sh

ORIG=$PWD

source ~/ncs/zephyr/zephyr-env.sh
export ARCH=arm

cd herald-wearable

mkdir build-5340
mkdir build-52832
mkdir build-52840
mkdir build-52833

cd build-5340
export BOARD=nrf5340dk_nrf5340_cpuapp
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 32
cd ..

cd build-52840
export BOARD=nrf52840dongle_nrf52840
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 32
cd ..

cd build-52832
export BOARD=nrf52dk_nrf52832
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 32
cd ..

cd build-52833
export BOARD=nrf52833dk_nrf52833
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 32
cd ..

cd ..
cd herald-venue-beacon

mkdir build-5340
mkdir build-52832
mkdir build-52840
mkdir build-52833

cd build-5340
export BOARD=nrf5340dk_nrf5340_cpuapp
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 32
cd ..

cd build-52840
export BOARD=nrf52840dongle_nrf52840
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 32
cd ..

cd build-52832
export BOARD=nrf52dk_nrf52832
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 32
cd ..

cd build-52833
export BOARD=nrf52833dk_nrf52833
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 32
cd ..

cd ..

mkdir build-zephyr
cd build-zephyr
mkdir nrf5340dk
mkdir nrf52832dk
mkdir nrf52833dk
mkdir nrf52840dk
mkdir nrf5340dk/herald-wearable
mkdir nrf52832dk/herald-wearable
mkdir nrf52833dk/herald-wearable
mkdir nrf52840dk/herald-wearable
mkdir nrf5340dk/herald-venue-beacon
mkdir nrf52832dk/herald-venue-beacon
mkdir nrf52833dk/herald-venue-beacon
mkdir nrf52840dk/herald-venue-beacon
cp ../herald-wearable/build-5340/zephyr/merged.hex nrf5340dk/herald-wearable/
cp ../herald-wearable/build-5340/hci_rpmsg/zephyr/zephyr.hex nrf5340dk/herald-wearable/
cp ../herald-wearable/build-52840/zephyr/zephyr.hex nrf52840dk/herald-wearable/
cp ../herald-wearable/build-52832/zephyr/zephyr.hex nrf52832dk/herald-wearable/
cp ../herald-wearable/build-52833/zephyr/zephyr.hex nrf52833dk/herald-wearable/
cp ../herald-venue-beacon/build-5340/zephyr/merged.hex nrf5340dk/herald-venue-beacon/
cp ../herald-venue-beacon/build-5340/hci_rpmsg/zephyr/zephyr.hex nrf5340dk/herald-venue-beacon/
cp ../herald-venue-beacon/build-52840/zephyr/zephyr.hex nrf52840dk/herald-venue-beacon/
cp ../herald-venue-beacon/build-52832/zephyr/zephyr.hex nrf52832dk/herald-venue-beacon/
cp ../herald-venue-beacon/build-52833/zephyr/zephyr.hex nrf52833dk/herald-venue-beacon/
tar cjf zephyr-binaries.tar.gz nrf*
cd ..

echo "Done"
