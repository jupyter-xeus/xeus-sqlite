.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Getting started
===============

Launch Jupyter notebook with `jupyter notebook` or Jupyter lab with
`jupyter lab` and create a new SQLite notebook by selecting the **xsqlite**
kernel. Or launch Jupyter console with `jupyter console --kernel xsqlite`.

To perform operations on `xeus-sqlite` you must have a loaded database. You can do that using the `%LOAD` or `%CREATE` magics. Magics are commands that allow you to perform operations that are not necessarily SQLite code, we give a brief example on how to use the mentioned ones here, for more extensive explanation on magics access the main page_.

Creating a new database
-----------------------

If you want to create a new database from scratch you can use: ::

    %CREATE "path-to-db/yourdatabase.db"

This will create a `yourdatabase.db` file in the specified location.

Loading a new database
----------------------

If you already have a database you can load it with: ::

    %LOAD "path-to-db/yourdatabase.db"

Usage
-----

Now you can run normal SQLite code and the changes will take place on this database. ::

   Warning: the changes made while in the jupyter interface are permanent to your database.

To change the database you're working with simply run the `%LOAD` or `%CREATE` magic with a new target.

Recommendations
---------------

It's recommended to use words from the syntax in upper case and arguments in lower case simply to improve readability. The input strings are sanitized and it doesn't matter if the input is written in upper or lower case, **with the only exception** of column names in SQLite code (and all the other places where they're referenced).

.. _page: api
