<h1 align="center">CLiPExploder</h1>
<p align="center">A fancy C program for decrypting the ClipSp.sys driver in Windows.</p>

<hr>

## Usage

There's a configuration file (offsets.txt) that you need to modify according to the ClipSp version you're trying to decrypt.
The format is a comma-separated list of 3 offsets. The last two offsets should point to the data sections (DataConst & DataRW), and the first offset should correspond to the function responsible for decrypting the data sections.
An example configuration file for ClipSp.sys version 19041.1741 is included in the repository.

Check [GUIDE.md](GUIDE.md) for instructions on finding these offsets for your version of ClipSp.

## Building

Run one of these in an MSYS2 CLANG64 environment:
```
make release
make debug
make relsym
```
``make clean`` will clean up the artifacts.