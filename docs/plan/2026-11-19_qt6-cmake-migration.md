# soccerwindow2 Qt6対応・CMakeメイン化 計画書

- 作成日: 2026-11-19
- 最終更新: 2026-11-19（CMake による配布用 tarball の作成を Phase 5 として追加）
- ステータス: 実装中（Phase 3 まで完了）
- ベースブランチ: `develop`

## 1. 背景と目的

soccerwindow2 は Qt5 を前提に実装されているため、Qt6 環境（Ubuntu 24.04 の Qt 6.4.2 など）ではビルドできない。
主要ディストリビューションが Qt6 へ移行しつつあるため、Qt6 でもビルド・動作できるようにする。
あわせて、すでに使われていない Qt4 向けコードと qmake ビルドを整理する。ビルドシステムは CMake をメインとし、Autotools も引き続きサポートする。
将来は Autotools を外す可能性が高いため、`make dist` / `make distcheck` に相当する配布用 tarball の作成手段も CMake に用意する。

## 2. 決定事項

| 項目 | 決定内容 |
|---|---|
| 対応Qt | **Qt 5.15 以上と Qt 6.2 以上の両対応**。両方が入っている環境では Qt6 を優先し、オプションで Qt5 も選べる |
| Qt4 | サポートを廃止する（`QT_VERSION_CHECK(5, x, 0)` 分岐を削除） |
| `Q_WS_MAC` / `Q_WS_WIN` | 分岐を削除する。Qt5 以降では常に偽なので、現在の挙動は変わらない |
| qmake `.pro` | 削除する（Qt4 時代のライブラリ名を参照しており、すでにビルドできない） |
| ビルドシステム | CMake をメインとし、Autotools はサブとして Qt6 対応を含めて維持する |
| Boost | Boost.System への依存を外し、ヘッダだけを使う（Boost 1.89 で boost_system ライブラリが廃止されたため） |
| テスト | `test/` に置き、CTest に登録する。`--help` での起動テストと、Qt Test による GUI スモークテスト（offscreen）を用意する |
| `make check` | Autotools では `--help` 起動テストだけを実行する。Qt Test のテストは CMake（ctest）だけで実行する |
| 配布用 tarball | `git archive` を使う CMake のターゲット `dist` / `distcheck` を用意する（Phase 5）。ファイル名はバージョン番号を付けず、HEAD のコミット日を付けた `soccerwindow2-YYYYMMDD.tar.gz` とする。開発用のファイルと使われていないファイルは `.gitattributes` の `export-ignore` で除外する |
| CI | GitHub Actions を追加する（Qt5/Qt6 × CMake/Autotools）。ビルドに加えてテストも実行する |
| リリースノート | `RELEASE_NOTES.md` を用意し、新機能や主要な変更点を随時追記する |
| 計画書 | 日本語で作成する。ファイル名は `docs/plan/YYYY-MM-DD_<topic>.md` 形式とする |
| `CLAUDE.md` | コミットせず、`.gitignore` に追加する |
| スコープ外 | `src/qt/obsolete/` の削除、Doxygen の CMake 対応、Windows/macOS 向けパッケージング、文字列ベースの `SIGNAL/SLOT` 接続の全面置換 |

## 3. 現状の調査結果

### 3.1 ビルドシステム

- **CMake**（`CMakeLists.txt`）
  - `cmake_minimum_required(VERSION 3.5)` で、`find_package(Qt5 ...)` に固定されている（Qt4 へのフォールバックあり）。
  - `set(CMAKE_CXX_FLAGS "-W -Wall")` がユーザの指定を上書きしてしまう。
  - Boost を `COMPONENTS system` で要求している。
  - zlib は見つからなくてもよい扱いで検出しているのに、`ZLIB::ZLIB` は無条件にリンクしている。
  - `cmake/FindLibrcsc.cmake` はあるが使われていない。
- **Autotools**（`configure.ac`）
  - Qt の検出に `AX_HAVE_QT`（`m4/ax_have_qt.m4`）を使っている。
  - `qmake6` を見つけても moc を `which moc6` で探すため、検出に失敗する。Ubuntu の Qt6 の moc は `/usr/lib/qt6/libexec/moc` にあり、`qmake6 -query QT_HOST_LIBEXECS` で場所を取得できる。
  - `qtHaveModule` を使って、インストール済みの Qt モジュールを全部リンクしてしまう。
  - `m4/qt.m4` は使われていない。
- **qmake**: `.pro` ファイルは `-lrcsc_agent` など現存しないライブラリを参照しており、ビルドできない。

### 3.2 ソースコード（Qt6 ヘッダでの構文チェック結果）

`g++ -std=c++17 -fsyntax-only` に Qt 6.4.2 のヘッダを与え、`src/qt` と `src/model` の全 .cpp をチェックした結果は次のとおり。

- **コンパイルエラー**（7種類・約15箇所）
  - `QRegExpValidator` / `QRegExp`: `src/qt/action_sequence_selector.cpp:245`
  - `QString + int` の曖昧な変換（括弧の位置を誤った既存バグ）: `src/qt/formation_data_view.cpp:79-81`
  - `QLayout::setMargin` の削除: `src/qt/formation_editor_window.cpp:569,590,685`、`src/qt/simple_label_selector.cpp:131`
  - 修飾キー同士の `+` 結合（`Qt::CTRL + Qt::ALT`。`+ +` の誤記を含む）: `src/qt/main_window.cpp:917,1336,1345,1360,1375`、`src/qt/formation_editor_window.cpp:205,213`、`src/qt/mark_assignment_editor.cpp:260`
