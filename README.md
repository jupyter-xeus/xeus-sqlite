# xeus-sqlite
[![Azure Pipelines](https://dev.azure.com/jupyter-xeus/jupyter-xeus/_apis/build/status/jupyter-xeus.xeus-sqlite?branchName=master)](https://dev.azure.com/jupyter-xeus/jupyter-xeus/_build/latest?definitionId=5&branchName=master)
[![Documentation Status](https://readthedocs.org/projects/xeus-sqlite/badge/?version=latest)](https://xeus-sqlite.readthedocs.io/en/latest/?badge=latest)
[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/jupyter-xeus/xeus-sqlite/stable?filepath=examples/Simple%20operations.ipynb)

`xeus-sqlite` is a Jupyter kernel for SQL based on the native implementation of the Jupyter protocol [xeus](https://github.com/jupyter-xeus/xeus).

`xeus-sqlite` now supports [vega](https://vega.github.io/) visualizations thanks to [xvega](https://github.com/Quantstack/xvega).

## Usage

Launch the Jupyter notebook with `jupyter notebook` or Jupyter lab with `jupyter lab` and launch a new SQL notebook by selecting the **xsqlite** kernel.

## Installation

xeus-sqlite has been packaged for the mamba (or conda) package manager on the **Linux** and **OS X** platforms. At the moment, we are not providing packages for the **Windows** platform.

To ensure that the installation works, it is preferable to install xeus-sqlite in a fresh environment.

To ensure that the installation works, it is preferable to install `xeus` in a fresh environment. It is also needed to use
a [miniforge](https://github.com/conda-forge/miniforge#mambaforge) or [miniconda](https://conda.io/miniconda.html) installation because with the full [anaconda](https://www.anaconda.com/)
you may have a conflict.

The safest usage is to create an environment named xeus-sqlite

```
mamba create -n xeus-sqlite
source activate xeus-sqlite
```

### Installing from conda-forge 

To install xeus-sqlite with the mamba (or conda) package manager

```
mamba install xeus-sqlite jupyterlab -c conda-forge
```

### Installing from source

To install the xeus-sqlite dependencies

```bash
mamba install cmake nlohmann_json xtl cppzmq xeus sqlite sqlitecpp cpp-tabulate=1.3 xvega xproperty xtl cppzmq xproperty jupyterlab -c conda-forge
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

### Build docs

```
mamba install sphinx breathe doxygen sphinx_rtd_theme
```

## Dependencies

``xeus-sqlite`` depends on

- [xeus-zmq](https://github.com/jupyter-xeus/xeus-zmq)
- [SQLite](https://github.com/sqlite/sqlite)
- [SQLiteCPP](https://github.com/SRombauts/SQLiteCpp)
- [Tabulate](https://github.com/p-ranav/tabulate)
- [XVega](https://github.com/Quantstack/xvega)

| `xeus-sqlite`|    `xeus-zmq`   |     `SQLite`    |   `SQLiteCPP`   |   `tabulate`    | `nlohmann_json` | `xvega` |`xvega-bindings`|
|--------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------|----------------|
|    master    | >=1.0.2, <2.0.0 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<1.5    | >=3.0.0         | >= 0.0.10 | >= 0.0.3       |

Prior to version 0.6, `xeus-sqlite`  was depending on `xeus` instead of `xeus-zmq`:

| `xeus-sqlite`|      `xeus`     |     `SQLite`    |   `SQLiteCPP`   |   `tabulate`    | `nlohmann_json` | `xvega` |`xvega-bindings`|
|--------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------|----------------|
|    0.5.2     | >=2.4.0, <3.0.0 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<1.5    | >=3.0.0         | >= 0.0.10 | >= 0.0.3       |
|    0.5.1     | >=2.0.0, <3.0.0 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<1.5    | >=3.0.0         | >= 0.0.10 | >= 0.0.3       |
|    0.3.0     | >=1.0.0, <2.0.0 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<1.5    | >=3.0.0         | >= 0.0.10 | >= 0.0.3       |
|    0.2.0     | >=0.23.9, <0.24 | >=3.30.1, <4    | >=3.0.0, <4     | =1.2.0          | >=3.0.0         | >= 0.0.5  | >= 0.0.3       |
|    0.1.1     | >=0.24.0, <0.25 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<1.5    | >=3.0.0         | >= 0.0.4  | >= 0.0.3       |
|    0.1.0     | >=0.23.9, <0.24 | >=3.30.1, <4    | >=3.0.0, <4     | =1.2.0          | >=3.0.0         | >= 0.0.4  | >= 0.0.3       |

## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md) to know how to contribute and set up a development environment.

## License

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the [LICENSE](LICENSE) file for details.
