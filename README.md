# Winter and Summer - unChallenge tool

The Games: Winter Challenge, and Summer Challenge - tool to extract
game assets.

## What is this?

A tool to extract data assets (images, scenery, music, fonts and
3D mesh data) from an Accolade video game series:
* Winter Challange ('91)
* The Games: Summer Challange ('92)

## How to build

```
$ make all
```

## How to use it

```
$ mkdir out
$ ./extract WINTER.EXE
```

## DOS version

Compile & link using Open Watcom.
Precompiled binary targeting DOS 32bit protected mode is placed
in `dosbin` directory.

### history

The tool was made by me around 2001, recently I was asked to publish
the source.
