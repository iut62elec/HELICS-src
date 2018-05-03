#!/bin/bash

# Install required python packages
pip --version
python --version
pip install ghp-import
pip install breathe
pip install sphinx_rtd_theme
pip install nbsphinx
pip install sphixcontrib-pandoc-markdown

# also has github target
make doxygen html
