#!/bin/sh

echo "Get source code of edifor. (latest)"
sh scripts/get_edifor.sh

echo "Install dependencies."
sudo sh scripts/install_execute.sh

echo "Install edifor. (latest)"
sudo sh scripts/install_edifor.sh

echo "Install ediforGUI"
sudo sh scripts/install_edifor_gui.sh

sudo mkdir /usr/share/pixmaps/edifor/

sudo cp resource/*.png /usr/share/pixmaps/edifor/

sudo cp edifor.desktop /usr/share/applications/
