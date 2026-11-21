# アイコン

日本語 | [English](icons.en.md)

この文書では、soccerwindow2 のアイコンの作り方と埋め込み方、アイコンを追加・修正する手順を説明する。

- [概要](#概要)
- [ファイル](#ファイル)
- [アイコンの追加・修正](#アイコンの追加修正)
- [描き方の決まり](#描き方の決まり)
- [アプリのアイコン](#アプリのアイコン)
- [palette.txt](#palettetxt)
- [generate.sh](#generatesh)
- [Qt のリソースシステム（.qrc）](#qt-のリソースシステムqrc)
- [テスト](#テスト)

## 概要

- アイコンは1つずつ、明るい配色用の色で SVG ファイル（`icons/svg/NAME.svg`）に描く。
- `icons/generate.sh` が Inkscape を使い、SVG ファイルから4つの PNG ファイルを作る。
  明るい配色用と暗い配色用の、それぞれ 16×16 と 32×32 ピクセルの画像である。
  暗い配色用の色は `icons/palette.txt` から取る。
- PNG ファイルはリポジトリにコミットするので、Inkscape が必要なのは PNG ファイルを作るときだけである。
  ビルドと CI には Inkscape は不要である。
- PNG ファイルは、Qt のリソースシステム（`icons/icons.qrc`）でプログラムに埋め込む。
- プログラムは `UIIcon::fromName( "NAME" )`（`src/qt/ui_icon.h`）でアイコンを作る。
  アイコンを描くたびに今の配色（明るい・暗い）に合った画像を使うので、「View > UI Color Scheme」を切り替えるとアイコンも切り替わる。
  32×32 の画像は、HiDPI の画面（device pixel ratio が 2 など）と 32×32 のアイコンで使う。

DebugMessageWindow のデバッグレベルの番号のアイコン（1〜32）は、`UIIcon::fromNumber( N )` が実行時に描く。
数字の形は `src/qt/ui_icon.cpp` の `DIGIT_SHAPES`（SVG のパスと同じ書き方）にあり、アイコンの文字と同じく 2ピクセルの線で、高さ 10ピクセルに描く。
色は `palette.txt` の `red` と同じ色を `src/qt/ui_icon.cpp` に書いている。

アプリのアイコン（ウィンドウのアイコン）は `icons/app/` にあり、配色では切り替えない（[アプリのアイコン](#アプリのアイコン)）。

## ファイル

```
icons/
  svg/NAME.svg          アイコンの元のファイル（このファイルを編集する）
  light/16/NAME.png     明るい配色用のアイコン（16×16）    } generate.sh
  light/32/NAME.png     明るい配色用のアイコン（32×32）    } が作る
  dark/16/NAME.png      暗い配色用のアイコン（16×16）      }
  dark/32/NAME.png      暗い配色用のアイコン（32×32）      }
  palette.txt           色の対応表（明るい配色用 → 暗い配色用）
  palette.awk           色の検査と置き換え（generate.sh とテストが使う）
  generate.sh           PNG ファイルと下の一覧を作る
  app/svg/NAME.svg      アプリのアイコンの元のファイル（32×32 の格子）
  app/svg/NAME.SIZE.svg アプリのアイコンの、ある大きさ専用の元のファイル（あれば）
  app/SIZE/NAME.png     アプリのアイコン（SIZE: 16、32、48、64、128、256） } generate.sh が作る
  app/make_soccerwindow2_svg.py  soccerwindow2 のアイコンの SVG を作るスクリプト
  icons.qrc             Qt のリソースファイル（PNG ファイルの一覧）   } generate.sh
  icons.am              Automake 用のファイルの一覧                  } が作る
  generated.sha256      SVG ファイルと palette.txt のチェックサム    }
test/check_icons.sh     上のファイルを検査する
```

「generate.sh が作る」ファイルは、手で編集しない。

## アイコンの追加・修正

1. Inkscape（またはテキストエディタ）で `icons/svg/NAME.svg` を描く・編集する。[描き方の決まり](#描き方の決まり)に従うこと。
   NAME に使える文字は `a-z`、`0-9`、`_` だけである。
2. PNG ファイルを作り、一覧を更新する。
   ```
   icons/generate.sh NAME
   ```
3. ファイルを検査し、PNG ファイルを目で確かめる。
   ```
   test/check_icons.sh
   ```
4. （新しいアイコンの場合）ソースコードでアイコンを使う。
   ```cpp
   #include "ui_icon.h"

   QAction * act = new QAction( UIIcon::fromName( "NAME" ), tr( "..." ), this );
   ```
5. ビルドしてテストを実行する（`./cmake-helper.sh test`）。
   CMake は新しいファイルを自動で埋め込む。
   Autotools では、`icons/icons.am` が変わるので、`make` が automake を自動で実行し直す。
6. `icons/svg/NAME.svg`、4つの PNG ファイル、`icons/icons.qrc`、`icons/icons.am`、`icons/generated.sha256` をコミットする。

アイコンを削除するときは、SVG ファイルと4つの PNG ファイルを削除し（`git rm`）、`icons/generate.sh --lists` を実行する。

## 描き方の決まり

アイコンの線の太さと色合いをそろえるため、次の決まりに従う。

- 大きさ: `width="16" height="16" viewBox="0 0 16 16"`。
  32×32 の画像は、同じ SVG ファイルを2倍の大きさで出力して作る。
- 画素の格子: 1ピクセルの線は画素の中心（例: `x="1.5"`）に、塗りの図形の端は画素の境界（例: `x="2"`）に置き、16×16 の画像がぼやけないようにする。
  そうすれば、32×32 の画像もぼやけない。
- 余白: ほとんどのアイコンは 1〜15 の範囲（14×14 ピクセル）に描く。
  図形に必要なら、輪郭が端に触れてもよい。
- 線: 輪郭は 1ピクセルで、色は `fg`。
  記号（矢印、×など）は 1 か 2ピクセルの線で描く。
  線はなるべく水平・垂直・45° にする。
- 小さな丸（外径 7ピクセル以下）: `circle` で描くと 16×16 でぼやけるので、画素にそろえた角丸の四角で描く。
  `rx` は幅の半分より 0.5 小さくする。
  例: 外径 5ピクセルの丸は `<rect x="1.5" y="1.5" width="4" height="4" rx="1.5"/>`、外径 7ピクセルの丸は `width="6" height="6" rx="2.5"`。
  大きな丸（record の丸など）は `circle` でよい。
- ログプレイヤーの記号（再生、停止、コマ送りなど）: 三角は高さ 10ピクセル（y 3〜13）・幅 5ピクセル（45° の辺）、棒は幅 2ピクセルにそろえる。
  並べて使うので、大きさをそろえる。
- 塗り: 部品のある物（フォルダ、書類など）には `fg` の輪郭を付ける。
  単純な色の印（record の赤い丸など）には輪郭を付けない。
  黒い記号（再生の三角など）は `fg` で塗り、輪郭を付けない。
- 色: `palette.txt` の明るい配色用の色（LIGHT）だけを使う。
  色を薄くするために透明度を使わず、`palette.txt` に色を加える。
- 選手やチームを表すアイコンには、soccerwindow2 のフィールドの描画で同じ意味を持つ色を使う。

Inkscape で編集したときは、`palette.txt` にない色（グラデーションや、スポイトで拾った色など）が加わっていないか確かめる。
`generate.sh` と `test/check_icons.sh` がそのような色を報告する。

## アプリのアイコン

アプリのアイコン（ウィンドウのアイコンなど）は、ツールバーのアイコンとは次の点が違う。

- 元のファイルは `icons/app/svg/NAME.svg`。32×32 の格子（`viewBox="0 0 32 32"`）で描く。
- `generate.sh` が `icons/app/SIZE/NAME.png`（SIZE: 16、32、48、64、128、256）を作る。
- 小さな大きさで細部がつぶれるときは、その大きさ専用の元のファイル `icons/app/svg/NAME.SIZE.svg`（例: `fedit2.16.svg`。16×16 の格子で描く）を置く。その大きさだけ、そちらから作る。
- ウィンドウマネージャやデスクトップ環境が表示するので、配色では切り替えない（暗い配色用は作らない）。そのため、色は `palette.txt` に縛られない。
- プログラムは `UIIcon::appIcon( "NAME" )` で、6つの大きさの画像を持つアイコンを作る（例: `setWindowIcon( UIIcon::appIcon( "soccerwindow2" ) )`）。

今のアプリのアイコンは次の2つである。

| 名前 | 使う場所 |
|---|---|
| soccerwindow2 | MainWindow のウィンドウのアイコン、About メニューのアイコン。インストールもする（16px は `soccerwindow2.16.svg` から作る） |
| fedit2 | FormationEditorWindow のウィンドウのアイコン（16px は `fedit2.16.svg` から作る） |

soccerwindow2 のアイコンは、斜め上から見た fedit2 のフィールド（選手とフォーメーションの三角形）の手前にサッカーボールを置いた絵である。
ボールの模様（切頂二十面体を球面に投影したもの）とフィールドの遠近は計算で描くので、
SVG は `icons/app/make_soccerwindow2_svg.py`（Python 3 の標準ライブラリだけを使う）が作る。
ボールの向きや位置を変えるときは、このスクリプトを直して実行し、`icons/generate.sh soccerwindow2` で PNG を作り直す。

```
python3 icons/app/make_soccerwindow2_svg.py
icons/generate.sh soccerwindow2
```

soccerwindow2 のアイコンは、インストールのときに freedesktop.org の hicolor テーマの場所に置く
（`PREFIX/share/icons/hicolor/SIZExSIZE/apps/soccerwindow2.png` と `PREFIX/share/icons/hicolor/scalable/apps/soccerwindow2.svg`）。
CMake はトップの `CMakeLists.txt`、Autotools はトップの `Makefile.am`（`hicolor*_DATA`）でインストールする。

## palette.txt

各行は、役割、明るい配色用の色（LIGHT）、暗い配色用の色（DARK）の3つの欄からなる。

```
# role      light    dark
fg          #303030  #e0e0e0
paper       #ffffff  #505050
```

- SVG ファイルでは LIGHT の色だけを使う。
  `generate.sh` は、LIGHT の色をそれぞれ DARK の色に置き換えて、暗い配色用のアイコンを作る。
- 色は `#rrggbb` の形（小文字）で書く。役割と LIGHT の色は重複してはいけない。
- 両方の配色で同じ色にしたいとき（例: フィールドの白線 `field_line`）は、LIGHT と DARK に同じ色を書く。
  ただし、LIGHT の色はほかの役割と重複できないので、例えば白は `paper`（#ffffff）と区別して #f8f8f8 にしている。
- DARK の色は、暗い背景（例: `#323232`）に対して、LIGHT の色が明るい背景（例: `#efefef`）に対して持つのと同じくらいのコントラストになるように選ぶ。
- `palette.txt` を変えたら、`icons/generate.sh`（名前を指定しない）を実行して、すべてのアイコンを作り直す。
  名前を指定しても、`generate.sh` が自動ですべてを作り直す。

## generate.sh

```
icons/generate.sh [NAME...]     アイコンの PNG ファイルを作る（省略するとすべてのアイコン。アプリのアイコンも同じ）
icons/generate.sh --lists       icons.qrc と icons.am だけを更新する（Inkscape は使わない）
icons/generate.sh --help        使い方を表示する
```

- Inkscape 1.0 以降が必要である。必要なら、環境変数 `INKSCAPE` でコマンドを指定する。
  例: `INKSCAPE=/snap/bin/inkscape`、`INKSCAPE="flatpak run org.inkscape.Inkscape"`。
- 実行するコマンドを、実行する前に表示する。例:
  ```
  + inkscape --pipe --export-type=png --export-filename=- --export-background-opacity=0 -w 16 -h 16 < svg/open.svg > light/16/open.png
  + awk -v mode=dark -f palette.awk palette.txt svg/open.svg | inkscape --pipe ... -w 16 -h 16 > dark/16/open.png
  ```
  SVG のデータを標準入力から渡し、PNG のデータを標準出力で受け取るので、Inkscape はファイルを自分では読み書きしない
  （snap でインストールした Inkscape は隠しディレクトリのファイルを読めず、シェルのカレントディレクトリも使わないため）。
- Inkscape を実行する前に、SVG ファイルの色を `palette.txt` で検査する。
- PNG ファイルを作った後、`icons.qrc`、`icons.am`、`generated.sha256` を更新する。
  `generated.sha256` には、PNG ファイルを作ったときの `palette.txt` と SVG ファイルのチェックサムを記録する。
  `test/check_icons.sh` はこれを使って、`generate.sh` を実行せずに編集した SVG ファイルを見つける。

## Qt のリソースシステム（.qrc）

Qt のリソースシステムは、ファイルをプログラムに埋め込む仕組みである。
埋め込むファイルをリソースファイル（`.qrc`）に並べると、リソースコンパイラ `rcc` がそれらを C++ のソースファイルに変換し、プログラムにコンパイル・リンクされる。
プログラムは、`:/` で始まるパスで、ディスク上のファイルと同じように読み込める
（例: `QPixmap( ":/icons/light/16/open.png" )`）。

### 書式

`icons/icons.qrc` は XML のファイルである。

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
<qresource prefix="/icons">
    <file>light/16/open.png</file>
    <file>light/32/open.png</file>
    <file>dark/16/open.png</file>
    <file>dark/32/open.png</file>
</qresource>
</RCC>
```

- `<file>` には、`.qrc` ファイルのあるディレクトリからの相対パスでファイルを書く。
- `<qresource>` の `prefix` は、プログラムで使うパスの前に付く。
  ファイル `icons/light/16/open.png` は `:/icons/light/16/open.png` で読み込む。
- `alias` 属性で別の名前を付けられる。`<file alias="open.png">light/16/open.png</file>` は `:/icons/open.png` で読み込む。
- `rcc` は、サイズが小さくなる場合にファイルを圧縮する。PNG ファイルは、ふつうそのまま格納される。

`icons/icons.qrc` は `generate.sh` が SVG ファイルの一覧から作るので、手で編集する必要はない。
ほかのファイル（ほかの画像など）を埋め込むときは、同じ書式で別の `.qrc` ファイルを作り、`icons/icons.qrc` と同じようにビルドに加える（下記）。

### ビルド

CMake（`src/qt/CMakeLists.txt`）: `.qrc` ファイルをターゲットのソースに加え、`AUTORCC` で `rcc` を実行する。
CMake は `.qrc` ファイルに載っているファイルを知っているので、それらが変わると `rcc` を実行し直す。

```cmake
add_library(soccerwindow2_gui STATIC
  ...
  ${PROJECT_SOURCE_DIR}/icons/icons.qrc
  )
set_target_properties(soccerwindow2_gui PROPERTIES AUTOMOC ON AUTORCC ON)
```

Autotools（`src/qt/Makefile.am`）: `configure` が選んだ Qt の `rcc` を探し（`QT_RCC=/path/to/rcc` で指定もできる）、`make` が `qrc_icons.cpp` を作る。

```make
ICONS_DIR = $(top_srcdir)/icons
include $(top_srcdir)/icons/icons.am

qrc_icons.cpp: $(ICONS_DIR)/icons.qrc $(ICONS_PNG_FILES)
	$(QT_RCC) --name icons --output $@ $(ICONS_DIR)/icons.qrc
```

`qrc_icons.cpp` は `nodist_soccerwindow2_SOURCES` に載せている。
トップの `Makefile.am` でも `icons/icons.am` を `include` し、SVG と PNG のファイルを配布用の tarball に加えている（`EXTRA_DIST`）。

### 初期化

静的ライブラリに入れたリソースは、使う前に `Q_INIT_RESOURCE( NAME )` で登録しなければならない。
NAME は `.qrc` ファイルの名前から拡張子を除いたもの（または `rcc --name` で指定した名前）である。
登録しないと、リンカがリソースを取り除いてしまうことがある。CMake のビルドでは、アイコンは静的ライブラリ `soccerwindow2_gui` にリンクされる。

`UIIcon::fromName()` は、`Q_INIT_RESOURCE( icons )` を呼ぶ `UIIcon::initResources()` を自動で呼ぶ。
`:/icons/...` のパスでファイルを直接読み込む場合は、先に `UIIcon::initResources()` を呼ぶこと。
`Q_INIT_RESOURCE` は、グローバル名前空間の関数から呼ばなければならない。

## テスト

- `test/check_icons.sh`（CTest の `check_icons` と `make check`）は、Inkscape を使わずに次のことを検査する。
  - `palette.txt` が正しく、SVG ファイルがその LIGHT の色だけを使っていること。
  - 各 SVG ファイルに、正しい大きさの4つの PNG ファイルがあり、SVG ファイルのない PNG ファイルがないこと。
  - 各アプリのアイコンに、正しい大きさの6つの PNG ファイルがあり、大きさ専用の SVG ファイルが正しい大きさのものであること。
  - `icons.qrc` と `icons.am` に、ちょうどこれらのファイルが載っていること。
  - PNG ファイルが、今の SVG ファイルと `palette.txt` から作られていること（`generated.sha256`）。
- GUI のテスト（`test/test_gui_smoke.cpp`）は、アイコンが埋め込まれていること、
  `UIIcon::fromName()` が今の配色と device pixel ratio に合った画像を使うこと、
  ツールバーのボタンが今の配色に合った画像で描かれること、
  ウィンドウのアイコンが6つの大きさのアプリのアイコンであることを検査する。
