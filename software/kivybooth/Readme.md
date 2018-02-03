Required packages: libghoto2-dev, cython3

https://kivy.org/docs/installation/installation.html#installation-devel

install Python requirements: sudo pip install -r requirements.txt

after installing requirements additional kivy modules should be installed: sudo garden install androidtabs

compiling kivy: 
git submodules init
git submodules update

cd lib/kivy
sudo pip3 install -e .