- **コンパイルは通るが実行時に壊れる箇所**
  - `SIGNAL( mapped( int ) )` が Qt6 で削除された: `src/qt/debug_message_window.cpp:680`、`src/qt/main_window.cpp:1931`、`src/qt/trainer_dialog.cpp:348,426`
- **非推奨の警告**
  - `Qt::CTRL + Qt::Key_X` 形式（約90箇所）
  - `QMenu::addAction(text, receiver, member, shortcut)`（5箇所）
  - `QMouseEvent::globalPos()`: `src/qt/field_canvas.cpp:480,509,517`
  - `QDropEvent::pos()`（2箇所）
  - ボタンを int で指定する `QMessageBox::question`: `src/qt/image_save_dialog.cpp:567`

### 3.3 Qt4 向けのコードと使われていないコード

- `QT_VERSION` による分岐が85箇所ある。判定はすべて 5.15 以下なので、新しい API 側に一本化できる。
- `Q_WS_MAC` / `Q_WS_WIN` が62箇所（8ファイル）ある。
- `USE_GLWIDGET` 関連のコード（`QGLWidget`）がある。
- Qt6 で削除された `QPainter::HighQualityAntialiasing` は、どこでも定義されていないマクロ `USE_HIGH_QUALITY_ANTIALIASING` の分岐の中にある（当初は `USE_GLWIDGET` のブロック内と判断していたが、Phase 2 の作業中に訂正）。

### 3.4 その他

- `src/model` は Qt に依存していない。
- Boost はヘッダだけを使っている（`boost/lexical_cast.hpp`、librcsc のヘッダが使う `boost/array.hpp` など）。
- `csv_logger.cpp` が CMake のソース一覧にしかなく、ソース一覧が一致していない。

## 4. 実装計画

### 4.0 Git の運用

- 作業ブランチはユーザが GitHub 上で `develop` から作成する（例: `feature/qt6-cmake`）。
- コミットは各フェーズにつき1つを基本とする。変更が大きいフェーズは、作業の種類ごとにコミットを分ける（Phase 2 は A〜E の5コミット、Phase 3 は4コミット）。Phase 3 以降は、段階ごとにユーザが差分を確認してからコミットする。コミット前にコミットメッセージ案と git コマンドを提示し、ユーザの確認を取る。
- コミットメッセージの1行目は英語で書く。本文は日本語を先に、続けて英語を書く。Co-Author は付けない。
- PR は `develop` 向けに1本作る。これまでの履歴に合わせて Squash and merge を推奨する。
- テストは `test/` に置く。`test/` にある既存の未追跡ファイル（作業用）はコミットに含めず、テスト用のファイルは1つずつ指定して `git add` する。

### Phase 0: 計画書とリリースノートの作成

- 本計画書 `docs/plan/2026-11-19_qt6-cmake-migration.md` を作成する。作業中に決まったことは随時この計画書に反映し、末尾の「実装記録」に各フェーズの実施内容を追記する。
- ルートに `RELEASE_NOTES.md`（英語）を新しく作り、`Unreleased` 節を設ける。以降、各フェーズで加えた新機能や主要な変更点を随時追記する。
- `.gitignore` に `CLAUDE.md` を追加する。

### Phase 1: CMake の刷新（メインのビルドシステムにする）

対象: `CMakeLists.txt`、`src/CMakeLists.txt`、`src/model/CMakeLists.txt`、`src/qt/CMakeLists.txt`、`cmake/FindLibrcsc.cmake`、`cmake-config.h.in`

- `cmake_minimum_required(VERSION 3.16)` にする（Qt6 の要件で、Ubuntu 22.04 以降なら満たす）。古い `CMAKE_VERSION < 3.7` の分岐は削除し、`CMAKE_CXX_EXTENSIONS OFF` を設定する。
- Qt はキャッシュ変数 `SOCCERWINDOW2_QT_VERSION`（`AUTO`/`5`/`6`、既定値 `AUTO`）で選べるようにする。
  - `AUTO` の場合は、まず `find_package(Qt6 6.2 QUIET COMPONENTS Core Gui Widgets Network)` で Qt6 を探し、見つかれば 6、見つからなければ 5 を `QT_VERSION_MAJOR` にする。
    - 当初は `find_package(QT NAMES Qt6 Qt5 ...)` を使う予定だった。しかし Qt5 と Qt6 が同じプレフィックス（`/usr/lib/x86_64-linux-gnu/cmake/`）にあると、CMake はディレクトリ順に探すため Qt5 が先に見つかってしまう。そのため Qt6 を明示的に先に探す方式に変えた。
  - そのうえで `find_package(Qt${QT_VERSION_MAJOR} <最低バージョン> REQUIRED COMPONENTS Core Gui Widgets Network)` を呼ぶ（最低バージョンは Qt5 が 5.15、Qt6 が 6.2）。
  - リンクにはバージョンに依存しないターゲット `Qt::Core Qt::Gui Qt::Widgets Qt::Network` を使う。AUTOMOC は soccerwindow2 のターゲットにだけ設定し、使う Qt のバージョンを `message(STATUS)` で表示する。
