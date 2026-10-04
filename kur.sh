#!/usr/bin/env bash
# CtrLang - Otomatik kurulum

set -e

echo "=== CtrLang Kurulum ==="
echo ""

# Platform algıla
if [ -d "/data/data/com.termux" ]; then
    PLATFORM="termux"
    BIN_DIR="$PREFIX/bin"
    echo "Platform: Android (Termux)"
elif [ "$(uname -s)" = "Darwin" ]; then
    PLATFORM="macos"
    BIN_DIR="/usr/local/bin"
    echo "Platform: macOS"
elif [ "$(uname -s)" = "Linux" ]; then
    PLATFORM="linux"
    BIN_DIR="/usr/local/bin"
    echo "Platform: Linux"
else
    echo "❌ Desteklenmeyen platform"
    exit 1
fi

# Derle
echo ""
echo "→ Derleniyor..."
make

if [ ! -f "ctrc" ]; then
    echo "❌ Derleme başarısız"
    exit 1
fi
echo "✅ Derleme başarılı"

# Sisteme kur
echo ""
echo "→ Sisteme kuruluyor: $BIN_DIR"

if [ "$PLATFORM" = "termux" ]; then
    cp ctrc "$BIN_DIR/ctrc"
    ln -sf "$BIN_DIR/ctrc" "$BIN_DIR/ctr"
    chmod +x "$BIN_DIR/ctrc" "$BIN_DIR/ctr"
else
    sudo cp ctrc "$BIN_DIR/ctrc"
    sudo ln -sf "$BIN_DIR/ctrc" "$BIN_DIR/ctr"
    sudo chmod +x "$BIN_DIR/ctrc" "$BIN_DIR/ctr"
fi

echo "✅ Kuruldu: $BIN_DIR/ctrc"
echo "✅ Kısayol: $BIN_DIR/ctr"

# Test
echo ""
echo "→ Test:"
ctrc --platform

echo ""
echo "🎉 Kurulum tamam!"
echo ""
echo "Kullanım:"
echo "  ctr --calistir merhaba.ctr"
echo "  ctrc --yardim"
