# esp32-poc
ESP32 Proof of concept

In order to setup environment and build the project follow these steps:

1. Clone ESP-IDF framework v3.2<br>
$ git clone -b v3.2 --recursive https://github.com/espressif/esp-idf.git

2. Install pip for python<br>
$ sudo apt install python-pip

3. Setup environment variables<br>
$ export IDF_PATH=$HOME/projects/esp/esp-idf

4. Install python libraries needed for the project<br>
$ python -m pip install --user -r $IDF_PATH/requirements.txt

5. Install necessary packages for the project<br>
$ sudo apt-get install gcc git wget make libncurses-dev flex bison gperf python python-pip python-setuptools python-serial python-cryptography python-future

6. Download and install toolchain<br>
$ wget https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz<br>
$ tar cvzf xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz

7. Setup toolchain environment variables<br>
$ export PATH="$HOME/projects/esp/xtensa-esp32-elf/bin:$PATH"

8. Add user to the dialout group in order to use /dev/ttyUSBx device without sudo (logout is required afterward)<br>
$ sudo usermod -a -G dialout $USER

9. Clone esp32-poc<br>
$ git clone https://github.com/emb-team/esp32-poc.git

10. Build and run esp32-poc<br>
$ make -j3 flash monitor
