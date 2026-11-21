# GNOME のデスクトップからの起動（デスクトップエントリ）計画書

- 作成日: 2026-11-21
- ステータス: 段階5 を実施済み（PR の作成待ち）
- ブランチ: `feature/desktop-entry`（2026-11-21 にユーザが GitHub 上で develop から作成。PR #95 のマージ後）
- 前の計画: `docs/plan/2026-11-21_icon-redesign.md`（アプリのアイコンを freedesktop.org の hicolor テーマの場所にインストールするようにした）

## 1. 背景と目的

前の計画で、アプリのアイコン（soccerwindow2）を `PREFIX/share/icons/hicolor/` にインストールするようにした。
この計画では、GNOME のデスクトップ（アクティビティの検索やアプリの一覧）から soccerwindow2 を検索・起動できるようにする。
あわせて、Autotools のビルドでも librcsc のパスを実行ファイルに埋め込み（RPATH）、`LD_LIBRARY_PATH` なしで起動できるようにする。

GNOME は、`~/.local/share` と `XDG_DATA_DIRS` のディレクトリ（`/usr/local/share`、`/usr/share` など）の下の `applications/*.desktop`（freedesktop.org の Desktop Entry）を一覧に出す。
`.desktop` ファイルの `Icon=soccerwindow2` は、同じディレクトリの下の `icons/hicolor/` から名前で探される。

## 2. 現状（2026-11-21、develop の PR #95 マージ後）

- `.desktop` ファイルはない。
- インストール先の既定値
  - CMake: `~/local`（トップの `CMakeLists.txt` で、`CMAKE_INSTALL_PREFIX` が既定値のときに `$HOME/local` に変える）。`~/local/share` は GNOME の探索先に入っていないので、`.desktop` ファイルとアイコンを置いても見つからない。
  - Autotools: `/usr/local`（`configure` の既定値）。`/usr/local/share` は探索先に入っている。
- librcsc の探索先は、CMake が `~/.local;~/local`、Autotools が `~/local`、`~/.local`、`/opt/local`、`/usr/local`、`/usr`。librcsc の置き場所によらず、soccerwindow2 のインストール先は変えられる。
- librcsc の実行時のパス
  - CMake のビルドは librcsc のパスを実行ファイルに埋め込む（`CMAKE_INSTALL_RPATH_USE_LINK_PATH`）ので、`LD_LIBRARY_PATH` は不要。Ubuntu のリンカは RPATH ではなく RUNPATH として書く（`readelf -d build-qt6/soccerwindow2` で `RUNPATH: /home/akiyama/local/lib` を確かめた）。
  - Autotools のビルドは埋め込まない。`configure.ac` は librcsc の lib ディレクトリ（`--with-librcsc` の指定か、`~/local` などを探して見つけたもの）を `LIBRCSCLIB` に入れ、`LDFLAGS` に `-L` を加えるだけ。librcsc が標準の場所にないときは `LD_LIBRARY_PATH` が必要で、デスクトップから起動するとシェルの設定（`~/.bashrc` など）が読まれないため、librcsc が見つからずに起動に失敗する。今は `docs/build.md` に1行の注意があるだけで、README には書いていない。
  - `make check` は、`AM_TESTS_ENVIRONMENT` で `LD_LIBRARY_PATH` に `LIBRCSCLIB` を加えて実行している。
  - librcsc は `librcsc.so.20` の1つで、依存しているのはシステムのライブラリ（`libz`、`libm` など）だけ。RUNPATH は実行ファイルが直接使うライブラリにしか効かないが、この構成ならそれで足りる。
- soccerwindow2 は、アプリ名やデスクトップファイル名を設定していない（`QGuiApplication::setDesktopFileName()` などを呼んでいない）。X11 の `WM_CLASS` は、実行ファイル名から `soccerwindow2` になる。
- `sswindow2` は `soccerwindow2 -c -d` を実行するスクリプト（`-c`: サーバに接続してモニタとして起動、`-d`: デバッグサーバを起動）。
- `desktop-file-validate`（desktop-file-utils）は手元にある。CI（GitHub Actions）ではインストールしていない。

