# by courtesy of Ernst

I attached the makefile that I used to compile scxmlrun on OSX:

I run make with:
QTVERSION=5.11.2 make
make install

And the installed dependencies:
brew install coreutils
PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"

brew tap nlohmann/json
brew install nlohmann_json

brew install mosquitto

brew install qt5
brew link qt5 --force

brew install gcc@6

Kind regards,
Ernst 