- librcsc は、既存の `cmake/FindLibrcsc.cmake` を更新して `find_package(Librcsc REQUIRED)` で使う（`CMAKE_MODULE_PATH` に追加）。
  - インポートターゲット `Librcsc::Librcsc` を提供する。
  - 既存のキャッシュ変数 `LIBRCSC_INSTALL_DIR` と環境変数 `LIBRCSC_PREFIX` は引き続き使えるようにする。これらは `HINTS` として、システムの既定パスより先に探す（以前は `PATHS` で、既定パスより後に探していた）。
  - `find_package_handle_standard_args` に渡しているパッケージ名の不一致も直す。
- Boost は、`if(POLICY CMP0167)` のときに NEW を設定したうえで `find_package(Boost 1.41 REQUIRED)` を呼び、`Boost::headers` をリンクする（`system` コンポーネントは要求しない）。最低バージョンは README の記載に合わせて 1.41 にする。
- `sswindow2` は、ビルドディレクトリに生成したファイルをインストールする（以前はソースディレクトリの `src/sswindow2` を参照しており、Autotools を実行していない状態ではインストールに失敗していた）。
- zlib は `ZLIB_FOUND` のときだけ `ZLIB::ZLIB` をリンクする（`HAVE_LIBZ` の定義と合わせる）。
- 警告オプションは `target_compile_options(... PRIVATE -W -Wall)`（GCC/Clang のときのみ）で付け、`CMAKE_CXX_FLAGS` の上書きはやめる。
- インクルードディレクトリは `target_include_directories` でターゲットごとに整理する。model ライブラリは、自身のディレクトリと librcsc・Boost を PUBLIC で伝える。
- インストールには `GNUInstallDirs` を使う（`${CMAKE_INSTALL_BINDIR}`、`${CMAKE_INSTALL_DATADIR}/soccerwindow2`）。既定のインストール先 `$HOME/local` は変えない。`CMAKE_INSTALL_RPATH_USE_LINK_PATH ON` を設定し、`LD_LIBRARY_PATH` を指定しなくても librcsc を見つけられるようにする。
- `.gitignore` に `/build*/` を追加する。
- バージョン番号は `project(VERSION 2026)` と `configure.ac` の `AC_INIT` の2箇所にあるので、更新するときは両方を変える。

### Phase 2: ソースコードからの Qt4 対応の除去と Qt6 対応（Qt5.15 との互換は維持）

対象: `src/qt/` 配下（`obsolete/` を除く）。分岐を書く場合は、既存のスタイル `#if (QT_VERSION >= QT_VERSION_CHECK(...))` に合わせる。

#### 2-1. Qt4 向けのコードと使われていないコードの除去（機械的な作業）

- 次のような include の分岐を `#include <QtWidgets>` だけにする（約48ファイル）。

  ```cpp
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  #include <QtWidgets>
  #else
  #include <QtGui>
  #endif
  ```

- `QT_VERSION_CHECK(5, 0|10|11|15, 0)` による分岐は、すべて新しい API の側に一本化する。
  - 例: `src/qt/main_window.cpp:1782` の `horizontalAdvance`、`src/qt/main_window.cpp:2138` の `angleDelta`、`src/qt/action_sequence_selector.cpp:1067` の `QRandomGenerator`
- `Q_WS_MAC` / `Q_WS_WIN` の分岐を削除し、`#else` の側（現在実際に使われているコード）を残す。`#ifdef` と `#ifndef` を取り違えないように注意する。
  - 対象: `main_window.cpp`、`main_window.h`、`debug_message_window.cpp`、`draw_config.cpp`、`launcher_dialog.cpp`、`formation_editor_window.cpp`、`log_player_tool_bar.cpp`、`mark_assignment_editor.cpp`
- `USE_GLWIDGET` 関連のコードを削除する。`field_canvas.h:35-66`、`field_canvas.cpp`、`ball_painter.cpp`、`debug_painter.cpp` などにある `#ifdef USE_GLWIDGET` のブロックと、それをコメントアウトした部分が対象。
- `#ifdef USE_HIGH_QUALITY_ANTIALIASING` の分岐（`#else` 側の `QPainter::Antialiasing` を残す）も削除する（作業中に追加）。
- 判定が `#ifdef`/`#ifndef`/`#if (QT_VERSION >= QT_VERSION_CHECK(5, x, 0))` の単純な形だけだったので、分岐を機械的に除去するスクリプトで処理した。コメントアウトされたコード中のディレクティブ（`// #ifdef Q_WS_MAC` など）は変更しない。`QT_VERSION` 判定のために入れていた `#include <QtGlobal>` は残す。

#### 2-2. Qt6 でのコンパイルエラーの修正

- `QRegExpValidator( QRegExp( ... ) )` を `QRegularExpressionValidator( QRegularExpression( "\\d+(\\s\\d+)*" ) )` に置き換える。
- `horizontalAdvance( tr( "..." ) + 64 )` を `horizontalAdvance( tr( "..." ) ) + 64` に直す（既存バグの修正）。Qt5 では `+ 64` が文字 `@`（`+ 32` は空白）の連結と解釈されていたため、修正後はフォーメーションエディタのデータビューの列幅が以前より広くなる。
- `setMargin( n )` を `setContentsMargins( n, n, n, n )` に置き換える。
- キーの組み合わせは `|` で結合する（例: `Qt::CTRL | Qt::ALT | Qt::Key_F`）。`Qt::CTRL + Qt::Key_0 + i` は `Qt::CTRL | Qt::Key( Qt::Key_0 + i )` と書く。`+` による結合も、まとめて `|` に置き換える（`|` は Qt5 でも使える）。修飾キーとキーのビットは重ならないので、キーコードの値は変わらない。
  - 2-1 で `Q_WS_MAC` 側の分岐が消えたため、置き換えは51行になった。
  - 修飾キーのない `Qt::Key_0 + i`（int として `QKeySequence` に渡る）は Qt6 でも警告が出ないため変更しない。

