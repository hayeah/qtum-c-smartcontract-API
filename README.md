# QTUM X86 Smart Contract API Proposal

We'd like to make the smart contract API to resemble a standard program with a `main` function.

If a smart contract is just a "main" program, we could write an emulation library for the smart contract so developers can debug the program offchain using standard toolings like gdb and lldb.

## Transaction Context

The special `qtum_context` object encapsulates the execution context of a transaction:

+ whether the tx is "sendtocontract" or "createcontract"
+ transaction calldata
+ transaction value
+ sender address
+ contract address
+ block time, block number, etc.

The `qtum_context_open` function initializes the context object:

```c
#include "qtum.h"

int main(int argc, char** argv) {
  char* err = NULL;
  qtum_context* ctx = qtum_context_open(argc, argv, &err);

  // ...

  qtum_context_close(ctx);
}
```

The CLI arguments are used to initialize the context, but we don't specify what exactly the arguments are. Smart contract programmers should pass `argv` to `qtum_context_open` as though it is an opaque pointers.

The context struct:

```c
typedef struct qtum_context {
  qtum_action action;
  char address[20];
  char sender[20];
  uint64_t value;

  size_t datasize;
  uint8_t* data;

  // ...other internal fields
} qtum_context;
```

In the on-chain execution environment, the OS could put this in a specific memory region, and the C program could map the struct directly from memory without parsing.

In an emulated off-chain execution environment, we could load the context from a JSON file for debugging purposes.

## Debugging Context

The debugging context can be specified as a JSON file:

```
[
  "aa00000000000000000000000000000000112233",
  "bb00000000000000000000000000000000223344",
  "000000000000000011"
]
```

+ contract address
+ sender address
+ transaction data

# Smart Contract API

What follows is the C equivalent of this Solidity smart contract:

```
contract CheckOwner {
  address owner;

  function constructor() {
    owner = msg.sender;
  }

  function handle() {
    require(owner == msg.sender);

    // ...
  }
}
```

Problems that the C API needs to solve:

+ How could the C program tell between constructor and normal method call?
+ How to store persistent data in storage?
+ How to represent global variables like `msg.sender`?

Ideally the API should look like a normal C program that we can run locally.

## Contract Life Cycle

Use the `ctx->action` field to determine whether the current invokation is a "sendtocontract" or "createcontract" transaction:

```c
switch (ctx->action) {
  case QTUM_ACTION_INIT:
    init(ctx, &err);
    break;
  case QTUM_ACTION_CALL:
    handle(ctx, &err);
    break;
  default:
    err = "Invalid action";
    goto exit_err;
}
```

## Persistent Store

The storage API is similar to leveldb's `put` and `get` API.

```c
void qtum_put(qtum_context* ctx, const char* key, size_t keylen,
              const char* data, size_t datalen, char** err);

char* qtum_get(qtum_context* ctx, const char* key, size_t keylen,
               size_t* retlen, char** err);
```

Here's an example for setting the contract owner in `init`:

```c
int init(qtum_context* ctx, char** err) {
  qtum_put(ctx, keyOwner, sizeof(keyOwner), ctx->sender, 20, err);

  if (*err != NULL) {
    return ERR_ABNORMAL;
  }

  return 0;
}
```

And when handling a "sendtocontract" transaction, we could check the contract ownership:

```c
int isOwner(qtum_context* ctx, char** err) {
  size_t datalen;
  char* owner = qtum_get(ctx, keyOwner, sizeof(keyOwner), &datalen, err);
  int result = memcmp(owner, &ctx->sender, 20) == 0;
  free(owner);
  return result;
}

int handle(qtum_context* ctx, char** err) {
  int authok = isOwner(ctx, err);
  if (*err != NULL) {
    return ERR_ABNORMAL;
  }

  if (!authok) {
    *err = "Not contract owner";
    return ERR_FORBIDDEN;
  }

  ...
}
```

# Debug API

We'd like to run the smart contract like a normal C program, so we can use standard tools like debugger and memory analyzer like valgrind.

The debug API emulates the smart contract locally, and store persistent data in leveldb.

The transaction context may be passed into the contract as a JSON file.

The second argument could be `init` or `call` to distinguish between the life cycle stages.

To build the sample smart contract:

```
make
```

To "init" the smart contract:

```
./main init context-init.json
```

To "call" the smart contract:

```
./main call context-call.json
```
