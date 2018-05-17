## init

+ call data: `[uint64 supply (8 bytes)]`

```
./main init context-init.json
contract: aa00000000000000000000000000000000112233
sender: bb00000000000000000000000000000000223344
call data: 00000000aabbccdd
owner: bb00000000000000000000000000000000223344
initial supply: 00000000aabbccdd
```

## totalSupply

+ call data: `[00]`

```
./main call context-totalSupply.json

contract: aa00000000000000000000000000000000112233
sender: bb00000000000000000000000000000000223344
call data: 00
exit with 8 bytes of data:
00000000aabbccdd
```

## balanceOf

+ call data: `[01][address (20 bytes)]`

```
./main call context-balanceOf.json

contract: aa00000000000000000000000000000000112233
sender: bb00000000000000000000000000000000223344
call data: 01bb00000000000000000000000000000000223344
exit with 8 bytes of data:
00000000aabbccdd
```

