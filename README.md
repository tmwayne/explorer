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

## Notes

As `preview` is free software, I make no guarantee to its performance.
Currently, there's no support for `stdin`, meaning `cat <data> | preview`
doesn't work. This is being added now.