## 3. 決定事項

ユーザと相談して決めた（2026-11-21）。

1. **既定のインストール先:** CMake の既定のインストール先を `~/local` から `~/.local` に変える。`~/.local/share` は GNOME が常に探し、`~/.local/bin` は Ubuntu では `PATH` に入る。
2. **起動時の右クリックメニュー:** 「サーバに接続」を加える（`.desktop` ファイルの `[Desktop Action]`）。
3. **ファイルの関連付け:** `.rcg` をファイルマネージャから開けるようにはしない（MIME 型の定義や `MimeType` は加えない）。
4. **Autotools のビルドの RPATH:** Autotools のビルドでも、librcsc の lib ディレクトリを実行ファイルに埋め込む（RPATH。`--disable-rpath` で外せる）。これで、どちらのビルドでも `LD_LIBRARY_PATH` なしで起動でき、GNOME からも起動できる。`LD_LIBRARY_PATH` の設定は README などに書くが、必要になるのは `--disable-rpath` でビルドしたときと、librcsc の置き場所を変えて soccerwindow2 をビルドし直していないときとする。
   - 最初は「RPATH には対応せず、GNOME から使うなら CMake でビルドすると文書に書く」と決めていたが、ユーザの質問（RPATH を埋め込めば `LD_LIBRARY_PATH` は不要になるか）への回答を受けて、ユーザが変えた（2026-11-21）。
5. **「サーバに接続」で実行するもの:** `sswindow2` と同じ `soccerwindow2 -c -d`（サーバに接続してモニタとして起動し、デバッグサーバも起動する）。
6. **`.desktop` ファイルの日本語訳:** 入れない。
7. **Autotools の既定のインストール先:** 変えない（`/usr/local` のまま）。
8. **librcsc のインストール先の書き方:** README などの librcsc のインストール先は、そのうち `~/.local` に変える予定なので、`~/local` と `~/.local` のどちらにも対応できるように書く（例: `LD_LIBRARY_PATH` の設定の例を両方で示す）。
9. **`reconfig.sh`（`./configure --prefix=$HOME/local`）:** インストール先を `$HOME/.local` に変える（CMake の既定値にそろえる）。
   - 最初は案のとおり変えないことにしていたが（ユーザから返答がなかった）、段階2 の後にユーザが変えるよう指示した（2026-11-21）。

## 4. 未決事項

| 項目 | 案 |
|---|---|
| （なし） | |

## 5. 方針

### 5.1 既定のインストール先

- トップの `CMakeLists.txt` で、`CMAKE_INSTALL_PREFIX` が既定値のときに `$HOME/.local` にする。
- 既存のビルドディレクトリには、前の既定値（`~/local`）がキャッシュに残る（キャッシュの値は変えない）。新しい既定値を使うには、ビルドディレクトリを作り直す（`./cmake-helper.sh clean` の後に `build`）か、`-DCMAKE_INSTALL_PREFIX=$HOME/.local` を指定する。`~/local` にインストールしたものは、前のビルドディレクトリで `uninstall` してから移る。これを文書とリリースノートに書く。
- `cmake-helper.sh` の使い方の表示、`README.md`、`docs/build.md` の既定値の記述を直す。

### 5.2 Autotools のビルドの RPATH

- `configure.ac` で、librcsc を見つけた後に、その lib ディレクトリ（`LIBRCSCLIB`）を埋め込むリンクのオプション `-Wl,-rpath,DIR` を作り、`AC_SUBST` の変数（例: `LIBRCSC_RPATH`）にして、`src/qt/Makefile.am` の `soccerwindow2_LDFLAGS` に加える。libtool は使わない。
- 埋め込まないとき
  - `--disable-rpath` を指定したとき（`AC_ARG_ENABLE`。既定は埋め込む）。ディストリビューションのパッケージなどで RPATH を避けたい場合のため。
  - librcsc のディレクトリが、動的リンカが必ず探す標準の場所（`/usr/lib`、`/usr/lib64`、`/lib`、`/lib64`）のとき。`/usr/local/lib` は、librcsc のインストールの後に `ldconfig` を実行していないと見つからないことがあるので、埋め込む。
