# Installing gwion

## Get the sources

The source is accessible on [github](https://github.com).

Provided you have git installed, you can get it with:

``` sh
git clone https://github.com/fennecdjay/gwion
```

then change to the source directory
``` sh
cd gwion
```

### Don't forget submodules

You'll need the sources for all base module
``` sh
git submodule update --init util ast
```

> At this point, you might want to configure the build.
  In this case, have a look at the [configuration page](Configure.md)


## Build the libraries and program
``` sh
make
```

## Install the package

``` sh
make install
```
> You may need root privilege to do this.