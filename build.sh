TERMUX_PKG_HOMEPAGE=https://github.com/darking053official/CtrLang
TERMUX_PKG_DESCRIPTION="Türkçe backend web dili"
TERMUX_PKG_LICENSE="GPL-3.0"
TERMUX_PKG_MAINTAINER="@darking053official"
TERMUX_PKG_VERSION=0.2.0
TERMUX_PKG_SRCURL=https://github.com/darking053official/CtrLang/archive/refs/tags/v${TERMUX_PKG_VERSION}.tar.gz
TERMUX_PKG_SHA256=6e8d5c2b2b0295c1120de4930a75654ebefb7bd7ad0f39502bf79a0e5facf472
TERMUX_PKG_BUILD_IN_SRC=true
TERMUX_PKG_DEPENDS="clang"

termux_step_make() {
    make
}

termux_step_make_install() {
    install -Dm755 ctrc "$TERMUX_PREFIX/bin/ctrc"
    ln -sf ctrc "$TERMUX_PREFIX/bin/ctr"
    install -Dm644 README.md "$TERMUX_PREFIX/share/doc/ctrlang/README.md"
}
