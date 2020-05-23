# Willani

Willani is a compiler for the C programming language inspired by [Compilerbook](https://www.sigbus.info/compilerbook).

This is under developping.

Now, willani can compile such as the following code.

```
foo123=3;
bar=5;
return foo123+bar;
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