#### 2-3. 実行時の不具合の修正

- `SIGNAL( mapped( int ) )` を `SIGNAL( mappedInt( int ) )` に置き換える（Qt 5.15 以降で使える。4箇所）。
- そのほかの `SIGNAL()` で使っているシグナル名は、Qt6 にも存在することを確認済み。実行時に `QObject::connect: No such signal/slot` が出ないことを検証で確かめる。

#### 2-4. 非推奨 API の置き換え（Qt6 で警告ゼロを目指す）

- `QMenu::addAction( text, receiver, member, shortcut )` は、`QAction * act = menu->addAction( text );` で作ったうえで `act->setShortcut( ... );` と `connect( ... )` を行う形にする。
  - 対象: `formation_editor_window.cpp:500`、`label_editor_window.cpp:370`、`mark_assignment_editor.cpp:255,261`、`simple_label_selector.cpp:158`
- `QMouseEvent::globalPos()` は、`QT_VERSION_CHECK(6, 0, 0)` で分岐し、Qt6 では `globalPosition().toPoint()`、Qt5 では `globalPos()` を使う。`field_canvas.cpp` では3箇所あるので、無名名前空間の補助関数 `global_pos()` にまとめる。
- `QDropEvent::pos()` も同様に、Qt6 では `position().toPoint()`、Qt5 では `pos()` を使う（`action_sequence_tree_view.cpp:137`、`formation_data_view.cpp:304`）。
- `QMessageBox::question` は `QMessageBox::StandardButtons` を受け取る版に置き換える。既定ボタンは No とし、廃止予定の別名 `QMessageBox::YesAll` は `QMessageBox::YesToAll` に置き換える。
- `QMouseEvent::pos()` など、Qt6 で警告が出ないものは変更しない。

### Phase 3: Autotools の Qt6 対応（サブのビルドシステムとして維持）

対象: `configure.ac`、`Makefile.am`、`src/Makefile.am`、`src/model/Makefile.am`、`src/qt/Makefile.am`、`m4/`

- `ax_have_qt.m4` をもとに、新しいマクロ `SW2_CHECK_QT`（`m4/sw2_check_qt.m4`）を作る（元のライセンス表記は残す）。
  - `--with-qt=auto|5|6` オプションを追加する。既定値の auto では `qmake6 qmake-qt6 qmake qmake-qt5` の順に探し、Qt6 を優先する。環境変数 `QMAKE=...` で直接指定することもできる。
  - 候補の qmake を順に調べ、要求したメジャーバージョンと最低バージョンを満たす最初のものを使う（例えば `--with-qt=5` のとき、`qmake` が Qt6 のものなら飛ばす）。`QMAKE` を指定した場合は、条件を満たさなければエラーにする。
  - `$QMAKE -query QT_VERSION` で、最低バージョン（5.15 / 6.2）を満たしているかと、指定したメジャーバージョンと一致しているかを確認する。
  - moc は、`$QMAKE -query QT_HOST_LIBEXECS`（Qt6）、`QT_HOST_BINS`（Qt5）、PATH の順に探す。環境変数 `QT_MOC` で直接指定することもできる。uic/rcc は使っていないので探さない。
  - 検出用の一時 .pro ファイルでは `QT += core gui widgets network` だけを指定し、不要なモジュールをリンクしない。
  - 既存の moc 動作確認テストはそのまま引き継ぐ。
  - `ax_have_qt.m4` が要求していた `AC_PATH_X`/`AC_PATH_XTRA`（X11 の検出）は不要なので外す。
- `configure.ac` では、`AM_CONDITIONAL(BUILD_QT, ...)` がシェルの `if` の中で呼ばれていた（Automake の作法に反する）ので、`SW2_CHECK_QT` の後で1回だけ呼ぶ形に直す。
- `AX_CXX_COMPILE_STDCXX_17(noext)` を Qt のチェックより前に移す（Qt6 のヘッダは C++17 が必須）。
- `AX_BOOST_SYSTEM` と `$BOOST_SYSTEM_LIB` を削除し、Boost の検出は `AX_BOOST_BASE` だけにする。最低バージョンは CMake と同じ 1.41 に揃える（以前は 1.38）。ヘッダだけを使うので `BOOST_LDFLAGS` も `LDFLAGS` に加えない。
- `m4/ax_have_qt.m4`、`m4/qt.m4`、`m4/ax_boost_system.m4` を削除する。
- `src/model/Makefile.am` に `csv_logger.cpp` と `csv_logger.h` を追加し、CMake とソース一覧を揃える。
- `src/qt/Makefile.am` の `soccerwindow2_CPPFLAGS` に `-I$(top_srcdir)` を追加する。`#include "xpm/....xpm"` は現在 `-I$(top_builddir)` で解決されているため、ソースとビルドのディレクトリが異なる VPATH ビルド（`make distcheck` を含む）では見つからない（Phase 1 の作業中に判明）。
- `EXTRA_DIST` から `.pro` ファイルを外し、CMake 関連ファイル（各 `CMakeLists.txt`、`cmake-config.h.in`、`cmake/FindLibrcsc.cmake`）と `RELEASE_NOTES.md` を加える。これで `make dist` で作った tarball からも CMake でビルドできるようになる。
- `make distcheck` で見つかった配布物の不足を補う（Phase 3 の作業中に判明）。
  - `src/model/Makefile.am` の `noinst_HEADERS` に、`agent_id.h`、`draw_data_handler.h`、`draw_types.h` が入っていなかった。
  - ルートの `EXTRA_DIST` に、ソースが参照している `xpm/data_auto_select.xpm`、`xpm/fedit2.xpm`、`xpm/sync-ball.xpm` が入っていなかった。あわせて、2回書かれていた `xpm/num_31.xpm` を1つにする。参照されていない xpm は従来どおり配布しない。
