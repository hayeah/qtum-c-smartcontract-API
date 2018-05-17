# QTUM C++ Smart Contract

Make sure that `qtum.dylib` was built in the root directory.

To build

```
make
```

Set the dynamic library load path to include the root directory:

```
export DYLD_LIBRARY_PATH=../..
```

Run init the set the owner:

```
./main init context.json

contract: aa00000000000000000000000000000000112233
sender: bb00000000000000000000000000000000223344
put owner: bb00000000000000000000000000000000223344
```

Run call to get the owner:

```
./main call context.json

contract: aa00000000000000000000000000000000112233
sender: bb00000000000000000000000000000000223344
get owner: bb00000000000000000000000000000000223344
```
