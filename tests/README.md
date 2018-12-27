To run test cases in this directory, you need a small utility program called [shelltest](https://github.com/simonmichael/shelltestrunner),
which can be installed
by running `apt-get install shelltestrunner`.

Once installed, you can run test cases defined in each of \*.conf files
in the [examples directory](../examples) by applying the conf file to `shelltest`,
or run all of them by:

```
$ make test
```

