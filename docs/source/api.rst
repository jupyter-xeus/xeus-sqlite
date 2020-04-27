===
API
===

-----
Magic
-----

Magics here will allow you to make operations on your Database.

.. object:: %LOAD <"path-to-db/yourdatabase.db"> [R | RW]

   Loads a database.
   
   Receives two arguments, the path to the database location as a string and an option to open the database either as read and write "RW" or read only mode "R".
   If the optional argument is not set it will default to read and write mode.


.. object:: %CREATE <"path-to-db/yourdatabase.db">

   Creates a database in read and write mode.

   Receives one argument, a string that's the path for the database.


.. object:: %DELETE

   Deletes the database that's currently loaded in the Jupyter application.

   Warning: this will delete the file permanently.


.. object:: %TABLE_EXISTS "table_name"

   Checks if a table exists.


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


.. object:: %IS_UNENCRYPTED

   Test if a file contains an unencrypted database.


.. object:: %LOAD_EXTENSION <"extension">

   Load a module into the current sqlite database instance.

   Receives the name of the shared library containing the extension.

.. object:: %REKEY <"key">

   Reset the key for the current sqlite database instance.
   This is the equivalent of the sqlite3_rekey call and should thus be called after the database has been opened with a valid key. To decrypt a database, call this method with an empty string.

   Receives one argument which is the key you want to reset.


.. object:: %SET_KEY <"key">

   Set the key for the current sqlite database instance.
   This is the equivalent of the sqlite3_key call and should thus be called directly after opening the database.

   Receives one argument which is the key you want to reset.


.. object:: %BACKUP <0, 1>

   Load the contents of a database file on disk into the "main" database of open database connection, or to save the current contents of the database into a database file on disk.

   Receives one argument which is an int that can either be 0 for saving and 1 for loading.