- `configure` の確認の表示に、埋め込むディレクトリ（または埋め込まない理由）を出す（`checking for the library path of librcsc to embed (RPATH)... DIR`）。
- `Makefile.am` の `AM_TESTS_ENVIRONMENT` は、RPATH を埋め込むときは `LD_LIBRARY_PATH` を設定しない（`AM_CONDITIONAL`）。これで、`make check` と `make distcheck` のスモークテストが、`LD_LIBRARY_PATH` なしで起動できることを確かめる。`--disable-rpath` のときは今までどおり設定する。
- CMake のビルドで RPATH を外したいときは、CMake の標準の変数（`-DCMAKE_SKIP_INSTALL_RPATH=ON` など）が使える。これを `docs/build.md` に書く（CMake のビルドの設定は変えない）。

### 5.3 `.desktop` ファイル

- テンプレート `soccerwindow2.desktop.in` をトップに置き、インストールのときに `Exec` の実行ファイルのパスを埋め込んで、`PREFIX/share/applications/soccerwindow2.desktop` に置く。
- 内容（案）:
  ```ini
  [Desktop Entry]
  Type=Application
  Name=soccerwindow2
  GenericName=RoboCup Soccer Simulator Viewer
  Comment=Monitor, log player, and visual debugger for the RoboCup Soccer Simulator
  Exec="@SOCCERWINDOW2_BINDIR@/soccerwindow2"
  Icon=soccerwindow2
  Terminal=false
  Categories=Science;ArtificialIntelligence;
  Keywords=RoboCup;soccer;simulator;monitor;logplayer;
  StartupWMClass=soccerwindow2
  Actions=connect;

  [Desktop Action connect]
  Name=Connect to the Server
  Exec="@SOCCERWINDOW2_BINDIR@/soccerwindow2" -c -d
  ```
- 「サーバに接続」（`Actions=connect`）は `sswindow2` と同じく `-c -d` で起動する（「3.」の5）。日本語訳は入れない（「3.」の6）。
- `Exec` は実行ファイルの絶対パスにし、引用符で囲む（パスに空白があっても壊れないように。段階3 で決めた。`desktop-file-validate` は空白を含むパスでも通る）。置き換える目印は、Autotools の変数（`@bindir@` など）と紛れないよう `@SOCCERWINDOW2_BINDIR@` にする。デスクトップのセッションの `PATH` に `PREFIX/bin` が入っているとは限らないため（Ubuntu の `~/.profile` は、ログインのときに `~/.local/bin` があれば `PATH` に加えるので、初めてインストールした直後はログインし直すまで入らない）。
- `%f`（ファイルを受け取る）は付けない（「3.」の3）。
- 埋め込むパスは、インストールのときのインストール先にする。`cmake --install build --prefix DIR`（`./cmake-helper.sh install --prefix DIR`）のように、インストールのときにインストール先を変えられるため。
  - CMake: `install(CODE ...)` で、インストールのときの `CMAKE_INSTALL_PREFIX` を使ってテンプレートから `.desktop` ファイルを作り（スクリプトでも使える `file(READ)`、`string(CONFIGURE)`、`file(WRITE)`）、続く `install(FILES ...)` で置く（`install_manifest.txt` に載るので `uninstall` で消える）。`DESTDIR` はパスに含めない。
  - Autotools: `Makefile.am` の規則で、`$(bindir)` を `sed` で埋め込んで作り、`applications_DATA` で置く。テンプレートを `EXTRA_DIST` に、作ったファイルを `CLEANFILES` に加える。
- `MimeType` を書かないので、`update-desktop-database` は不要。

### 5.4 起動したウィンドウとの結び付け