- `soccerwindow2.spec.in` の `Requires` / `BuildRequires` にある Qt と Boost の記述を、現在の要件に合わせて更新する。
  - `BuildRequires` は `(qt6-qtbase-devel >= 6.2 or qt5-qtbase-devel >= 5.15)` と `boost-devel >= 1.41` にする。
  - 実行時の `Requires` から Qt と Boost を外す。Qt のライブラリは rpmbuild が自動で依存関係に加え、Boost はヘッダしか使わないため。
- qmake 用のファイル `soccerwindow2.pro`、`src/src.pro`、`src/qt/qt.pro`、`src/model/model.pro`、`src/model/model.pro.win` を削除する（`src/qt/soccerwindow2.rc` と `.ico` は残す）。

### Phase 4: テストの整備

テストは `test/` に置き、CMake では CTest、Autotools では `make check` から実行できるようにする。テスト時は環境変数 `HOME` と `XDG_CONFIG_HOME` をビルドディレクトリ内の一時ディレクトリに向け、ユーザの `~/.soccerwindow2` を読み書きしないようにする。

#### 4-1. 起動テスト（CMake と Autotools で共通）

- シェルスクリプト `test/smoke_help.sh` を作る。引数で渡された soccerwindow2 の実行ファイルを `QT_QPA_PLATFORM=offscreen` で `--help` 付きで起動し、次の2点を確認する。この起動では MainWindow を生成するので、その時点のシグナル/スロット接続を検査できる。
  - 標準出力に `Usage:` が含まれること（`--help` の終了コードは 1 なので、終了コードでは判定しない）
  - 標準エラー出力に `QObject::connect` の警告が出ていないこと
- CMake からは `add_test(NAME soccerwindow2_help COMMAND sh test/smoke_help.sh $<TARGET_FILE:soccerwindow2>)` の形で登録する。
- Autotools では `Makefile.am` の `TESTS` にこのスクリプトを登録し、`make check` で実行する。スクリプトは `EXTRA_DIST` にも加える。

#### 4-2. Qt Test による GUI スモークテスト（CMake のみ）

- テスト用に GUI のコードをリンクできるよう、`src/qt` のソースのうち `main.cpp` 以外を静的ライブラリ `soccerwindow2_gui` にまとめる。実行ファイル `soccerwindow2` は `main.cpp` と `soccerwindow2_gui` から作る。
- テストのビルドは CTest 標準の `BUILD_TESTING`（既定値 ON）で切り替える。ON のときだけ Qt の `Test` コンポーネントを要求する。
- `test/test_gui_smoke.cpp`（`QTEST_MAIN`、offscreen で実行）で次を検査する。
  - メッセージハンドラで警告を捕まえ、MainWindow の生成時と各ウィンドウ・ダイアログの表示時に `QObject::connect` の警告が出ないこと。ウィンドウ・ダイアログは、MainWindow の QAction のうち表示用のもの（`show_formation_editor_window` など）を objectName で選んで `trigger()` して開く。モーダルになってテストが止まるもの（ファイル選択ダイアログなど）は対象から外す。対象は実装時にコードを確認して決める。
  - 主要なショートカットが期待どおりであること（例: `show_formation_editor_window` が Ctrl+Alt+F）。`+` から `|` への書き換えで意味が変わっていないことを確かめる。
  - MainWindow 内の QAction で、同じショートカットが重複して割り当てられていないこと。既存のコードに重複がある場合は、修正するか許容するかをユーザに確認し、計画書に記録する。
- `make dist` の tarball からも CMake でテストできるよう、`test/CMakeLists.txt` と `test/test_gui_smoke.cpp` も `EXTRA_DIST` に加える。
- 実行方法: `ctest --test-dir build --output-on-failure`

### Phase 5: CMake による配布用 tarball の作成

将来 Autotools を外すことを見据え、`make dist` / `make distcheck` に相当する手段を CMake に用意する。Autotools を残している間は、`make dist` / `make distcheck` もそのまま使える。

- 方式: `git archive` で HEAD のコミット済みファイルから tarball を作る。
  - CMake のカスタムターゲット `dist` と `distcheck` を追加する。処理は `cmake -P` で実行する CMake スクリプト（`cmake/` 以下）に書き、make に依存しないようにする。
  - `cmake --build build --target dist` で、ビルドディレクトリに `soccerwindow2-YYYYMMDD.tar.gz` を作る。展開すると `soccerwindow2-YYYYMMDD/` になる。
  - 日付は HEAD のコミット日（コミット時のタイムゾーン）とし、バージョン番号は付けない。同じコミットからは同じ名前の tarball ができる。
  - 作業ツリーに未コミットの変更があるときは、それが tarball に含まれないことを警告する。
  - ソースが git リポジトリでない場合（tarball を展開したソースなど）は、エラーで止める。
