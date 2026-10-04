#!/bin/sh
# CtrLang kurulum scripti
# Kullanım: curl -fsSL https://darking053official.github.io/CtrLang/kur | sh

set -e

echo "CtrLang kuruluyor..."

# Platform algıla
if [ -d "/data/data/com.termux" ]; then
    PLATFORM="termux"
    BIN_DIR="$PREFIX/bin"
    ARCH=$(uname -m)
    echo "Platform: Android (Termux) - $ARCH"
elif [ "$(uname -s)" = "Darwin" ]; then
    PLATFORM="macos"
    BIN_DIR="/usr/local/bin"
    ARCH=$(uname -m)
    echo "Platform: macOS - $ARCH"
elif [ "$(uname -s)" = "Linux" ]; then
    PLATFORM="linux"
    BIN_DIR="/usr/local/bin"
    ARCH=$(uname -m)
    echo "Platform: Linux - $ARCH"
else
    echo "Desteklenmeyen platform"
    exit 1
fi

# Sürüm
VERSION="v0.2.0"
BASE_URL="https://github.com/darking053official/CtrLang/releases/download/$VERSION"

# Mimari belirle
case "$ARCH" in
    x86_64|amd64)
        if [ "$PLATFORM" = "macos" ]; then
            DOSYA="ctrc-macos-x64.zip"
        else
            DOSYA="ctrc-linux-x64.zip"
        fi
        ;;
    aarch64|arm64)
        if [ "$PLATFORM" = "macos" ]; then
            DOSYA="ctrc-macos-arm64.zip"
        else
            DOSYA="ctrc-linux-arm64.zip"
        fi
        ;;
    *)
        echo "Desteklenmeyen mimari: $ARCH"
        exit 1
        ;;
esac

echo "Indiriliyor: $DOSYA"

# Geçici klasör
TMP_DIR=$(mktemp -d)
cd "$TMP_DIR"

# İndir
if command -v curl >/dev/null 2>&1; then
    curl -fsSL -o ctrc.zip "$BASE_URL/$DOSYA"
elif command -v wget >/dev/null 2>&1; then
    wget -q -O ctrc.zip "$BASE_URL/$DOSYA"
else
    echo "curl veya wget gerekli"
    exit 1
fi

# Aç
if command -v unzip >/dev/null 2>&1; then
    unzip -q ctrc.zip
else
    echo "unzip gerekli"
    exit 1
fi

# Kur
chmod +x ctrc
if [ "$PLATFORM" = "termux" ]; then
    cp ctrc "$BIN_DIR/ctrc"
    ln -sf "$BIN_DIR/ctrc" "$BIN_DIR/ctr"
else
    sudo cp ctrc "$BIN_DIR/ctrc"
    sudo ln -sf "$BIN_DIR/ctrc" "$BIN_DIR/ctr"
fi

# Temizle
cd /
rm -rf "$TMP_DIR"

echo ""
echo "CtrLang kuruldu!"
echo ""
echo "Test:"
ctr --surum
echo ""
echo "Kullanım:"
echo "  ctr --yardim"
echo "  ctr run app.ctr"
