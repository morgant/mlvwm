pkgname=mlvwm
pkgver=master
pkgrel=1
pkgdesc='Unoffical package for Macintosh-Like Virtual Window Manager, it attempts to emulate the pre-Mac OS X Macintosh look and feel in its layout and window design.'
arch=('i686' 'x86_64')
url="http://www2u.biglobe.ne.jp/~y-miyata/mlvwm.html"
license=('mit')
depends=('libxpm')
makedepends=('imake')
source=(https://github.com/saintofvice/${pkgname}/archive/${pkgver}.zip)

build() {
  cd ${pkgname}
  # https://github.com/morgant/mlvwm/issues/40
#  sed -i -e 's/void (\*action)()/void (\*action)(char \*)/g' mlvwm/functions.h
#  sed -i -e '182d' mlvwm/mlvwm.h
#  sed -i -e 's|#define DEFAULTFS.*|#define DEFAULTFS "-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-iso8859-1"|' mlvwm/mlvwm.h
#  sed -i -e 's|#define DEFAULTFONT.*|#define DEFAULTFONT "-misc-chicago kare-*-*-r-*-*-12-*-*-*-p-*-*-*"|' mlvwm/mlvwm.h
  cd man && xmkmf && cd -
  cd sample_rc && xmkmf && cd -
  cd mlvwm && xmkmf && cd -
  xmkmf
  make
  #make PREFIX="$pkgdir"/usr BINARY=$pkgname install man
}
package () {
  cd ${pkgname}
  sed -n '1,34 p'< mlvwm/mlvwm.c >license
  mkdir -p "$pkgdir"/usr/share/licenses/$pkgname/
  mkdir -p "$pkgdir"/usr/share/man/man1/
  mkdir -p "$pkgdir"/usr/bin
  mkdir -p "$pkgdir"/usr/share/$pkgname
  cp -R sample_rc/Mlvwmrc* "$pkgdir"/usr/share/$pkgname
  install -m755 mlvwm/mlvwm "$pkgdir"/usr/bin/$pkgname
  install -c -m444 man/mlvwm._man "$pkgdir"/usr/share/man/man1/mlvwm.1x
  install -m644 license "$pkgdir"/usr/share/licenses/$pkgname/
}