- 除外するもの（`.gitattributes` の `export-ignore` で指定する）
  - 計画書: `docs/plan/`
  - ビルド対象外の古いソース: `src/qt/obsolete/`
  - 開発用の設定ファイル: `.gitignore`、`.gitattributes`、`.clang-format`、`.github/`
  - ソースからもビルド設定からも参照されていないファイル: `xpm/canvas_enlarge.xpm`、`xpm/canvas_player_number.xpm`、`xpm/canvas_player_type.xpm`、`xpm/copy.xpm`、`xpm/cut.xpm`、`xpm/debug_view_player.xpm`、`xpm/float.xpm`、`xpm/hand.xpm`、`xpm/help.xpm`、`xpm/left.xpm`、`xpm/logplayer_accelerate.xpm`、`xpm/logplayer_decelerate.xpm`、`xpm/logplayer_slider.xpm`、`xpm/monitor_connect.xpm`、`xpm/monitor_discard_player.xpm`、`xpm/monitor_disconnect.xpm`、`xpm/monitor_kickoff.xpm`、`xpm/monitor_move_player.xpm`、`xpm/paste.xpm`、`xpm/preview.xpm`、`xpm/print.xpm`、`xpm/right.xpm`、`xpm/soccerwindow2.png`、`src/soccerwindow2.in`、`src/model/xpm-memo.txt`
  - xpm を新しく使い始めるときは、`.gitattributes` から外す必要がある。外し忘れても `distcheck` のビルドで検出できる。
- `make dist` の tarball との違い
  - `configure` などの Autotools の生成物は含まれない。Autotools を残している間、tarball から Autotools でビルドするには `./bootstrap` を実行する必要がある（README に記載する）。
  - `make dist` に含まれていた未使用の xpm（13個）は含まれず、`LICENSE`、`README.md`、`reconfig.sh` などは新たに含まれる。
- `distcheck`
  - tarball を一時ディレクトリに展開し、ソースを読み取り専用にしたうえで、ソース外での configure・ビルド・テスト（ctest）・一時ディレクトリへのインストールを行う。
  - 外側のビルドの `SOCCERWINDOW2_QT_VERSION` と `LIBRCSC_INSTALL_DIR` を、内側の configure にそのまま引き継ぐ。
  - 成功したら一時ディレクトリを削除する。

### Phase 6: GitHub Actions による CI

- `.github/workflows/build.yml` を新しく作る。master と develop への push、および pull_request で実行する。
- 次の組み合わせで計8ジョブを実行する。
  - OS: `ubuntu-22.04`（Qt 5.15.3 / 6.2.4）、`ubuntu-24.04`（Qt 5.15.13 / 6.4.2）
  - Qt: `5`、`6`
  - ビルドシステム: `cmake`、`autotools`
- 各ジョブの手順
  1. apt で依存パッケージを入れる。
     - Qt5: `qtbase5-dev qt5-qmake`
     - Qt6: `qt6-base-dev qt6-base-dev-tools qmake6 libgl-dev`
     - 共通: `build-essential cmake autoconf automake libtool libboost-dev zlib1g-dev`
  2. `helios-base/librcsc` をチェックアウトし、ビルドして `$HOME/local` にインストールする。
  3. soccerwindow2 をビルドする。
  4. テストを実行する。CMake のジョブは `ctest --test-dir build --output-on-failure`、Autotools のジョブは `make check` を使う。
  5. CMake のジョブでは、`cmake --build build --target distcheck` も実行する。

### Phase 7: ドキュメントの更新

- `README.md` を次のように更新する。
  - 依存要件: Qt 5.15 以上または Qt 6.2 以上、CMake 3.16 以上、Boost のヘッダ
  - Ubuntu 22.04/24.04 向けの apt コマンドを Qt6 用と Qt5 用に分けて記載
  - ビルド手順は CMake を先に書く（`cmake -S . -B build && cmake --build build && cmake --install build`、`-DSOCCERWINDOW2_QT_VERSION=5`、`-DLIBRCSC_INSTALL_DIR=...`）
  - Autotools の手順に `--with-qt` オプションを追記
  - Ubuntu 16.04〜20.04（`qt5-default`、Qt 5.12）の記述を削除
  - `sswindow2` は CMake でもインストールされる、という記述に直す
  - テストの実行方法（`ctest --test-dir build --output-on-failure`、`make check`）を追記
  - Qt6 版で保存したフォント設定は Qt5 版で読めない旨の注意を追記
  - 配布用 tarball の作り方（`dist` / `distcheck` ターゲット）と、tarball から Autotools でビルドするときは `./bootstrap` が必要なことを追記
- `RELEASE_NOTES.md` と本計画書の「実装記録」を更新する。

## 5. 検証方法

ローカル環境（Ubuntu 24.04。Qt 6.4.2 と Qt 5.15.13 が共存し、librcsc は `~/local` にある）で次を実施する。

1. **CMake でのビルド**（Qt6 と Qt5 の両方）
   - `cmake -S . -B build-qt6 && cmake --build build-qt6 -j` を実行する。設定時に Qt6 が選ばれ、`ldd build-qt6/soccerwindow2 | grep Qt6` で Qt6 にリンクされていることを確かめる。
   - `cmake -S . -B build-qt5 -DSOCCERWINDOW2_QT_VERSION=5 && cmake --build build-qt5 -j` を実行し、Qt5 にリンクされることを確かめる。
   - どちらのビルドでも、エラーと Qt の非推奨警告が0件であること。追加の確認として、`-DCMAKE_CXX_FLAGS=-DQT_DISABLE_DEPRECATED_BEFORE=0x050F00` を付けてもビルドが通ること。
   - 一時ディレクトリへ `cmake --install build-qt6 --prefix <一時ディレクトリ>` でインストールし、`bin/soccerwindow2`、`bin/sswindow2`、`share/soccerwindow2/soccerwindow2.xpm` が置かれること。
