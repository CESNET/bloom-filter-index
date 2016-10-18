Bloom Filter Indexes
====================

This repository contains source files for the bloom filter indexing library.
This library was mainly created for IP address indexing in:
  * IPFIXCol storage plugin LNFStore (https://github.com/CESNET/ipfixcol/tree/master/plugins/storage/lnfstore)
  * FDistDump netflow data quering tool (https://github.com/CESNET/fdistdump/)


Instalation
-----------
Bloom filter indexing library could be installed in a common way:

```
./bootstrap.sh
./configure
make
sudo make install
```

For information about configuration options see:
```
./configure --help
```


Example of use
--------------
For examples of usage see FDistDump or LNFStore code.
