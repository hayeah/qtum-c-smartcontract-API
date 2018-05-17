# QTUM C Smart Contract

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
```

Run call to get the owner:

```
./main call context.json
```
