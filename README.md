# xrd_ckslib
## General
This is an xrootd checksum plugin that provides *only get* method for checksum retrieval, i.e. checksum retrieval from file attributes.
Can be combined with checksum script to speed up retrieval process.

## Compilation
This library can be compiled as follows:
1. Clone xrootd repo to some local directory <REPODIR>
2. Install `devtoolset` and `xrootd-devel`.
3. Then compilation can be done like this:
```
$ source /opt/rh/devtoolset-7/enable
$ g++ -I /usr/include/xrootd -I <REPODIR>/src -fPIC -g -c MyXrdCksManager.cc
$ g++ -g -shared -o MyXrdCksManager.so MyXrdCksManager.o
```

## Usage
To use the library, add the following to xrootd config:
```
ofs.ckslib * <path to MyXrdCksManager.so>
```