2. **Autotools でのビルド**（Qt6 と Qt5 の両方。切り替えの間に `make distclean` を実行する）
   - `./bootstrap && ./configure --with-qt=6 && make -j` と `./configure --with-qt=5 && make -j` がどちらも通ること。
   - `make distcheck DISTCHECK_CONFIGURE_FLAGS="--with-librcsc=$HOME/local"` が通り、tarball に CMake 関連ファイルが含まれていること。
3. **自動テスト**
   - `ctest --test-dir build-qt6 --output-on-failure` と `ctest --test-dir build-qt5 --output-on-failure` がすべて成功すること。
   - Autotools のビルドで `make check` が成功すること（Qt6/Qt5 とも）。
   - テストを実行しても `~/.soccerwindow2` が変更されないこと。
4. **GUI の手動テスト**（Qt6 のビルドを中心に行い、主要な項目は Qt5 のビルドでも確認する）
   - rcg ログの再生、ステップ送り、マウスドラッグ、ホイールでのズーム（`globalPosition` に変えた箇所）
   - ショートカット: Ctrl+Alt+F/L/S/M での各エディタの起動、Ctrl+数字キーでの選手選択（`|` 結合に変えた箇所）
   - QSignalMapper を変えた箇所: プレイモード変更メニュー、トレーナダイアログ、デバッグメッセージウィンドウでのデバッグレベル切り替え
   - フォーメーションエディタ: データビューでのドラッグ&ドロップ、右クリックメニューとショートカット、レイアウトの余白
   - アクションシーケンスセレクタ: ID フィルタが `1 2 3` を受け付け、`a` を受け付けないこと
   - 画像保存ダイアログでの上書き確認、色設定（アルファ値）、`~/.soccerwindow2` の読み書き
   - rcssserver が使える場合はモニタとして接続する（`-c`）
   - HiDPI の確認として、`QT_SCALE_FACTOR=2` で描画が崩れないこと
   - 既知の注意点: Qt6 で保存したフォント設定の文字列は Qt5 で読めない場合がある。そのため、Qt5 と Qt6 のビルドを切り替えると設定がリセットされることがある。確認した結果は「実装記録」に書く。
5. **配布用 tarball**
   - `cmake --build build-qt6 --target dist` で `soccerwindow2-YYYYMMDD.tar.gz` ができ、除外対象と未追跡のファイル（`old-tests/`、`CLAUDE.md` など）が含まれないこと。
   - `cmake --build build-qt6 --target distcheck` と `cmake --build build-qt5 --target distcheck` が成功すること。
   - 未コミットの変更があるときに警告が出ること。
6. **CI**: PR 上で8つのジョブ（ビルドとテスト）がすべて成功すること。

## 6. 実装記録

（各フェーズの完了時に、実施内容・計画からの変更点・検証結果を追記する）

### Phase 0（2026-11-19）

- 本計画書を作成した。計画の確認中に、テスト方針（4-1・4-2）、`make check` の範囲、`CLAUDE.md` の扱いを決定事項に加えた。テストの置き場所は `test/` に決めた。
- `RELEASE_NOTES.md` を新しく作り、`Unreleased` 節を設けた。
- `.gitignore` に `CLAUDE.md` を追加した。
- 作業ブランチ `feature/qt6-cmake` はユーザが GitHub 上で作成した。

### Phase 1（2026-11-19）

- `CMakeLists.txt`、`src/CMakeLists.txt`、`src/model/CMakeLists.txt`、`src/qt/CMakeLists.txt`、`cmake/FindLibrcsc.cmake` を書き換えた。`cmake-config.h.in` は変更不要だった。
- 計画からの変更点
  - Qt の AUTO 選択を、`find_package(QT NAMES Qt6 Qt5)` から Qt6 を先に明示的に探す方式に変えた（理由は Phase 1 の節に記載）。
  - librcsc の探索ヒントを `HINTS` にし、指定したディレクトリをシステムの既定パスより優先するようにした。
  - `sswindow2` のインストール元をビルドディレクトリの生成ファイルに直した。
  - Boost の最低バージョンを 1.41 にした。
  - Autotools の VPATH ビルドで xpm が見つからない問題が分かったので、Phase 3 に対応を追加した。
- 検証結果（Ubuntu 24.04、CMake 3.28.3）
  - `-DSOCCERWINDOW2_QT_VERSION=5`: Qt 5.15.13 でビルドでき、警告とエラーは0件だった。
  - 既定（AUTO）: Qt 6.4.2 が選ばれた。コンパイルエラーは、調査で見つけた箇所（Phase 2 で修正予定）だけだった。
  - `-DSOCCERWINDOW2_QT_VERSION=6`: Qt 6.4.2 が選ばれた。`4` などの不正な値はエラーで止まる。
  - 一時ディレクトリへの `cmake --install` で `bin/soccerwindow2`、`bin/sswindow2`、`share/soccerwindow2/soccerwindow2.xpm` が配置された。インストールした実行ファイルには RUNPATH `/home/akiyama/local/lib` が設定され、`LD_LIBRARY_PATH` なしで librcsc を解決できた。
  - インストールした Qt5 版を `QT_QPA_PLATFORM=offscreen` で `--help` 付きで起動すると、`Usage:` が出力され、`QObject::connect` の警告は出なかった。

