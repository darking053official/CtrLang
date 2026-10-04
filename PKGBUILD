# Maintainer: darking053official <darking053@protonmail.com>
pkgname=ctrlang
pkgver=0.2.0
pkgrel=1
pkgdesc="Türkçe backend web dili"
arch=('x86_64' 'aarch64')
url="https://github.com/darking053official/CtrLang"
license=('GPL3')
depends=('glibc' 'gcc-libs')
makedepends=('git' 'make' 'gcc')
source=("$pkgname-$pkgver.tar.gz::https://github.com/darking053official/CtrLang/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('6e8d5c2b2b0295c1120de4930a75654ebefb7bd7ad0f39502bf79a0e5facf472')

build() {
    cd "$srcdir/CtrLang-$pkgver"
    make
}

package() {
    cd "$srcdir/CtrLang-$pkgver"
    install -Dm755 ctrc "$pkgdir/usr/bin/ctrc"
    ln -sf ctrc "$pkgdir/usr/bin/ctr"
    install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
