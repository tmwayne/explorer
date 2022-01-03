# Preview

Preview is an no-frills, `ncurses`-based delimited data (pre-)viewer. 
It is similar to `less` but prints the data in tabular form and allows 
multidirectional scrolling.

The program is written in C and loads data using mmap, allowing you to preview 
datasets much larger than RAM, with no load time.

The motivation is for Data Science/Engineering workflows. Sometimes
it's useful to be able to look at the data without loading it into a
scripting language like Python or R. 

## Requirements

- GNU/Linux

At the time of writing, `preview` hasn't been tested on MacOS.

## Installation

```
git clone --depth 1 git@github.com:tmwayne/preview.git && cd preview
sh ./bootstrap
./configure
make
make check
make install
```

## Limitations / Bugs

Some known limitations, which are actively being worked on. Known bugs
will also be recorded here, when I get around to it.

- There's no support for piping, meaning `cat <data> | preview`
doesn't work. Piped streams will require standard file I/O, that is `read` and
`write`, limiting the maximum data sizes.

- The only supported navigation is through vim bindings:
`h` (left), `j` (down), `k` (up), and `l` (right). The input scanner is
a small `flex` program, which as it turns out doesn't interface that nicely
with `ncurses`. This will be written as a custom scanner for use with `bison`.
