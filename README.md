Bloom Filter Indexes
====================

This repository contains source files for the Bloom filter indexing library.
This library was originally created for IP address indexing in:
  * IPFIXCol storage plugin LNFStore (https://github.com/CESNET/ipfixcol/tree/master/plugins/storage/lnfstore)
  * FDistDump net-flow data querying tool (https://github.com/CESNET/fdistdump/)


Table of contents
-----------------
1. Installation
2. API documentation
3. Example


1. Installation
--------------
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

2. API documentation
--------------------
Doxygen documentation in the html format could be find in [doc/](./doc/).

Usage of Bloom filter index simply consists in a few steps:
1) Declare an empty index of type `bfi_index_t`.
2a) Initialize the index with given `estimated item count` and `false positive
 probability` or
2b) load an existing index from a file.
3) Add elements into the index.
4) Store the index into a file.
5) Destroy the index.

It is also possible to clear the index (e.g. for new iteration of indexing)
which provides an empty ready-to-use index (i.e. like index after
initialization) or get count of stored elements in the index (e.g. for dynamic
re-calculation of the Bloom filter parameters).


3. Example
----------
For examples of usage see FDistDump or LNFStore code.
