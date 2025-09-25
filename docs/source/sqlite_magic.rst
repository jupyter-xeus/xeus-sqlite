SQLite magics
=============

Magics that allow you to operate on the database.

LOAD
~~~~

.. object:: %LOAD <path-to-db/yourdatabase.db> [r | rw]

   Loads a database.
   
   Receives two arguments, the path to the database location as a string (it can be either the local or absolute path) and an option to open the database either as read and write "RW" or read only mode "R".
   If the optional argument is not set it will default to read and write mode.

CREATE
~~~~~~

.. object:: %CREATE <path-to-db/yourdatabase.db> name_of_database

   Creates a database in read and write mode.

   Receives two arguments: a string that's the path to where it will create the database, and a string for the name of the database.

DELETE
~~~~~~

.. object:: %DELETE

   Deletes the database that's currently loaded in the Jupyter application.

   Warning: this will delete the file permanently.

TABLE_EXISTS
~~~~~~~~~~~~

.. object:: %TABLE_EXISTS table_name

   Checks if a table exists.

GET_INFO
~~~~~~~~

.. object:: %GET_INFO

   Get the following information about the loaded database:

   * Magic header string
   * Page size bytes
   * File format write version
   * File format read version
   * Reserved space bytes
   * Max embedded payload fraction
   * Min embedded payload fraction
   * Leaf payload fraction
   * File change counter
   * Database size pages
   * First freelist trunk page
   * Total freelist trunk pages
   * Schema cookie
   * Schema format number
   * Default page cache size bytes
   * Largest B tree page number
   * Database text encoding
   * User version
   * Incremental vaccum mode
   * Application ID
   * Version valid for
   * SQLite version

IS_UNENCRYPTED
~~~~~~~~~~~~~~

.. object:: %IS_UNENCRYPTED

   Test if a file contains an unencrypted database.

LOAD_EXTENSION
~~~~~~~~~~~~~~

.. object:: %LOAD_EXTENSION <extension>

   Load a module into the current sqlite database instance.

   Receives the name of the shared library containing the extension.

REKEY
~~~~~

.. object:: %REKEY <key>

   Reset the key for the current sqlite database instance.
   This is the equivalent of the sqlite3_rekey call and should thus be called after the database has been opened with a valid key. To decrypt a database, call this method with an empty string.

   Receives one argument which is the key you want to reset.

SET_KEY
~~~~~~~

.. object:: %SET_KEY <key>

   Set the key for the current sqlite database instance.
   This is the equivalent of the sqlite3_key call and should thus be called directly after opening the database.

   Receives one argument which is the key you want to reset.

BACKUP
~~~~~~

.. object:: %BACKUP <0, 1>

   Load the contents of a database file on disk into the "main" database of open database connection, or to save the current contents of the database into a database file on disk.

   Receives one argument which is an int that can either be 0 for saving and 1 for loading.