- `main()` で `QGuiApplication::setDesktopFileName( "soccerwindow2" )` を呼ぶ。Wayland では、これが `app_id` になり、GNOME が起動したウィンドウをこの `.desktop` ファイルと結び付ける（Dash に正しいアイコンが出て、別のアプリとして並ばない）。
- X11 では、`WM_CLASS`（`soccerwindow2`）と `StartupWMClass=soccerwindow2` で結び付ける。`sswindow2` から起動しても同じ実行ファイルなので同じになる。

### 5.5 文書

- `README.md`: 既定のインストール先（`~/.local`）、GNOME のアクティビティから起動できること、librcsc のパスは実行ファイルに埋め込まれるので通常は `LD_LIBRARY_PATH` が不要なこと、`LD_LIBRARY_PATH` が必要になる場合（`--disable-rpath` でビルドしたとき、librcsc の置き場所を変えて soccerwindow2 をビルドし直していないとき）とその設定方法。
  - librcsc のインストール先は `~/local` と `~/.local` のどちらにも対応できるように書く（「3.」の8）。例えば、librcsc のインストールの説明は「`~/.local` または `~/local`」とし、`LD_LIBRARY_PATH` の設定の例は librcsc のインストール先に合わせて `export LD_LIBRARY_PATH=$HOME/.local/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}`（`~/local` なら `$HOME/local/lib`）のように示す。
- `docs/build.md`: 既定のインストール先、インストールされるファイルに `.desktop` ファイルを加える、デスクトップからの起動（探索先、ログインし直しが要る場合、`~/local` から移る手順）、librcsc のパスの埋め込み（CMake と Autotools、`--disable-rpath`、`CMAKE_SKIP_INSTALL_RPATH`、librcsc を移したらビルドし直すこと）と `LD_LIBRARY_PATH`。Autotools でも GNOME から使えること（既定のインストール先 `/usr/local` の `share/applications` も GNOME の探索先に入っている。`--prefix=$HOME/.local` も使える）。
- `RELEASE_NOTES.md`: 既定のインストール先の変更と移り方、Autotools のビルドの RPATH（`--disable-rpath`）、`.desktop` ファイル。

## 6. 実装計画

各段階で、ユーザに確認を受けてからコミットする。

### 段階1: 既定のインストール先の変更

- トップの `CMakeLists.txt` の既定値を `$HOME/.local` にする。
- `cmake-helper.sh` の使い方の表示を直す。
- 一時的なビルドディレクトリで、既定値が `~/.local` になること、`--prefix` を指定したときと既存のキャッシュがあるときは変わらないことを確かめる。

### 段階2: Autotools のビルドの RPATH

- `configure.ac`、`src/qt/Makefile.am`、トップの `Makefile.am` を変える（「5.2」）。
- `readelf -d` で RUNPATH が埋め込まれること、`--disable-rpath` では埋め込まれないこと、`make check` と `make distcheck` が `LD_LIBRARY_PATH` なしで通ることを確かめる。

### 段階3: `.desktop` ファイル

- `soccerwindow2.desktop.in` を加え、CMake と Autotools でインストールする（「5.3」）。
- `main.cpp` で `QGuiApplication::setDesktopFileName( "soccerwindow2" )` を呼ぶ（「5.4」）。
- テストを加える（「7. テスト」）。CI に desktop-file-utils を加える。

### 段階4: 文書と実機での確認

- `README.md`、`docs/build.md`、`RELEASE_NOTES.md` を更新する（「5.5」）。
- ユーザが実機（GNOME）で確かめる: アクティビティで「soccerwindow2」や「RoboCup」で検索して出ること、アイコン、起動、右クリックの「サーバに接続」、起動中のウィンドウが Dash で同じアイコンにまとまること。

### 段階5: 仕上げ

- PR のタイトルと説明の案を作る。

## 7. テスト

