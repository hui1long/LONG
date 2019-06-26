1. Распаковываем в папку bitcoin

2. Установка зависимостией для винды в частности 64 Бит
https://github.com/bitcoin/bitcoin/blob/master/doc/build-windows.md

# Права на папку
chmod 777 -R ~/bitcoin
chown ubuntu:ubuntu -R ~/bitcoin

PATH=$(echo "$PATH" | sed -e 's/:\/mnt.*//g')
cd depends
make HOST=x86_64-w64-mingw32
cd ..
./autogen.sh
 
CONFIG_SITE=$PWD/depends/x86_64-w64-mingw32/share/config.site ./configure --prefix=`pwd`/depends/x86_64-w64-mingw32 --with-gui=qt5 --enable-tests=no --disable-shared LDFLAGS="-L/home/ubuntu/bitcoin/db4/lib/" CPPFLAGS="-I/home/ubuntu/bitcoin/db4/include/" CXXFLAGS="--param ggc-min-expand=1 --param ggc-min-heapsize=32768" --enable-module-ecdh

make


3. настройка под линукс


cd ~
mkdir bitcoin/db4/
wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
tar -xzvf db-4.8.30.NC.tar.gz
cd db-4.8.30.NC/build_unix/
../dist/configure --enable-cxx --disable-shared --with-pic --prefix=/home/ubuntu/bitcoin/db4/
make install

cd ~/bitcoin/
./autogen.sh

./configure LDFLAGS="-L/home/ubuntu/bitcoin/db4/lib/" CPPFLAGS="-I/home/ubuntu/bitcoin/db4/include/" CXXFLAGS="--param ggc-min-expand=1 --param ggc-min-heapsize=32768" --with-gui=qt5 --enable-tests=no --disable-shared --enable-module-ecdh

make




Если  нужно заменить данные для генезис блока и публичного ключа
сгенерировать генезис блок
genesisgen/genesisgen7

для генезис блока в файле 
bitcoin/src/chainparams.cpp:53-...
для публичного ключа в файлах
bitcoin/src/init.cpp:1532-1533
bitcoin/src/qt/addresstablemodel.cpp:245


