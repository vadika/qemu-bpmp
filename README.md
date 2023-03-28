
Short intstructions:

```

1. git clone https://github.com/vadika/qemu-bpmp/
2. git checkout -b v7.2.0-bpmp
3. $ cd qemu
   $ ./configure --target-list=aarm64-softmmu
   $ make -j12



Device memory map: 
 
0x090c0000 +  /* Base address */
     0x000 \ Request buffer
     0x07F /

 Data should be aligned to 64bit paragraph. 
 Protocol is:
 1) write data buffer to 0x000-0x00b (48*bytes, 12*64bit words)
 2) start operation by writing anything to address 0x07F
 3) read response from 0x000-0x00b


For reading and writing busybox may be used as:

busybox devmem 0x090c000 
and so on

```
