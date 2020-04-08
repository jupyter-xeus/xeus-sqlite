.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Build and configuration
=======================

Build
-----

``xeus-sqlite`` build supports the following options:

- ``BUILD_TESTS``: enables the ``xtest`` and ``xbenchmark`` targets (see below).
- ``DOWNLOAD_GTEST``: downloads ``gtest`` and builds it locally instead of using a binary installation.
- ``GTEST_SRC_DIR``: indicates where to find the ``gtest`` sources instead of downloading them.

The ``BUILD_TESTS`` and ``DOWNLOAD_GTEST`` options are disabled by default. Enabling ``DOWNLOAD_GTEST`` or
setting ``GTEST_SRC_DIR`` enables ``BUILD_TESTS``. If the ``BUILD_TESTS`` option is enabled, the `xtest` target is made available, which builds and run the test suite.
