
Short intstructions:

```

1. git clone https://github.com/vadika/qemu-bpmp/
2. git checkout -b v7.2.0-bpmp
3. $ cd qemu
   $ ./configure --target-list=aarm64-softmmu
   $ make -j12



Device memory map: 
 
0x090c0000 +  /* Base address, size 0x01000 */

     0x0000 \ Tx buffer
     0x01FF /
     0x0200 \ Rx buffer
     0x03FF /
     0x400  -- Tx size
     0x401  -- Rx size
     0x402  -- Ret
     0x500  -- mrq



 Data should be aligned to 64bit paragraph. 
 Protocol is:
 1) write data buffers to 0x0000-0x01FF and 0x0200-0x03FF
 2) write buffer sizes to 0x400 (Tx) and 0x 401 (Rx)
 2) start operation by writing mrq opcode to address 0x0500
 3) read ret code from 0x402 and response data from the buffers 


For reading and writing busybox may be used as:

busybox devmem 0x090c000 
and so on

```
