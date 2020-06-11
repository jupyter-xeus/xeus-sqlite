.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Usage
=====

Launch the Jupyter notebook with `jupyter notebook` or Jupyter lab with
`jupyter lab` and launch a new SQLite notebook by selecting the **xsqlite**
kernel.

The first step is to load a SQLite database file, using ``%LOAD "filename.db"``.

If you don't have one you can create an empty file using ``%CREATE``::

    %CREATE "test.db"

Load the database in read-write mode using ``%LOAD``::

    %LOAD "test.db"

Now you can run SQL statements in notebook cells::

    select sqlite_version()

Or create new tables::

   create table demo (id integer primary key, name text)
