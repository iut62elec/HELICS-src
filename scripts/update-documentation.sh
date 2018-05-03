#!/bin/bash

# Install required python packages
pip3 --version
python --version
pip3 install ghp-import
pip3 install breathe
pip3 install sphinx_rtd_theme
pip3 install nbsphinx
pip3 install sphixcontrib-pandoc-markdown

# also has github target
make doxygen html
