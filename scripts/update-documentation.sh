#!/bin/bash

python3 --version
pip --version
pip3 --version

# Install required python packages
pip install -U sphinx
pip install -U ghp-import
pip install -U breathe
pip install -U sphinx_rtd_theme
pip install -U nbsphinx
pip install -U git+https://github.com/kdheepak/sphinxcontrib-pandoc-markdown.git@fix-StringList-AttributeError
pip install -U ipython

# also has github target
make doxygen html

