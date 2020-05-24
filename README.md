# Willani

Willani is a compiler for the C programming language inspired by [Compilerbook](https://www.sigbus.info/compilerbook).

This is under developping.

## Demo

Now, willani can compile such as the following code.

```
foo123=3;
bar=5;
if (foo123 == 0)
  return 0;
else
  return foo123+bar;
```

```sh
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ make
$ cat samplecode | xargs -0 ./willani > tmp.s
$ cc tmp.s
$ ./tmp
$ echo "$?"
```

## Usage

### Linux

```sh
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ make

$ ./willani 'return 0;' > asm.s
# Willani read C language code from a commandline argument, and write assembly language code to stdout.

$ make test   # Run test.
```

### macOS
```
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ ./docker.sh pull
$ ./docker.sh run   # compile and run test
```

