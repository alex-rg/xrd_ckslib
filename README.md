# xrd_ckslib
## General
This is an xrootd checksum plugin that provides Get and Calc methods for checksum retrieval/calculation.

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
ofs.ckslib * <path to MyXrdCksManager.so> <buffer_size> <xrd_ceph_params>
```
Example:
```
ofs.ckslib * /etc/xrootd/MyXrdCksManager.so 4194304 /opt/xrootd/lib64/libXrdCeph.so xrootd@,1,8388608,67108864
```
