+ calldata be raw bytes
  + json, msgpack, protocol buffer

+ context object

+ i think something like amazon lambda interface? main is just a shim

+ what should be the arguments
  +

"init"
"send"
"call"

./bin
./bin send
./bin call

./bin init context.json



```

struct qtm_context {
  address bytes32
  sender  bytes32
  value   uint64
  data    bytes
  dataSize size_t
}


qtm_log(data, size)
qtm_setReturn(data, size)
qtm_setReturnError(errCode, data, size)
```

```

int init(qtum_context *ctx) {
  //
}

int set(int i) {
  qtum_state_set("counter", sizeof("counter") , &i, 1);
}

int get() {
  qtum_state_get("counter", sizeof("counter"), &i, 1);
}

enum method{SetCounter, GetCounter};

int handle(qtum_context *ctx) {
  if (ctx.action == QTUM_ACTION_INIT) {
    if (!init(ctx)) {
      return qtm_setReturnError(errCode, data, size);
    }

    return 0;
  }
}

int main(int argc, char** argv) {
  qtum_context* ctx = qtum_context_open(argc, argv);
  int result = handle();
  qtum_context_close(ctx);

  return result;
}
```
