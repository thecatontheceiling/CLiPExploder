# Guide for Finding Offsets

**Note:** I am using IDA Pro for this guide, but Ghidra should work fine too.

## Collecting information

Open ClipSp.sys in your favorite decompiler and go to the exported `ClipSpUninitialize` function. You should see something like this:

<img width="1538" height="690" alt="image" src="https://github.com/user-attachments/assets/4d1f1e3f-3c55-4bbb-8a11-35a65e82b062" />

In this example, `sub_1800023DC` and `sub_180001740` are decryption functions; one is called twice on different data. They take pointers to a pair of `DataConst` and `DataRW` data sections as arguments. You will need to specify these in the configuration file.

For example, I copy the offsets of the parameters used here (first pair: `0xD6560`, `0xDA988`; second: `0xD6CD0`, `0xDAB50`; third: `0xD6610`, `0xDA9B0`).

These calls also lead to the function performing the actual decryption. You will need to specify this function's offset in the config as well.

Going into `sub_1800023DC`, for example:

<img width="1536" height="534" alt="image" src="https://github.com/user-attachments/assets/234126ed-aec4-4b23-9d69-f50221637778" />

Here, we can see the actual decryption function being called. You need to specify the offset of this function in the configuration file, along with the associated data offsets. Do the same for `sub_180001740`.

Specify this information in the configuration file like so:
```
0x2660,0xD6560,0xDA988
0x2660,0xD6CD0,0xDAB50
0x1868,0xD6610,0xDA9B0
```

You can find another pair in `ClipSpInitialize`:

<img width="1537" height="451" alt="image" src="https://github.com/user-attachments/assets/cc9c1c63-5f41-4380-a226-2083c4640d4d" />

Updating our config:
```
0x2660,0xD6560,0xDA988
0x2660,0xD6CD0,0xDAB50
0x1868,0xD6610,0xDA9B0
0x1868,0xD5B00,0xDA700
```

All `ClipSp` binaries I've seen contain 6 pairs of these data sections. This means there are two more pairs you need to locate. Admittedly, they are a bit harder to find.

The 5th pair is referenced by a few unexported/unnamed functions. You can find them in the xrefs of your equivalent `sub_1800023DC` and `sub_180001740` functions, but the list will be polluted by functions that decrypt the segments you have already found offsets for.

I've found that installing the **hrtng** IDA plugin and pressing `Shift` + `X` shows the pseudocode of the xref rather than just disassembly. This makes finding what you need much easier, as you can simply skip over xrefs called with offsets you already know.

The 6th pair is referenced by functions that are encrypted. This means you will need to run `CLiPExploder` with your unfinished configuration once to decrypt the functions referencing the last pair. After this, the same process applies here as with the 5th pair.