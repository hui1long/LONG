# Переходим в папку генератора

```bash
cd ~/genesisgen
```

# Сборка генератора

```bash
sudo apt-get update
sudo apt-get install build-essential libssl-dev
gcc genesisgen7.c -o genesisgen7 -lcrypto
```

# Использование. 
# genesisgen7 <pubkey> "<timestamp>" <nBits>
# для 0000007fffffffffffffffffffffffffffffffffffffffffffffffffffffffff - 0x1d7fffff - nBits - 494927871
# В генераторе nBits зафиксировано

# Запускаем создание генезис блока 
./genesisgen7 04583cb1b994aaaeb0039eff0fc2f9737cc696d8545b03360b7e77ebdca227fa4d2a116c7b99b208862b88f0b55b082dc8fb32fb0c9c7bbaec27f6771014e29381 "LONG network" 494927871

# Получаем ответ 
Coinbase: 04ffff7f1d01040c4c4f4e47206e6574776f726b

PubkeyScript: 4104583cb1b994aaaeb0039eff0fc2f9737cc696d8545b03360b7e77ebdca227fa4d2a116c7b99b208862b88f0b55b082dc8fb32fb0c9c7bbaec27f6771014e29381ac

Merkle Hash: 16d3a330a1fba75b50059d110bb75c821eeb543bc57aee1e3c5107a989a6864e
Byteswapped: 4e86a689a907513c1eee7ac53b54eb1e825cb70b119d05505ba7fba130a3d316
Generating block...
1337857 Hashes/s, Nonce 86622659
Block found!
Hash: 00000031ae9e29c7368fb404de7c936d8b1086c90f5dbedf4ddc6b1907f6bb00
Nonce: 87850842
Unix time: 1548914258



chainparams.cpp.new
# Удаляем файл chainparams.cpp
rm src/chainparams.cpp

# Дублируем шаблон 
cp src/chainparams.cpp.new src/chainparams.cpp

# руками заменяем полученные значения в файле chainparams.cpp

unix time {{nTimeGen}} 
nonce {{NonceGen}}
hashblock {{hashGen}}
Byteswapped {{ByteswappedGen}}


# запускаем сборку 

