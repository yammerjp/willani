# Willani

Willani is a compiler for the C programming language inspired by [Compilerbook](https://www.sigbus.info/compilerbook).

This is in under developping.

Now, Willani can compile the following code.

```
foo123=3;
bar=5;
return foo123+bar;
```

## Usage

```sh
# linux
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ make
$ ./willani
# willani read C language code from stdin, and write assembly language code to stdout.

# MacOS
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ ./docker.sh pull
$ ./docker.sh run # compile and run test
```

