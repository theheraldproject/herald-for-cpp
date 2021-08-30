#!/bin/bash

# Run this ONCE
# Then run the make-all-zephyr.sh file

ORIG=$PWD

sudo apt update
sudo apt upgrade

# Note: Be sure to set ZEPHYR_BASE before calling this file: E.g. D:\devtools\ncs\v1.5.0\zephyr
sudo apt install --no-install-recommends git cmake ninja-build gperf \
ccache dfu-util device-tree-compiler wget \
python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
make gcc gcc-multilib g++-multilib libsdl2-dev

mkdir ~/gn && cd ~/gn
wget -O gn.zip https://chrome-infra-packages.appspot.com/dl/gn/gn/linux-amd64/+/latest
unzip gn.zip
rm gn.zip
echo 'export PATH=${HOME}/gn:"$PATH"' >> ~/.bashrc
source ~/.bashrc

cd ~/
wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz
mkdir gnuarmemb
bunzip2 gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz 
cd gnuarmemb/
tar xf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar 
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH="~/gnuarmemb/gcc-arm-none-eabi-10-2020-q4-major"


# Now install Nordic connect - https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html
mkdir ~/ncs
cd ~/ncs
west init -m https://github.com/nrfconnect/sdk-nrf --mr v1.6.1
west update
west zephyr-export

source ~/ncs/zephyr/zephyr-env.sh
cd ~/ncs
pip3 install --user -r zephyr/scripts/requirements.txt
pip3 install --user -r nrf/scripts/requirements.txt
pip3 install --user -r bootloader/mcuboot/scripts/requirements.txt

cd $ORIG

echo "Completed Zephyr dependency installation"

exit 0
