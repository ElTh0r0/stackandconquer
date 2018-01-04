#!/bin/bash

set -o errexit -o nounset

# Hold on to current directory
project_dir=$(pwd)

# Output macOS version
sw_vers

# Update platform (enable if needed)
#echo "Updating platform..."
#brew update

# Install p7zip for packaging
brew install p7zip

# Install Qt
echo "Installing Qt..."
brew install qt

# Add Qt binaries to path
PATH=/usr/local/opt/qt/bin/:${PATH}

# Build app
echo "Building..."
cd ${project_dir}
mkdir build
cd build
qmake ../stackandconquer.pro CONFIG+=release
make

# Build and run tests here

# Package
echo "Packaging..."

# Remove build directories that should not be deployed
rm -rf moc
rm -rf obj
rm -rf qrc

echo "Creating dmg archive..."
macdeployqt StackAndConquer.app -dmg
mv StackAndConquer.dmg "StackAndConquer_${TAG_NAME}.dmg"

# Copy other project files
cp "${project_dir}/README.md" "${project_dir}/build/README.md"
cp "${project_dir}/COPYING" "${project_dir}/build/COPYING"
cp -r "${project_dir}/data/cpu/" "${project_dir}/build/cpu/"

echo "Packaging zip archive..."
7z a StackAndConquer_${TAG_NAME}_macos.zip "StackAndConquer_${TAG_NAME}.dmg" "README.md" "COPYING" "cpu/"

echo "Done!"

exit 0
