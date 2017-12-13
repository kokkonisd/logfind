# logfind

A grep-like log-searching tool, written in C.
Inspired by Zed A. Shaw's ["Learn C The Hard Way"](https://learncodethehardway.org/c/).


## how to install logfind

To install logfind, simply get the [latest version](https://github.com/kokkonisd/logfind/releases/tag/v1.1), decompress it, `cd` into it, then run:

```bash
$ make install
```

## how to uninstall logfind

To uninstall logfind, `cd` into its previously downloaded package, then run:

```bash
$ make uninstall
```

## how to use logfind

In order for logfind to work, you have to input a list of log files you want to search through. To do that, simply write the file names of the log files you want to search through in `~/.logfind`. The files should be separated by newlines.

### logical _and_

By default, logfind searches for the specified terms by putting a logical _and_ between them.
For example,
```bash
$ logfind wifi system
```
will output a list of log files containing both "wifi" _and_ "system".

### logical _or_

To change the default logical _and_ behaviour, simply use the `-o` option.
For example,
For example,
```bash
$ logfind -o wifi system
```
will output a list of log files containing either "wifi" _or_ "system".

### version

To get logfind's current version, simply run

```bash
$ logfind -v
```

or

```bash
$ logfind --version
```

### help

To get a list of all the available commands, simply run

```bash
$ logfind --help
```
