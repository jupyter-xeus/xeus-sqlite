.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

.. raw:: html

   <style>
   .rst-content .section>img {
       width: 30px;
       margin-bottom: 0;
       margin-top: 0;
       margin-right: 15px;
       margin-left: 15px;
       float: left;
   }
   </style>

Installation
============

With Conda
----------

`xeus-sqlite` has been packaged for the conda package manager.

To ensure that the installation works, it is preferable to install `xeus-sqlite` in a fresh conda environment.
It is also needed to use a miniconda_ installation because with the full anaconda_ you may have a conflict with
the `zeromq` library which is already installed in the anaconda distribution.


The safest usage is to create an environment named `xeus-sqlite` with your miniconda installation

.. code::

    conda create -n xeus-sqlite
    conda activate xeus-sqlite # Or `source activate xeus-sqlite` for conda < 4.6

Then you can install in this freshly created environment `xeus-sqlite` and its dependencies

.. code::

    conda install xeus-sqlite notebook -c conda-forge

or, if you prefer to use JupyterLab_

.. code::

    conda install xeus-sqlite jupyterlab -c conda-forge

From Source
-----------

You can install ``xeus-sqlite`` from source with cmake. This requires that you have all the dependencies installed in the same prefix.

.. code::

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    make install

On Windows platforms, from the source directory:

.. code::

    mkdir build
    cd build
    cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    nmake
    nmake install

.. _miniconda: https://conda.io/miniconda.html
.. _anaconda: https://www.anaconda.com
.. _JupyterLab: https://jupyterlab.readthedocs.io
