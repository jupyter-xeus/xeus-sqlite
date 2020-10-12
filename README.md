# xeus-sqlite

[![Travis](https://travis-ci.com/jupyter-xeus/xeus-sqlite.svg?branch=master)](https://travis-ci.org/jupyter-xeus/xeus-sqlite)
[![Documentation Status](https://readthedocs.org/projects/xeus-sqlite/badge/?version=latest)](https://xeus-sqlite.readthedocs.io/en/latest/?badge=latest)
[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/jupyter-xeus/xeus-sqlite/stable?filepath=examples/Simple%20operations.ipynb)

`xeus-sqlite` is a Jupyter kernel for SQL based on the native implementation of the Jupyter protocol [xeus](https://github.com/jupyter-xeus/xeus).

## Usage

Launch the Jupyter notebook with `jupyter notebook` or Jupyter lab with `jupyter lab` and launch a new SQL notebook by selecting the **xsqlite** kernel.

## Installation

xeus-sqlite has been packaged for the conda package manager.

To ensure that the installation works, it is preferable to install xeus-sqlite in a fresh conda environment.

To ensure that the installation works, it is preferable to install `xeus` in a fresh conda environment. It is also needed to use
a [miniconda](https://conda.io/miniconda.html) installation because with the full [anaconda](https://www.anaconda.com/)
you may have a conflict.

The safest usage is to create an environment named xeus-sqlite with your miniconda installation

```
conda create -n xeus-sqlite
conda activate xeus-sqlite
```

### Installing from conda

To install xeus-sqlite with the conda package manager

```
conda install xeus-sqlite jupyterlab -c conda-forge
```

### Installing from source

To install the xeus-sqlite dependencies

```bash
conda install cmake nlohmann_json xtl cppzmq xeus sqlite sqlitecpp cpp-tabulate=1.3 xvega xproperty jupyterlab -c conda-forge
```

Then you can compile the sources

```bash
mkdir build
cd build
cmake -D CMAKE_INSTALL_PREFIX=$CONDA_PREFIX ..
make
make install
```

## Documentation

https://xeus-sqlite.readthedocs.io/en/latest/

## Dependencies

``xeus-sqlite`` depends on

- [xeus](https://github.com/jupyter-xeus/xeus)
- [SQLite](https://github.com/sqlite/sqlite)
- [SQLiteCPP](https://github.com/SRombauts/SQLiteCpp)
- [Tabulate](https://github.com/p-ranav/tabulate)
- [XVega](https://github.com/Quantstack/xvega)

| `xeus-sqlite`|      `xeus`     |     `SQLite`    |   `SQLiteCPP`   |   `tabulate`    | `nlohmann_json` | `xvega` |
|--------------|-----------------|-----------------|-----------------|-----------------|-----------------|---------|
|    master    | >=0.24.0, <0.25 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<2.0    | >=3.0.0         | >= 0.0.4|
|    0.1.1     | >=0.24.0, <0.25 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<2.0    | >=3.0.0         | >= 0.0.4|
|    0.1.0     | >=0.23.9, <0.24 | >=3.30.1, <4    | >=3.0.0, <4     | =1.2.0          | >=3.0.0         | >= 0.0.4|

## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md) to know how to contribute and set up a development environment.

## License

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the [LICENSE](LICENSE) file for details.
