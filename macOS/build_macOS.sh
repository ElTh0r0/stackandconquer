#!/bin/bash

set -o errexit -o nounset

# Hold on to current directory
project_dir=$(pwd)

# Output macOS version
sw_vers

# Update platform
echo "Updating platform..."
brew update

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
qmake -config release
make

# Build and run tests here

# Package
echo "Packaging..."
cd ${project_dir}/build/macOS/clang/x86_64/release/

# Remove build directories that should not be deployed
rm -rf moc
rm -rf obj
rm -rf qrc

echo "Creating dmg archive..."
macdeployqt StackAndConquer.app -qmldir=../../../../../src -dmg
mv StackAndConquer.dmg "StackAndConquer_${TAG_NAME}.dmg"

# Copy other project files
cp "${project_dir}/README.md" "README.md"
cp "${project_dir}/COPYING" "COPYING"
cp -r "${project_dir}/data/cpu/" "cpu/"

echo "Packaging zip archive..."
7z a StackAndConquer_${TAG_NAME}_macos.zip "StackAndConquer_${TAG_NAME}.dmg" "README.md" "COPYING" "cpu/"

echo "Done!"

exit 0
