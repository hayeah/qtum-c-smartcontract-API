# QTUM X86 Smart Contract API Proposal

We'd like to make the smart contract API to resemble a standard program with a `main` function.

If a smart contract is just a "main" program, we could write an emulation library for the smart contract so developers can debug the program offchain using standard toolings like gdb and lldb.

The example contracts are:

+ [c](contracts/c)
+ [c++](contracts/cpp)
+ [rust](contracts/rust)

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
  qtum_err* err = NULL;
  qtum_context* ctx = qtum_context_open(argc, argv, &err);

  // ...
}
```

The CLI arguments are used to initialize the context, but we don't specify what exactly the arguments are. Smart contract programmers should pass `argv` to `qtum_context_open` as though it is an opaque pointers.

The context struct:

```c
typedef struct qtum_context {
  qtum_action action;
  uint8_t address[ADDRESS_SIZE];
  uint8_t sender[ADDRESS_SIZE];
  uint64_t value;
  uint8_t* data;
  size_t datasize;

  // ...other internal fields
} qtum_context;
```

In the on-chain execution environment, the OS could put this in a specific memory region, and the C program could map the struct directly from memory without parsing.

In an emulated off-chain execution environment, we could load the context from a JSON file for debugging purposes.

## Debugging Context

The debugging context can be specified as a JSON file:

```
{
  // contract address
  "address": "aa00000000000000000000000000000000112233",
  // sender address
  "sender": "bb00000000000000000000000000000000223344",
  // call data
  "data": "aabb"
}
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
    log(owner);
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
extern void qtum_put(qtum_context* ctx, const uint8_t* key, size_t keylen,
                     const uint8_t* data, size_t datalen, qtum_err** err);

extern uint8_t* qtum_get(qtum_context* ctx, const uint8_t* key, size_t keylen,
                         size_t* retlen, qtum_err** err);
```

Here's an example for setting the contract owner in `init`:

```c
void init(qtum_context* ctx) {
  qtum_err* err = NULL;
  qtum_put(ctx, keyOwner, sizeof(keyOwner), ctx->sender, ADDRESS_SIZE, &err);
  if (err) qtum_exit_error(err);

  printf("put owner: %s\n", bytesToHexString(ctx->sender, ADDRESS_SIZE));
}
```

And when handling a send/call invokation, we could check the contract ownership by getting the stored owner:

```c
void handle(qtum_context* ctx) {
  qtum_err* err = NULL;

  size_t datalen;
  uint8_t* owner = qtum_get(ctx, keyOwner, sizeof(keyOwner), &datalen, &err);
  if (err) qtum_exit_error(err);

  printf("get owner: %s\n", bytesToHexString(owner, ADDRESS_SIZE));
}
```

## C++ API

The C++ smart contract API uses "exception free" style. The API methods return results along with possible err values.

```cpp
void init(qtum::Context& ctx) {
  auto err = ctx.put(kOwner, ctx.sender);
  if (err) qtum::exit_error(*err);

  std::cout << "put owner: " << ctx.sender.hexstr() << std::endl;
}

void handle(qtum::Context& ctx) {
  auto err = std::unique_ptr<qtum::Error>{};
  auto owner = std::unique_ptr<qtum::Address>{};

  std::tie(owner, err) = ctx.getAddress(kOwner);
  if (err) qtum::exit_error(*err);

  std::cout << "get owner: " << owner->hexstr() << std::endl;
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