### Phase 2（2026-11-19）

- ユーザと相談し、作業の種類ごとに5つのコミットに分けた。各段階で Qt5 のビルドが通ることを確認した。
  - A: Qt4 向けのコードと使われていないコードの除去（47ファイル）
  - B: キーの組み合わせを `|` で結合
  - C: 残りの Qt6 コンパイルエラーの修正（この段階から Qt6 でビルドできる）
  - D: `mapped(int)` を `mappedInt(int)` に修正
  - E: 非推奨 API の置き換えと、計画書・リリースノートの更新
- 計画からの変更点
  - `USE_HIGH_QUALITY_ANTIALIASING` の分岐の削除を追加した（`HighQualityAntialiasing` は `USE_GLWIDGET` のブロック内ではなく、この分岐内にあった）。
  - 修飾キーのない `Qt::Key_0 + i` は変更しなかった。
  - `field_canvas.cpp` の `globalPos()` は補助関数にまとめた。
- 検証結果（Ubuntu 24.04、Qt 5.15.13 / 6.4.2）
  - Qt5・Qt6 ともクリーンビルドで警告とエラーが0件だった。`-DQT_DISABLE_DEPRECATED_BEFORE=0x050F00` を付けたビルドも、両方で警告とエラーが0件だった。
  - C の段階の Qt6 版を `--help` で起動すると `QObject::connect: No such signal QSignalMapper::mapped( int )` が出た。計画している起動テストでこの種の不具合を検出できることを確認した。D の修正後は、Qt5・Qt6 とも警告が出なくなった。
  - 既知の注意点を確認した。Qt6 版は `~/.soccerwindow2` にフォントを16項目の形式（例: `Sans Serif,10,-1,5,400,0,0,0,0,0,0,0,0,0,0,1`）で保存し、Qt5 版はこれを読めない（`QFont::fromString: Invalid description` が出て既定のフォントに戻る）。Qt5 版が保存する10項目の形式は Qt6 版でも読める。Qt6 版から Qt5 版に戻すとフォント設定が失われる点は、README とリリースノートに記載する。
- GUI の手動テストは、Phase 4 のテストを整備した後にまとめて行う。

### Phase 3（2026-11-19）

- 4つの段階に分け、段階ごとにユーザが差分を確認してからコミットした。プッシュは Phase 3 の完了時にまとめて行う。
  1. Boost.System への依存の除去（`ccbc836`）
  2. qmake の `.pro` ファイルの削除（`d536a82`）
  3. Qt 検出マクロ `SW2_CHECK_QT` の作成（`be84a98`）
  4. ビルドと配布物の修正、計画書の実装記録
- 計画からの変更点
  - Boost の最低バージョンを CMake と同じ 1.41 にした。
  - Qt 検出マクロは、要求したメジャーバージョンと最低バージョンを満たす qmake を候補から選ぶ方式にした。`QT_MOC` で moc を直接指定できるようにした。
  - `make distcheck` で、`Makefile.am` に書かれていないヘッダ3つと xpm 3つが見つかったので追加した。
- 検証結果（Ubuntu 24.04、作業ツリーのコピーで実施）
  - `./configure` は既定で Qt 6.4.2（`/usr/bin/qmake6`、moc は `/usr/lib/qt6/libexec/moc`）を選び、`--with-qt=5` で Qt 5.15.13（`/usr/bin/qmake`、moc は `/usr/lib/qt5/bin/moc`）を選んだ。`--with-qt=4` と、`--with-qt=6` に Qt5 の `QMAKE` を指定した場合はエラーになった。
  - ソース外ビルド（VPATH ビルド）が Qt6・Qt5 とも通り、コンパイラの警告は0件だった。リンクされる Qt ライブラリは Core/Gui/Widgets/Network だけになった（以前はインストール済みの全モジュールがリンクされていた）。
  - `make distcheck` が Qt6 と Qt5（`DISTCHECK_CONFIGURE_FLAGS=--with-qt=5`）の両方で通った。
  - `make dist` で作った tarball を展開し、CMake で Qt6・Qt5 の両方をビルドできた。
  - `--help` での起動テストが Qt6・Qt5 とも通った（`Usage:` が出力され、標準エラー出力は空）。
  - rpm のツールがない環境のため、`soccerwindow2.spec` を使った rpmbuild は確認していない。

### 計画の変更（2026-11-19、Phase 3 完了後）

- ユーザから、将来 Autotools を外す可能性が高いので `make dist` に相当する手段を用意したいとの相談があり、次のとおり決めた。
  - 方式: `git archive` を使う CMake ターゲット `dist` / `distcheck`。CPack の `package_source` は、未追跡のファイルや生成物が混入しやすく除外パターンの保守が必要なため採らなかった。GitHub がタグごとに生成する tarball は、タグの運用が必要で検査の手段もないため採らなかった。
  - 除外: 計画書、`src/qt/obsolete/`、開発用の設定ファイル、参照されていないファイル。
  - ファイル名: バージョン番号を外し、日付だけを付ける（`soccerwindow2-YYYYMMDD.tar.gz`）。
  - テストの後の Phase 5 として追加し、CI を Phase 6、ドキュメントの更新を Phase 7 に繰り下げた。
