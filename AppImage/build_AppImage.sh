#!/bin/bash

echo "Building..."
qmake CONFIG+=release PREFIX=/usr
make -j$(nproc)
make INSTALL_ROOT=appdir -j$(nproc) install
find appdir

echo "Downloading linuxdeployqt..."
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR ; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export VERSION=$(git rev-parse --short HEAD)  # linuxdeployqt uses this for naming the file

echo "Creating AppImage..."
cp ./res/images/stackandconquer_64x64.png ./appdir/stackandconquer.png
./linuxdeployqt*.AppImage ./appdir/usr/share/applications/*.desktop -appimage -bundle-non-qt-libs
find ./appdir -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq

echo "Uploading..."
curl --upload-file StackAndConquer*.AppImage https://transfer.sh/StackAndConquer*.AppImage

echo "Done!"

exit 0
