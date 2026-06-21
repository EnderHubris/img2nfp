# img2nfp
Simple unix program for converting images into nfp files to be used in [tweaked: cc](https://tweaked.cc/).
This program compiles on linux systems using `g++`.

This program depends on:
- nothings stb_image.h - https://github.com/nothings/stb/
- CLI11 - https://github.com/CLIUtils/CLI11/

These files are downloaded through the `Makefile`

[How to build](#build)
```
img2nfp (Image Conversion Tool)
Usage: ./img2nfp [OPTIONS] input

Positionals:
  input Image File REQUIRED   Path to image file [png,jpg,jpeg]

Options:
  -h,--help                   Print this help message and exit
  -o,--output TEXT            Name of ntp output file
  -p,--preview                Preview Output in terminal
```

### Keep in mind!
Tweaked: CC cannot display large images on their max monitor set up of 8x6 blocks.
You are responsible for scaling your image to meet the specs displayed by your
target monitor.
```lua
local mon = peripheral.find("monitor")

-- this will change the size outputted, 0.5 recommended for more detail
mon.setTextScale(0.5)

local w, h = mon.getSize()
print(w, h)
```

You can draw to the monitor via:
```lua
local mon = peripheral.find("monitor")

mon.setTextScale(0.5) -- optional
mon.setBackgroundColor(colors.black)
mon.clear()

-- recommended to use absolute path for the lua program to find it
local image = paintutils.loadImage("/pict.nfp")

term.redirect(mon)
paintutils.drawImage(image, 1, 1)
term.redirect(term.native())
```

## Build
```console
https://github.com/EnderHubris/img2nfp.git
cd img2nfp
make
```