- `test/check_desktop_entry.sh`（新規。CTest と `make check`）
  - テンプレートから仮のパスで `.desktop` ファイルを作り、`desktop-file-validate` で検査する。`desktop-file-validate` がなければ飛ばす（CTest は `SKIP_RETURN_CODE`、Automake は終了コード 77）。
  - `Icon` が `icons/app/svg/` にあるアプリのアイコンの名前であること、`Exec` が `soccerwindow2` を指すこと。
- CMake の `distcheck`（`cmake/DistCheck.cmake`）: `share/applications/soccerwindow2.desktop` がインストールされ、その `Exec` がインストール先の `bin/soccerwindow2` を指すこと。アンインストールで消えること。
- Autotools の RPATH
  - `make check` と `make distcheck` のスモークテスト（`test/smoke_help.sh`）を、RPATH を埋め込むときは `LD_LIBRARY_PATH` なしで実行する（「5.2」）。librcsc が見つからなければ起動に失敗するので、埋め込みを確かめられる。
  - `--disable-rpath` のときは `LD_LIBRARY_PATH` を設定して実行する（今までどおり）。
- CMake の `distcheck`: インストールした実行ファイルを `LD_LIBRARY_PATH` なしで `smoke_help.sh` で起動し、インストールのときの RPATH を確かめる（今は、ビルドディレクトリの実行ファイルでしか起動を確かめていない）。
- Autotools の `make distcheck`: インストールとアンインストールでファイルが残らないこと（今までどおり）。
- CI: desktop-file-utils をインストールし、`check_desktop_entry.sh` を飛ばさずに実行する。

## 8. 検証方法

1. `./cmake-helper.sh test qt5`、`./cmake-helper.sh test qt6`、Autotools の `make check` と `make distcheck`、CMake の `distcheck` が通ること。
2. 一時的なインストール先への `cmake --install`（`--prefix` の指定あり・なし）で、`.desktop` ファイルの `Exec` が実際のインストール先を指すこと。
3. Autotools のビルドで、`readelf -d` に librcsc の lib ディレクトリの RUNPATH があること（`--disable-rpath` ではないこと）。
4. ユーザが実機の GNOME で確かめること（「段階4」）。
5. CI の8ジョブが成功すること。

## 9. 実装記録

（各段階の完了時に、実施内容・計画からの変更点・検証結果を追記する）

### 計画の作成（2026-11-21）

- 前の計画（PR #95）のマージの後に、ユーザから GNOME のデスクトップからアプリを検索できるようにしたいと相談があった。
- 仕組み（`.desktop` ファイル、探索先、`setDesktopFileName()`）と、既定のインストール先 `~/local` では GNOME から見つからないことを説明し、ユーザが決定事項（「3.」）を決めた。
- ユーザが `feature/desktop-entry` ブランチを作成した。
- 未決事項について、ユーザが次のとおり決めた（「3.」の5〜9）: 「サーバに接続」は `sswindow2` と同じ `-c -d`、日本語訳は入れない、Autotools の既定のインストール先は変えない、librcsc のインストール先は `~/local` と `~/.local` のどちらにも対応できるように書く（そのうち `~/.local` に変える予定）。`reconfig.sh` は案のとおり変えない。
- `.desktop` ファイルの案は、`desktop-file-validate` を通ることを確かめた。
- ユーザの質問（RPATH を埋め込めば `LD_LIBRARY_PATH` は不要になるか）に答えた: そのとおりで、CMake のビルドにはすでに RUNPATH が入っていること、librcsc は依存がシステムのライブラリだけなので RUNPATH で足りること、埋め込むのは絶対パスなので librcsc を移したらビルドし直すこと、Autotools でも `configure` が知っている librcsc のディレクトリを使って簡単に埋め込めること。これを受けてユーザが決定事項の4を変え、Autotools のビルドでも RPATH を埋め込むことにした（`--disable-rpath` で外せる）。段階2 として加え、`.desktop` ファイルを段階3、文書を段階4、仕上げを段階5 に繰り下げた。

### 段階1: 既定のインストール先の変更（2026-11-21）

