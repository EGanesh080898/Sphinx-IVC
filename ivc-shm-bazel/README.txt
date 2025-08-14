## PREREQUISITES ##
# Install the below mentioned packages using apt-get
$ sudo apt-get install libcunit1-dev libjsoncpp-dev
#make sure that timedatectl ntp is true
$sudo timedatectl set-ntp true

## BUILD PROCEDURE ##
# Run the below Command to compile ivc-shm
$ mkdir build
$ cmake -B build -S .
$ make -j$(nproc) -C build

#Output(bin,lib,inc...etc) is generated in build folder
