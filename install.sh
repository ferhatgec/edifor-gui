#!/bin/sh

echo 1
sh scripts/get_edifor.sh

echo 2
sudo sh scripts/install_execute.sh

echo 3
sudo sh scripts/install_edifor.sh

echo 4
sudo sh scripts/install_edifor_gui.sh
