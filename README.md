# Willani

Willani is a compiler for the C programming language inspired by [Compilerbook](https://www.sigbus.info/compilerbook).

This is under developping.

## Demo

Now, willani can compile such as the following code.

```sample.c
int fibonacci(int n);

int main () {
  return fibonacci(10);
}

int fibonacci(int n) {
  if ( n <= 1 ) {
    return n;
  }
  return n + fibonacci(n-1);
}
```

```sh
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ make
$ ./willani ' int fibonacci(int n); int main () { return fibonacci(10); } int fibonacci(int n) { if ( n <= 1 ) { return n; } return n + fibonacci(n-1); } ' > tmp.s
$ gcc --static tmp.s -o tmp.out
$ ./tmp.out
$ echo "$?"
```

## Usage

### Linux

```sh
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ make

$ ./willani 'int main(){return 0;}' > asm.s
# Willani read C language code from a commandline argument, and write assembly language code to stdout.

$ make test   # Run test.
```

### macOS
```sh
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ bin/docker.sh pull
$ bin/docker.sh sh
# run interactive shell on a docker container.

$ make

$ ./willani 'int main(){return 0;}' > asm.s
# Willani read C language code from a commandline argument, and write assembly language code to stdout.

$ make test   # Run test.
```

