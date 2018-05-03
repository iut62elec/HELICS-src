#!/bin/bash

# Install required python packages
python3 -m pip install --user ghp-import
python3 -m pip install --user breathe
python3 -m pip install --user sphinx_rtd_theme
python3 -m pip install --user nbsphinx
python3 -m pip install --user sphinxcontrib-pandoc-markdown

# also has github target
make doxygen html
