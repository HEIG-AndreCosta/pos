
sudo apt-get install python3-dev python3-pip python3-venv python3-wheel

pip install wheel

In python-sense-emu:
python3 setup.py bdist_wheel

and move the dist/<package>.whl in sense-hat/ directory

python3 -m pip install <package>.whl

-> Copy ~/.local/bin/* to /usr/local/bin

// Deprecated
To be executed in python-sense-hat:

# PYTHONPATH=../out/lib/python2.7/site-packages/ python setup.py install --prefix=../out

