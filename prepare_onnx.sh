#!/bin/bash

VERSION="1.21.0"
TARGET_DIR="libs/onnxruntime"

mkdir -p "$TARGET_DIR"

FILE_NAME="onnxruntime-linux-x64-${VERSION}.tgz"
DOWNLOAD_URL="https://github.com/microsoft/onnxruntime/releases/download/v${VERSION}/${FILE_NAME}"

echo "Downloading ONNX Runtime version $VERSION from $DOWNLOAD_URL..."

curl -L -o "$FILE_NAME" "$DOWNLOAD_URL"

if [ $? -ne 0 ]; then
    echo "Error: Failed to download ONNX Runtime version $VERSION."
    exit 1
fi

echo "Extracting $FILE_NAME to $TARGET_DIR..."
tar -xzf "$FILE_NAME" -C "$TARGET_DIR" --strip-components=1

if [ $? -ne 0 ]; then
    echo "Error: Extraction failed."
    rm -f "$FILE_NAME"
    exit 1
fi

echo "Cleaning up..."
rm -f "$FILE_NAME"

echo "ONNX Runtime version $VERSION successfully downloaded and extracted to $TARGET_DIR."

