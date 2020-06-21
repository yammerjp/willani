# Willani

Willani is a compiler for the C programming language inspired by [Compilerbook](https://www.sigbus.info/compilerbook).
It makes AT&T syntax assembly language code for the machine of Linux on x86_64.

It is under development, and now, it can compile a subset of the C programming language.

## Setup

### Linux

```sh
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ make test   # Run test.
```

### macOS
```sh
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ ./docker.sh pull
$ ./docker.sh sh
# run interactive shell on a docker container.
$ make test   # Run test.
```

## Demo

Now, willani can compile such as the following code.

```fib.c
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

You can run the code by executing the following commands on a Linux machine.

```sh
$ git clone https://github.com/basd4g/willani.git
$ cd willani
$ make
$ echo 'int fibonacci(int n); int main () { return fibonacci(10); } int fibonacci(int n) { if ( n <= 1 ) { return n; } return n + fibonacci(n-1); }' > fib.c
$ ./willani fib.c > fib.s
$ gcc --static fib.s -o fib.out
$ ./fib.out
$ echo "$?"
```
