# Willani

Willani is a compiler for the C programming language inspired by [Compilerbook](https://www.sigbus.info/compilerbook).

This is under developping.

## Demo

Now, willani can compile such as the following code.

```
main () {
  return fibonacci(10);
}

fibonacci(n) {
  if ( n <= 1 ) {
    return n;
  }
  return n + fibonacci(n-1);
}
```

```sh
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ ./run-samplecode
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