- 実施内容
  - トップの `CMakeLists.txt`: `CMAKE_INSTALL_PREFIX` が既定値のときの値を `$HOME/local` から `$HOME/.local` に変えた。理由と、前に設定したビルドディレクトリではキャッシュの値が残ることをコメントに書いた。
  - `cmake-helper.sh`: 使い方の表示の既定のインストール先を `$HOME/.local` に直した（同じ表示の `LIBRCSC_INSTALL_DIR` の例 `$HOME/local` は librcsc の場所の例なので、そのままにした）。
  - 文書（`README.md`、`docs/build.md`）とリリースノートの既定値の記述は、段階4 でまとめて直す。
- 検証（一時的なホームディレクトリで実行し、実際の `~/.local` には書き込んでいない）
  - 新しく設定したとき: `CMAKE_INSTALL_PREFIX` が一時的なホームの `~/.local` になった。
  - `-DCMAKE_INSTALL_PREFIX=DIR` を指定したとき: その値になった。
  - 前の既定値（`~/local`）がキャッシュにあるビルドディレクトリで設定し直したとき: `~/local` のままだった。
  - 既定のままインストールしたとき: `~/.local/bin` に `soccerwindow2` と `sswindow2`、`~/.local/share/icons/hicolor/` にアイコンが置かれ、実行ファイルには librcsc の RUNPATH が入っていた。
  - `./cmake-helper.sh test qt6`: 3つのテストがすべて成功。
  - 手元の既存のビルドディレクトリ（`build`、`build-qt5`、`build-qt6`）は、キャッシュの値 `~/local` のままである。新しい既定値を使うには、作り直す（`./cmake-helper.sh clean` の後に `build`）か、`-DCMAKE_INSTALL_PREFIX=$HOME/.local` を指定する。

### 段階2: Autotools のビルドの RPATH（2026-11-21）

- 実施内容
  - `configure.ac`: librcsc を見つけた後に、`--disable-rpath`（`AC_ARG_ENABLE`、既定は埋め込む）と、librcsc の lib ディレクトリ（`LIBRCSCLIB`）を埋め込む `LIBRCSC_RPATH`（`-Wl,-rpath,DIR`）を加えた。`/usr/lib`、`/usr/lib64`、`/lib`、`/lib64` のときは埋め込まない。確認の表示に、埋め込むディレクトリか埋め込まない理由を出す。`AM_CONDITIONAL( USE_RPATH )` を加えた。
  - `src/qt/Makefile.am`: `soccerwindow2_LDFLAGS` に `$(LIBRCSC_RPATH)` を加えた。
  - トップの `Makefile.am`: RPATH を埋め込むときは、`make check` で librcsc を `LD_LIBRARY_PATH` に加えない（`TESTS_LIBRARY_PATH`）。`--disable-rpath` のときは今までどおり加える。
  - `cmake/DistCheck.cmake`: インストールの後に、インストールした実行ファイルを `test/smoke_help.sh` で起動する手順を加えた（librcsc を `LD_LIBRARY_PATH` に加えずに起動できること、つまりインストールのときの RPATH を確かめる）。
  - `reconfig.sh`: `./configure` のインストール先を `$HOME/local` から `$HOME/.local` に変えた（ユーザの指示。「3.」の9）。
- 計画からの変更点
  - `configure` の「最後の表示」ではなく、確認の表示（`checking ...`）に出すことにした（`configure.ac` には最後にまとめて表示する部分がないため）。
