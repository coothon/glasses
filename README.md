# Glasses

Just an OpenGL image viewer because I got mad that [feh](https://github.com/derf/feh) doesn't have interactive zooming (with the scroll wheel).

## Usage

### Building

```sh
$ make
$ cd ./build
$ ./glasses
```

To clean:
```sh
$ make clean # in project root
```

### Keybinds

`ESC` to quit.  
`h` to toggle high quality resampling (lanczos).  
`l`/`n` to set OpenGL texture resampling mode (linear/nearest). Has no effect with lanczos on.  
`[` to decrease lanczos anti-ringing (not sure if this even works).  
`]` to increase.

Left click and drag to move image; scroll wheel to zoom at cursor position.

You'll have to change the `renderer_ready` call for the image renderer to change the image, for now.
