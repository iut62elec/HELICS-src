#!/bin/bash

python3 --version
pip --version
pip3 --version

# Install required python packages
python3 -m pip install sphinx
python3 -m pip install ghp-import
python3 -m pip install breathe
python3 -m pip install sphinx_rtd_theme
python3 -m pip install nbsphinx
python3 -m pip install git+https://github.com/kdheepak/sphinxcontrib-pandoc-markdown.git@fix-StringList-AttributeError
python3 -m pip install ipython

# also has github target
make doxygen html

