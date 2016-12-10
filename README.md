# Realtime Pong Xenomai


## Requirements 

### Xenomai 2.6.4 + kernel linux 3.14.17 (or equivalent) 

#### Installation


* Install Mint 17 in VMWARE or VIRTUALBOX

* Linux Mint 17 "Qiana" - KDE (32-bit) Version 32bits https://www.linuxmint.com/edition.php?id=167
* Requirements : QT

```
wget http://download.qt.io/official_releases/qt/5.7/5.7.0/qt-opensource-linux-x64-5.7.0.run
chmod +x qt-opensource-linux-x64-5.7.0.run
./qt-opensource-linux-x64-5.7.0.run
```

* Open a terminal

```
sudo su
cd /usr/src
wget https://www.kernel.org/pub/linux/kernel/v3.x/linux-3.14.17.tar.gz
wget https://xenomai.org/downloads/xenomai/stable/xenomai-2.6.4.tar.bz2 
ln -s linux-3.14.17 linux
ln -s xenomai-2.6.14 xenomai
./xenomai/scripts/prepare-kernel.sh --arch=x86_64 --linux=/usr/src/linux --adeos=/usr/src/xenomai/ksrc/arch/x86/patches/ipipe-core-3.14.17-x86-4.patch # ipipe-core may have another version
```

```
cd linux

cp /boot/config-$(uname -r) .config
make oldconfig # Accept all (press enter always)
make xconfig # ctrl +f and search for CPU_FREQ, CPU_IDLE, CC_STACKPROTECTOR, KGDB, APM, ACPI_PROCESSOR, INTEL_IDLE, INPUT_PCSPKR, PCI_MSI,

```







### SDL interface 1.2
#### Installation (sample)

sudo apt-get install libsdl1.2-dev

## Execution

make
./pong [name_user1] [name_user2 OR 'computer'] 




resource interface SDL: https://github.com/flightcrank/pong


gcc -o pong pong.c `sdl-config --cflags --libs`

install SDL sudo apt-get install libsdl1.2-dev


tarefa cinemática

ficheiro de caracteristicas/parametros

bola -> posicao x ; posicao y ; velocidade X max; velocidade Y max ; velocidade X min; velocidade Y min ;
barra -> posicao y
campo -> x, y


tarefa input barra


exemplos xenomai: http://www.cs.ru.nl/lab/xenomai/exercises
https://github.com/yunusdawji/xenomai-examples