- 検証（作業ツリーのコピーと一時的なクローン。手元のシェルには `LD_LIBRARY_PATH` に `~/local/lib` が設定されていたので、`env -u LD_LIBRARY_PATH` で外して実行した）
  - Autotools（既定）: `configure` が `checking for the library path of librcsc to embed (RPATH)... /home/akiyama/local/lib` と表示し、実行ファイルに `RUNPATH: /home/akiyama/local/lib` が入った。`make check`（`LD_LIBRARY_PATH` なし）と `make distcheck` が成功。
  - Autotools（`--disable-rpath`）: `... no (--disable-rpath)` と表示し、RUNPATH は入らなかった。`make check` は `LD_LIBRARY_PATH` を設定して成功した。`LD_LIBRARY_PATH` なしで実行ファイルを起動すると、`librcsc.so.20: cannot open shared object file` で失敗した（埋め込みの有無で違いが出ることを確かめた）。
  - CMake の `distcheck`（今の状態を一時的なクローンにコミットして実行）: 成功。「start the installed executable」の手順で、インストールした実行ファイルが `LD_LIBRARY_PATH` なしで起動した。
  - CI は `LD_LIBRARY_PATH` を設定していないので、CI の Autotools の `make check`・`make distcheck` と CMake の `distcheck` で、埋め込みが確かめられる。

### 段階3: `.desktop` ファイル（2026-11-21）

- 実施内容
  - `soccerwindow2.desktop.in`（新規、トップ）: 「5.3」の内容。`Exec` は `"@SOCCERWINDOW2_BINDIR@/soccerwindow2"`（「サーバに接続」は `-c -d` 付き）。先頭のコメントに、インストールのときに目印を `PREFIX/bin` に置き換えることを書いた。
  - CMake（トップの `CMakeLists.txt`）: `install(CODE ...)` で、インストールのときの `CMAKE_INSTALL_PREFIX` と `CMAKE_INSTALL_BINDIR`（絶対パスならそのまま）から目印を置き換えてビルドディレクトリに `.desktop` ファイルを作り、`install(FILES ...)` で `share/applications/` に置く。
  - Autotools（トップの `Makefile.am`）: `sed` で目印を `$(bindir)` に置き換える規則と、`applications_DATA` を加えた。テンプレートを `EXTRA_DIST` に、作ったファイルを `CLEANFILES` に加えた。
  - `src/qt/main.cpp`: `QGuiApplication::setDesktopFileName( "soccerwindow2" )` を呼ぶ。
  - `test/check_desktop_entry.sh`（新規）: テンプレートの `Exec`、「サーバに接続」の `Exec` と `Name`、`Icon`（`icons/app/svg/` のアプリのアイコン）、`StartupWMClass` を調べ、空白を含む仮のパスで作った `.desktop` ファイルを `desktop-file-validate` で検査する。`desktop-file-validate` がなければ、ほかの検査の後に終了コード 77 で飛ばす。CTest（`SKIP_RETURN_CODE 77`）と `make check` に加えた。
  - `cmake/DistCheck.cmake`: インストールした `.desktop` ファイルがあること、その `Exec` がインストール先の実行ファイルを指すこと、`desktop-file-validate` があればその検査を通ることを確かめる。
  - CI（`.github/workflows/build.yml`）: desktop-file-utils をインストールするようにした。
- 計画からの変更点
  - `Exec` のパスを引用符で囲むことにし、目印の名前を `@SOCCERWINDOW2_BINDIR@` にした（「5.3」に反映）。
- 検証
  - `./cmake-helper.sh test qt6`、`./cmake-helper.sh test qt5`: 4つのテスト（`check_desktop_entry` を含む）がすべて成功。
  - `test/check_desktop_entry.sh` の異常系（`Exec` の誤り、「サーバに接続」の引数の誤り、`Actions` の欠落、アイコン名の誤り、規格にない `Type`）を、コピーを壊して確かめた。`desktop-file-validate` を `PATH` から外すと終了コード 77 になった。
  - CMake（一時的なホーム）: 既定のインストール先では `Exec="HOME/.local/bin/soccerwindow2"`、`--prefix` で空白を含むパスを指定したときはそのパスになり、`desktop-file-validate` を通った。`DESTDIR` を使ったときは、`DESTDIR` を含まないパス（`/usr/local/bin`）になった。アンインストールで、最後にインストールした `.desktop` ファイルが消えた。
  - Autotools（作業ツリーのコピー、`LD_LIBRARY_PATH` なし）: `make check`（3つ）、一時的なインストール先への `make install`（`Exec` がインストール先を指し、`desktop-file-validate` を通った）と `make uninstall`（ファイルが残らない）、`make distcheck` が成功。tarball にテンプレートと検査スクリプトが入った。
  - CMake の `distcheck`（一時的なクローン、`LD_LIBRARY_PATH` なし）: 成功（「validate the desktop entry」と「start the installed executable」を含む）。

