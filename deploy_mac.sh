#!/usr/bin/env bash

build_folder=$1
app_path=$build_folder/DeepLabel.app

macdeployqt $app_path
python fix_paths_mac.py $app_path/Contents/MacOS/DeepLabel
hdiutil create -volname DeepLabel -srcfolder $app_path -ov -format UDZO $build_folder/deeplabel.dmg