### 段階4: 文書と実機での確認（2026-11-21）

- 実施内容（文書）
  - `README.md`: librcsc のインストール先を「`~/.local` または `~/local`」とし、soccerwindow2 が両方を探すことを書いた。既定のインストール先（`~/.local`。実行ファイル、アイコン、`.desktop` ファイル）と、前の既定値 `~/local` で設定したビルドディレクトリから移る手順への案内を書いた。Ubuntu では `~/.local/bin` がログインのときに `PATH` に入ること、GNOME のアクティビティから起動できることと「Connect to the Server」を書いた。librcsc のパスの埋め込み（RPATH）と、`LD_LIBRARY_PATH` が必要になる場合とその設定の例（librcsc が `~/.local` の場合と `~/local` の場合）を書いた。
  - `docs/build.md`: 目次に2つの節を加えた。librcsc のインストールの例を `~/.local`（または `~/local`）にした。CMake の既定のインストール先の記述（`cmake-helper.sh` の例、オプションの表）を直した。インストールされるファイルに `.desktop` ファイルを加え、`~/local` から移る手順を書いた。Autotools: `--prefix=$HOME/.local`（`reconfig.sh` と同じ）、`make install` がアイコンと `.desktop` ファイルも置くこと、`--disable-rpath`、`configure` の RPATH の表示と `make check` を書いた。新しい節「Starting soccerwindow2 from the desktop」（探索先、アクティビティでの検索、「Connect to the Server」、ログインし直し、ほかのインストール先では見つからないこと、`PATH`）と「The library path of librcsc (RPATH)」（両方のビルドで埋め込むこと、librcsc を移したらビルドし直すこと、埋め込まない場合、`LD_LIBRARY_PATH` の設定の例、デスクトップは `~/.bashrc` を読まないこと）を加えた。
  - `RELEASE_NOTES.md`: デスクトップからの起動（`.desktop` ファイル、「Connect to the Server」）、CMake の既定のインストール先の変更と移り方、Autotools の RPATH（`--disable-rpath`）と `reconfig.sh`、`check_desktop_entry`、distcheck の追加の確認を書いた。
- 実機での確認（ユーザが実施。Ubuntu の GNOME、Wayland のセッション）
  - 手元の状況: soccerwindow2 はどこにもインストールされていない（`~/local`、`~/.local` とも）。既存のビルドディレクトリ（`build`、`build-qt5`、`build-qt6`）はキャッシュに `~/local` を持つので、`./cmake-helper.sh clean` で作り直してから `./cmake-helper.sh install` で `~/.local` にインストールする。`~/.local/bin` はすでに `PATH` にある。librcsc は `~/local` にあり、RPATH に `~/local/lib` が入る。
  - 確かめること: アクティビティで「soccerwindow2」「RoboCup」で検索して出ること、アイコン、起動（デスクトップは `~/.bashrc` の `LD_LIBRARY_PATH` を読まないので、RPATH の確認にもなる）、右クリックの「Connect to the Server」（`-c -d`）、起動中のウィンドウが Dash で同じアイコンにまとまること。
  - 結果: すべての項目で問題なし（2026-11-21）。

### 段階5: 仕上げ（2026-11-21）

- PR: タイトルと説明の案（英語）をユーザに提示した。PR #94、#95 と同じく、develop へのマージコミット（Create a merge commit）を推奨した（段階ごと・種類ごとに分けたコミットの履歴を残すため）。
- テスト: 段階4 は文書だけの変更なので、段階3 で実施したテスト（CTest の Qt6・Qt5、Autotools の `make check` と `make distcheck`、CMake の `distcheck`）の結果がそのまま有効。CI は PR の作成で実行される。
