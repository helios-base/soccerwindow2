# ゲームログ (RCG) の読み込みの高速化 計画

| 項目 | 内容 |
|---|---|
| 作成日 | 2026-11-25 |
| 最終更新日 | 2026-11-26 |
| 対象 | `develop` (`6c9d09c`)。librcsc は `develop` の `4291244` (PR #123) |
| ブランチ | `feature/load-rcg-in-memory`。`develop` の `6c9d09c` から、ユーザが GitHub で作った |
| 状態 | 完了。PR #99 (`Speed up loading game logs with rcsc::gzread_file() of librcsc`) で develop に取り込む (Squash Merge)。詳しくは「進み具合」を参照 |
| 関連 | librcsc の `docs/todo.md` (「RCG パーサの高速化」の最初の項目。この計画の元)。librcsc の `docs/plan/2026-11-24-02_rcg-parser-speedup.md` (§4.7.2 に soccerwindow2 での試しのビルドの計測、§8 にこの作業を対象外にした記録。PR #123 で develop に取り込み済み、`4291244`) |

## 進み具合

| 段階 | 内容 | 状態 | ユーザの確認 | CI | コミット | プッシュ |
|---|---|---|---|---|---|---|
| 1 | メモリの経路への切り替え、テスト、文書 (§4.1、D1〜D4、S1、S2) | 完了 | 済み | 通過 | `61c2afc`〜`4d5eb8d` (段階 3 とまとめた) | 済み |
| 2 | 使えるライブラリによる読み込み方の選択 (§4.2、D7、S3〜S6)。段階 3 で取りやめた | 完了 | 済み | — | — (取りやめたため、コミットしていない) | — |
| — | 計画書を CLAUDE.md の書式に合わせ、`docs/todo.md` を作った (§8、§9) | 完了 | 済み | — | `4d5eb8d` | 済み |
| 3 | `rcsc::gzread_file()` だけへの整理、時間の表示の綴りの修正 (§4.3、D11〜D13、S7、S8) | 完了 | 済み | 通過 | `61c2afc`〜`4d5eb8d` | 済み |
| — | CI の失敗 (ext4 でディレクトリを開くと異常終了する) の修正 (§4.3.2、D14) | 完了 | 済み | 通過 | `8eda0a7`〜`e0b1f82` | 済み |
| — | コマンドラインで `.json` の名前のゲームログを受け付けない件の調査と見送り (§8、D15) | 完了 | 済み | — | — | — |
| — | develop への取り込み (PR #99、Squash Merge) | 完了 | 済み | 通過 | — | — |

## 1. 目的と方針

librcsc の PR #123 (2026-11-25) で、rcg パーサが速くなった。ファイル全体をメモリに読み込んでから解析し、大きなログでは行を並列に解析する。あわせて、次の API が加わった。

- `rcsc::gzread_file( path, &data )` (`rcsc/gz/gzread_file.h`): ファイル全体をメモリに読み込む。gzip のファイルは展開する (librcsc が libdeflate を使ってビルドされていれば libdeflate、そうでなければ zlib)。
- `rcsc::rcg::Parser::create( data, size )`、`Parser::parse( data, size, handler )`: メモリ上のデータから、パーサを選んで解析する。

soccerwindow2 は `rcsc::gzifstream` と `parse( std::istream & )` でログを読んでいた。librcsc を新しくするだけでも解析は速くなったが、`gzifstream` を通した展開 (zlib で 8 KB ずつ、libdeflate を使わない) の遅さが残っていた。librcsc の計画書の試しのビルドでは、読み込みを替えると、JSON のログを開く時間が約 115 ms から約 54 ms になり、rcssmonitor と同じ水準になることを確かめていた。

この計画では、soccerwindow2 の `MainData::openRCG()` を、`gzread_file()` とメモリの経路に替える (D1)。段階 2 では、使えるライブラリで読み込み方を soccerwindow2 の中で選ぶようにした (D7) が、`gzread_file()` が中で gzip かどうか・libdeflate と zlib のどちらを使うかを判断しているため、段階 3 で `gzread_file()` だけに戻した (D11)。

## 2. 現状 (2026-11-25、`develop` の `6c9d09c`)

- `MainData::openRCG()` (`src/model/main_data.cpp`) は、`HAVE_LIBZ` のとき `rcsc::gzifstream`、そうでなければ `std::ifstream` でファイルを開き、`Parser::create( std::istream & )` と `parse( std::istream &, handler )` で解析していた。開けないときは、それまでのデータを消さずに `false` を返す。
- 時間の表示 (`RCG paser elapsed time`。段階 3 で `RCG parser elapsed time` に直した (D13)) は、パーサを作った後から解析の終わりまでで、`gzifstream` での展開を含んでいた (`parse( std::istream & )` がストリームを読むため)。
- `openRCG()` を呼ぶのは `MainWindow::openRCG( const QString & )` だけで、メニューの「Open」、ドラッグ・アンド・ドロップ、コマンドラインの `-l` がこれを通る。ファイルがないことは、`MainWindow` が先に調べてエラーを出す。
- soccerwindow2 の zlib (`HAVE_LIBZ`) は、ほかに `saveRCG()` (`rcsc::gzofstream`) と、ファイルのダイアログのフィルタ (`*.rcg.gz`) で使う。
- `openRCG()` のテストはなかった。
- librcsc のバージョン (`2026`) と SOVERSION (20) は PR #123 でも変わっていない。`configure` と CMake は librcsc のバージョンやヘッダを調べていない。文書 (`README.md`、`docs/build.md`) は「最新の librcsc (`develop` ブランチ)」を求めている。
- librcsc は、libdeflate を使ってビルドしたかを公開していない。`librcscConfig.cmake`・`librcsc.pc` (`Libs: -lrcsc -lz -lm`)・インストールするヘッダのどれにも情報がなく、CMake のビルドでは libdeflate を PRIVATE でリンクしている。
- librcsc の CI は、CMake のジョブにだけ `libdeflate-dev` を入れ、Autotools のジョブでは zlib だけで確かめている。
- 参考 (librcsc の計画書 §4.7.2 のベンチマーク、JSON のログ): 読み込みだけの時間は、`gzifstream` で約 83 ms、`gzread_file()` (zlib) で約 51 ms、`gzread_file()` (libdeflate) で約 25 ms。
- `gzread_file()` (librcsc の `rcsc/gz/gzread_file.{h,cpp}`) は、中で、ファイルが gzip かどうか、libdeflate と zlib が使えるかで分岐する。gzip は libdeflate があれば libdeflate、なければ zlib で展開し、gzip でないファイルはそのまま読む。librcsc がどちらも使わずにビルドされていれば、gzip のファイルでは false を返す (段階 3 の初めに、ユーザが確かめた)。

## 3. 決定事項

### 3.1 ユーザの決定

| # | 決定 | 決めた日 | いつ |
|---|---|---|---|
| D1 | `openRCG()` の読み込みを、`gzread_file()`・`Parser::create( data, size )`・`Parser::parse( data, size, handler )` に替える。(2026-11-25、段階 2 の初めに D7 で改めた) | 2026-11-25 | 計画の作成前 (ユーザが実装した) |
| D2 | 時間の表示は、`rcsc::Timer` を読み込みの前に移し、読み込み (展開) と解析を合わせた時間を表示する。変更前と同じく「ログを開くのにかかった時間」の意味にする。読み込みと解析を別々に表示する案、解析だけの案と比べて選んだ。D7 のどの経路でも同じ | 2026-11-25 | 段階 1 の実装の確認のとき |
| D3 | `openRCG()` のテスト (Qt Test、`rcg_loading`) を加え、CI (CMake のジョブの `ctest`) で確かめる | 2026-11-25 | 段階 1 の実装の確認のとき |
| D4 | 古い librcsc (PR #123 より前) への対応は、`configure` と CMake では調べず、リリースノートと `docs/build.md` に書くだけにする (文書は既に最新の librcsc を求めている)。D7 の (1) と (3) は PR #123 の API を使うので、古い librcsc ではコンパイルが失敗する ((2) だけはコンパイルできる)。(2026-11-25、段階 3 の初めに D11 で、どの場合も PR #123 の API を使うことになった) | 2026-11-25 | 段階 1 の実装の確認のとき |
| D7 | 読み込み方を、使えるライブラリで選ぶ。(1) libdeflate が使えれば、最優先で `rcsc::gzread_file()` とメモリの経路を使う。(2) libdeflate がなく zlib が使えれば、次善の方法として従来の実装 (`rcsc::gzifstream` と `parse( std::istream & )`) を使う。(3) どちらもなければ、生のファイルとして読むことを試み、gzip のファイルであればエラーとして諦める (D1 を改めた)。(2026-11-25、段階 3 の初めに D11 で取りやめた) | 2026-11-25 | 段階 2 の初め (段階 1 の確認のとき) |
| D11 | 読み込みは `rcsc::gzread_file()` だけにし、`rcsc::gzifstream` での読み込みと、soccerwindow2 の中での読み込み方の切り替え (D7) を除く。zlib だけでも `gzread_file()` の方が `gzifstream` より速く、`gzread_file()` が中で gzip かどうか・libdeflate と zlib のどちらを使うかを判断しているため、soccerwindow2 でマクロを使って切り替える意味がない (D7 を改めた。D1 の形に戻した) | 2026-11-25 | 段階 3 の初め (段階 2 の確認のとき) |
| D12 | zlib も libdeflate もない場合 (旧 D7 の (3)) は確かめない。librcsc の `gzread_file()` の中で扱われる | 2026-11-25 | 段階 3 の初め (段階 2 の確認のとき) |
| D13 | 時間の表示の綴りを直し、`RCG paser elapsed time` を `RCG parser elapsed time` にする (S2 を改めた。ユーザが `src/model/main_data.cpp` を直した) | 2026-11-25 | 段階 3 (段階 3 の確認のとき) |
| D14 | CI で見つかった、ディレクトリを開くと異常終了する不具合 (`gzread_file()` が `std::length_error` を投げる。§9) は、soccerwindow2 の `openRCG()` で `gzread_file()` を try/catch で囲み、例外を読み込みの失敗として扱って直す。librcsc の `gzread_file()` の修正は、librcsc の `docs/todo.md` に記録する (この計画では行わない) | 2026-11-26 | CI の失敗の報告のとき |
| D15 | rcgverconv で JSON に変換したログを `test.json` のような名前でコマンドラインに置くと、soccerwindow2 は `Unsupported file` を出して終了する (§9)。この件には、今回は対応しない。`docs/todo.md` に記録する | 2026-11-26 | PR #99 の CI の確認の後 (ユーザの報告を調べたとき) |

D5・D6・D8・D9・D10 は Claude の判断だったため、3.2 の S1〜S5 に移した (2026-11-25、§9 を参照)。D の番号は振り直していない。

### 3.2 設計上の判断 (Claude)

| # | 判断 | 理由 |
|---|---|---|
| S1 | (旧 D5) テストのログは、librcsc がテストに使う `rcsc/rcg/test_data/` の `head_v5.rcg`・`head_v6.rcg`・`head_json.rcg` (実際のログの先頭、20〜40 KB) を `test/data/` に複製する。gzip のファイルは、テストの中で zlib を使って一時ディレクトリに作る (gzip のファイルをリポジトリに入れない) (段階 1 で決めた) | テキスト v5・v6 と JSON を、小さな実際のデータで確かめられる |
| S2 | (旧 D6) 表示の文字列 (`RCG paser elapsed time` の綴りを含む) は変えない (段階 1 で決めた)。(2026-11-25、段階 3 で D13 により綴りを直した) | 出力を見ている人や、librcsc の計画書での計測と比べられるようにする |
| S3 | (旧 D8) 「libdeflate が使える」は、soccerwindow2 の CMake と configure で libdeflate (`libdeflate.h` とライブラリ) を探して決め、`config.h` に `HAVE_LIBDEFLATE` を定義する。soccerwindow2 は libdeflate をリンクしない。止めるオプションは librcsc と同じ名前にする (CMake の `ENABLE_LIBDEFLATE` (既定 `ON`)、configure の `--without-libdeflate`)。どの経路かを、CMake の Build settings と configure の出力に表示する (段階 2 の初めに決めた)。(2026-11-25、段階 3 で D11 により取りやめた) | librcsc は libdeflate を使ってビルドしたかを公開していない (§2)。libdeflate があるのに librcsc が使わずにビルドされていても、`gzread_file()` は zlib で展開するので読める |
| S4 | (旧 D9) (3) では、`std::ifstream` でファイル全体を 64 KB ずつメモリに読み、メモリの経路 (`Parser::create( data, size )`・`parse( data, size, handler )`) で解析する。gzip かどうかは、先頭の 2 バイト (`0x1f 0x8b`) で判断する (拡張子では判断しない) (段階 2 で決めた)。(2026-11-25、段階 3 で D11 により取りやめた) | `parse( std::istream & )` も中でメモリに読み込むので、メモリの経路の方が単純で速い。1 文字ずつ読む `std::istreambuf_iterator` は遅く、ディレクトリで例外が出たため使わない (§4.2.2) |
| S5 | (旧 D10) CI では、librcsc の CI と同じく、CMake のジョブにだけ `libdeflate-dev` を入れる。CMake のジョブで (1)、Autotools のジョブで (2) を確かめ、`config.h` の `HAVE_LIBDEFLATE` が想定どおりかを `test/check_libdeflate.sh` で調べる。CMake のジョブの librcsc のキャッシュのキーに libdeflate を含める (段階 2 で決めた)。(2026-11-25、段階 3 で S7 に改めた) | librcsc も libdeflate を使ってビルドされ、(1) が本来の形で確かめられる。古いキャッシュ (libdeflate なしの librcsc) を使わないため |
| S6 | (2) では、開けたが解析できないファイル (ディレクトリなど) で、`clear()` の後に失敗するふるまいを、従来どおり残す。テストでは、データが残ることを、存在しないファイルの場合だけで確かめる (段階 2 で決めた)。(2026-11-25、段階 3 で D11 により (2) がなくなり、取りやめた) | D7 で、(2) は従来の実装を使うと決めたため |
| S7 | CI では、CMake のジョブにだけ `libdeflate-dev` を入れることと、librcsc のキャッシュのキーに libdeflate を含めることを残す (librcsc は CMake のジョブで libdeflate、Autotools のジョブで zlib を使ってビルドされる)。`HAVE_LIBDEFLATE` を確かめる手順と `test/check_libdeflate.sh` は除く (S5 を改めた) (段階 3 で決めた) | CMake のジョブで動く `rcg_loading` が、文書で勧める組み合わせ (libdeflate を使う librcsc) で gzip のログを読む。`rcg_loading` は Qt Test で CMake のジョブでしか動かないため、zlib だけの librcsc での展開は soccerwindow2 の CI では確かめられないが、librcsc の CI (Autotools のジョブ) で確かめられていて、手元でも確かめた (§4.3.2)。librcsc の CI と同じ組み合わせ |
| S8 | 文書では、librcsc のために `libdeflate-dev` を入れることを勧める (`README.md` の Quick Start、`docs/build.md` のパッケージと節「How game logs are read」)。zlib は、soccerwindow2 では gzip での保存に使うと書く (段階 3 で決めた) | librcsc を libdeflate を使ってビルドすれば、gzip のログを速く読める。Quick Start では librcsc より先にパッケージを入れる |

### 3.3 段階の初めに決めること

- 段階 2: 「libdeflate が使える」をどう判断するか (librcsc は公開していない)。→ soccerwindow2 の CMake と configure で探す (S3)。
- 取り込みの前: (3) (zlib も libdeflate もない場合) を CI でも確かめるか。今は手元でだけ確かめた (§4.2.2)。CI に加えると、ジョブごとにビルドが 1 回増える。→ 確かめない (D12)。
- 段階 3: zlib だけのときに、`gzread_file()` と `gzifstream` のどちらを使うか。→ `gzread_file()` だけにする (D11)。

## 4. 設計

### 4.1 段階 1: メモリの経路への切り替え

#### 4.1.1 設計

- `openRCG()` は、`gzread_file()` でファイル全体を `std::string` に読み込み、`Parser::create( data.data(), data.size() )` と `parse( data.data(), data.size(), M_view_holder )` で解析する (D1)。
- 読めないときは、`clear()` の前に `false` を返す (変更前と同じく、それまでのデータを消さない)。
- `rcsc::Timer` を読み込みの前で始める (D2)。表示の文字列は変えない (S2)。
- テスト `rcg_loading` を加える (D3、S1)。
- 文書: リリースノートと `docs/build.md` に、速さ、必要な librcsc (D4)、テストを書く。

#### 4.1.2 実装 (2026-11-25)

- ユーザが、librcsc の `docs/todo.md` に基づいて `openRCG()` を書き換えた (D1)。Claude が確かめ、時間の表示が解析だけの時間になること (展開を含まない)、行末の空白、古い librcsc でのコンパイルの失敗、テストがないことをユーザに伝えて、D2〜D4 を決めた。
- D2 に合わせて `rcsc::Timer` を読み込みの前に移し、行末の空白を除き、`// create rcg parser` のコメントを戻した。`rcsc/gz/gzfstream.h` と `<fstream>` は、`saveRCG()` (`gzofstream`) などで使うので残した。
- テスト `test/test_rcg_loading.cpp` を加えた (§6)。`test/CMakeLists.txt` に登録し、テストのデータのディレクトリを `SOCCERWINDOW2_TEST_DATA_DIR` で渡す。`Makefile.am` の `EXTRA_DIST` に、テストのソースと `test/data/` の 3 つのファイルを加えた (Autotools ではテストをビルドしないが、ソースの tarball に入れる)。
- 文書: `RELEASE_NOTES.md`、`docs/build.md` (依存の表、テストの一覧)。
- 確かめたこと (環境: Ubuntu 24.04、GCC 13.3、librcsc は `4291244` を CMake でビルドして `~/.local` にインストールしたもの (zlib と libdeflate を使う)。ビルドは scratchpad で行い、soccerwindow2 の実行では HOME などを一時ディレクトリにした):
  - CMake (Release、Qt 6.4.2 と Qt 5.15.13): 警告 0。`ctest` は 10 個のうち 9 個が通り、`monitor_client_live` は飛ばされた。
  - Autotools (Qt 5.15.13、作業ツリーの写し): 警告 0。`make distcheck` が通った (3 個のテスト)。tarball に新しいテストとデータが入っている。
  - ログを開く時間: 画面を使わずに (`QT_QPA_PLATFORM=offscreen`) `-l <ログ>` で開き、`RCG paser elapsed time` を 3 回ずつ測った (CMake の Release、Qt 6.4.2)。ログは librcsc の計画書 (§2.7) と同じもの。フレームの数は変更前と同じだった。

| ログ | フレーム数 | 変更前 (`6c9d09c`、ms) | 変更後 (ms) |
|---|---|---|---|
| JSON (`1-json.rcg.gz`、展開後 53.6 MB) | 6,209 | 140.9〜143.2 | 66.3〜67.8 |
| ULG5 (`202207160432-HELIOS2022_1-vs-CYRUS_0.rcg.gz`、28.7 MB) | 8,406 | 114.2〜118.8 | 56.6〜57.0 |
| ULG6 (`202607050404-HELIOS2026_0-vs-YuShan2026_1.rcg.gz`、22.3 MB) | 6,469 | 93.1〜94.6 | 48.5〜49.0 |

  - 内訳を見るため、読み込みの時間も表示する写しを scratchpad でビルドして測った: JSON で読み込み 38.3〜39.3 ms + 解析 27.5〜28.6 ms、ULG5 で 29.7〜31.2 + 26.1〜31.8 ms、ULG6 で 17.6〜22.6 + 21.8〜25.7 ms。
  - librcsc の計画書の計測 (JSON で 115 → 54 ms) より全体に遅いのは、計測したときの機械の負荷の違いと考えられる。変更前と変更後の比はおよそ 2 倍で、librcsc の計画書と同じ。
  - いろいろなファイル: 圧縮していない ULG6 (6,469 フレーム)、その先頭 1 MB (途中で切れている。256 フレーム)、JSON の先頭 3 MB を gzip にしたもの (345 フレーム) を開けた。ゲームログでないテキストと存在しないファイルは、変更前と同じ扱いだった (§8)。

### 4.2 段階 2: 使えるライブラリによる読み込み方の選択

#### 4.2.1 設計

この段階の読み込み方の選択は、段階 3 で取りやめた (D11、§4.3)。

- `openRCG()` は、D7 の 3 つの経路を `#if` で選ぶ。
  - `HAVE_LIBDEFLATE` があるか、`HAVE_LIBZ` がないとき、`SOCCERWINDOW2_READ_RCG_IN_MEMORY` を定義し、メモリの経路を使う。ファイルの読み込みは、無名名前空間の `read_rcg_file()` にまとめる。(1) `HAVE_LIBDEFLATE` では `rcsc::gzread_file()` で読む。(3) どちらもないときは S4 のとおり読み、gzip ならエラーにする。
  - (2) `HAVE_LIBZ` だけのときは、従来と同じく `rcsc::gzifstream` で開き、`Parser::create( fin )` と `parse( fin, M_view_holder )` で解析する (S6)。
  - `rcsc/gz/gzread_file.h` は `HAVE_LIBDEFLATE` のときだけ、`rcsc/gz/gzfstream.h` は `HAVE_LIBZ` のときだけ読み込む。
- ビルドシステム (S3)
  - `CMakeLists.txt`: `ENABLE_LIBDEFLATE` (既定 `ON`) のとき、`find_path( libdeflate.h )` と `find_library( deflate libdeflate )` で探す (librcsc の CMake と同じ探し方)。
  - `cmake-config.h.in`: `#cmakedefine HAVE_LIBDEFLATE`。
  - `configure.ac`: `--without-libdeflate` (既定は見つかれば使う。`--with-libdeflate` で見つからなければエラー)。`libdeflate.h` と `libdeflate_gzip_decompress_ex` を調べる。`LIBS` には加えない。
- テスト: 3 つの経路で同じテストが通るようにする。(3) で gzip を拒むことを、壊れた gzip のファイルで確かめる。
- CI (S5): `.github/workflows/build.yml` と、`config.h` を調べるスクリプト `test/check_libdeflate.sh` (`test/check_websocket.sh` と同じ形)。
- 文書: 3 つの読み込み方、オプション、`libdeflate-dev` を書く。

#### 4.2.2 実装 (2026-11-25)

- 4.2.1 のとおり実装した。CMake の Build settings に `game logs are read by: ...` を、configure に `configure: game logs are read by ...` を表示する。
- 3 つの経路をそれぞれ CMake でビルドして `rcg_loading` を実行したところ、次の 2 つが分かり、直した。
  - (2) では、ディレクトリを開くと、解析が失敗する前に `clear()` されて、データが消える (従来の実装のふるまい)。テストで、データが残ることは存在しないファイルの場合だけで確かめるようにした (S6)。
  - (3) では、ディレクトリを `std::istreambuf_iterator` で読むと、libstdc++ の `std::filebuf` が例外を投げた。また 1 文字ずつ読むので遅かった (圧縮していない ULG6 で 67〜68 ms。変更前は約 43 ms)。`istream::read()` で 64 KB ずつ読むようにした (`read()` は例外を投げずに badbit を立てる) (S4)。
- テストに、壊れた gzip のファイル (gzip のヘッダの後に圧縮していないデータ) のケースを加えた。どの経路でも `false` になり、(3) では gzip として拒まれる。
- CI (S5): CMake のジョブにだけ `libdeflate-dev` を入れ、`SW2_EXPECT_LIBDEFLATE` (CMake: `yes`、Autotools: `no`) を設定する。「Show the versions」に libdeflate の版を出す。librcsc のキャッシュのキーを、CMake のジョブでは `librcsc-<os>-cmake-libdeflate-<sha>` にした。「Check how the game logs are read」 (CMake・Autotools) で `test/check_libdeflate.sh` を実行する。`Makefile.am` の `EXTRA_DIST` にこのスクリプトを加えた。
- 文書: `RELEASE_NOTES.md` (3 つの読み込み方、オプション、必要な librcsc、テスト、CI)、`docs/build.md` (依存の表に libdeflate、Ubuntu のパッケージに `libdeflate-dev` (librcsc より先に入れる)、CMake・configure のオプション、新しい節「How game logs are read」、テストの一覧)、`README.md` (Quick Start のパッケージに `libdeflate-dev`)。
- 確かめたこと (環境は §4.1.2 と同じ。libdeflate 1.19):

| ビルド | 読み込み方 | 結果 |
|---|---|---|
| CMake、Release、Qt 6.4.2 | (1) libdeflate | 警告 0。`ctest` は 10 個のうち 9 個が通り、`monitor_client_live` は飛ばされた |
| CMake、Release、Qt 5.15.13 | (1) libdeflate | 警告 0。`ctest` は同じく 9 個が通り、1 個が飛ばされた |
| CMake、Release、Qt 6.4.2、`-DENABLE_LIBDEFLATE=OFF` | (2) zlib | 警告 0。`rcg_loading` は 9 個 (関数・ケース) がすべて通った |
| CMake、Release、Qt 6.4.2、`-DENABLE_LIBDEFLATE=OFF -DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=ON` | (3) 生のファイル | 警告 0。`rcg_loading` は 6 個が通り、gzip の 3 ケースが飛ばされた |
| Autotools、Qt 5.15.13 (作業ツリーの写し) | (1) libdeflate | 警告 0。`LIBS` に `-ldeflate` は入らない。`make distcheck` が通った (3 個のテスト)。tarball に新しいテスト・データ・スクリプトが入っている |
| Autotools、`--without-libdeflate` | (2) zlib | 警告 0。`check_libdeflate.sh config.h no` が通った |
| Autotools、`--with-libdeflate` | (1) libdeflate | configure が通った |

  - ログを開く時間 (§4.1.2 と同じ測り方、3 回ずつ)。フレームの数は、どの経路でも変更前と同じだった。(1) は変更前のおよそ 2 倍の速さで、(2) は変更前と同じ (同じ実装のため)。

| ログ | 変更前 (`6c9d09c`、ms) | (1) libdeflate (ms) | (2) zlib (ms) |
|---|---|---|---|
| JSON (gzip、展開後 53.6 MB) | 140.1〜151.2 | 65.4〜70.0 | 132.5〜145.9 |
| ULG5 (gzip、28.7 MB) | 107.6〜115.7 | 55.7〜63.1 | 112.3〜114.7 |
| ULG6 (gzip、22.3 MB) | 91.6〜94.8 | 48.3〜49.7 | 93.2〜100.1 |

  - 圧縮していないログ (ms)。(3) が (1) より遅いのは、読む前に大きさが分からず、文字列を何度か広げるためと考えられる。(3) は zlib もない場合の予備なので、これ以上は速くしない (§8)。

| ログ | 変更前 | (1) libdeflate | (2) zlib | (3) 生のファイル |
|---|---|---|---|---|
| JSON (53.6 MB) | 66.8〜70.3 | 47.7〜49.4 | 66.9〜69.7 | 56.7〜65.1 |
| ULG6 (22.3 MB) | 42.8〜43.7 | 32.7〜34.9 | 40.0〜43.8 | 36.7〜42.2 |

  - いろいろなファイル: (3) では、gzip のログは `a gzip file cannot be read without zlib and libdeflate.` を出して失敗し、圧縮していないログの先頭 1 MB (途中で切れている) は開けた (256 フレーム)。ゲームログでないテキストは、どの経路でも変更前と同じ扱いだった (§8)。

### 4.3 段階 3: `rcsc::gzread_file()` だけへの整理

#### 4.3.1 設計

- `openRCG()` は、段階 1 の形に戻す。どの場合も `gzread_file()` で読み、`Parser::create( data, size )` と `parse( data, size, handler )` で解析する (D11)。gzip の展開と、librcsc が libdeflate・zlib を使えないときの扱いは `gzread_file()` に任せる (D12)。
- 段階 2 で加えた次のものを除く: `openRCG()` の `#if` の経路と `read_rcg_file()`、`gzifstream` での読み込み、CMake・configure の libdeflate の検出とオプション (`ENABLE_LIBDEFLATE`、`--without-libdeflate`、`HAVE_LIBDEFLATE`)、`test/check_libdeflate.sh`。`rcsc/gz/gzfstream.h` (`HAVE_LIBZ`) は、`saveRCG()` の `gzofstream` で使うので残す。
- テスト: 存在しないファイル、ディレクトリ、壊れた gzip のファイルのどれでも、それまでのデータが残ることを確かめる (`gzread_file()` は、どれでも `clear()` の前に失敗する)。
- CI は S7、文書は S8 のとおりにする。

#### 4.3.2 実装 (2026-11-25)

- 4.3.1 のとおり実装した。`src/model/main_data.cpp` は段階 1 と同じ形になり、gzip の展開についてのコメントを加えた。`CMakeLists.txt`・`cmake-config.h.in`・`configure.ac` は、段階 2 の変更 (libdeflate の検出だけだった) を戻し、`develop` と同じになった。
- ユーザが、時間の表示の綴りを `RCG parser elapsed time` に直した (D13)。`docs/build.md`・`RELEASE_NOTES.md` の表示の名前を直し、`docs/todo.md` と §8 から綴りの項目を除いた。
- `.github/workflows/build.yml`: CMake のジョブにだけ `libdeflate-dev` を入れること、libdeflate の版の表示、キャッシュのキーを残し、`SW2_EXPECT_LIBDEFLATE` と「Check how the game logs are read」の手順を除いた (S7)。
- 文書: `docs/build.md` の節「How game logs are read」を、`gzread_file()` が展開することと、librcsc の libdeflate・zlib ごとの時間の表に書き直した。依存の表の zlib と libdeflate の説明を直し、オプションの表から `ENABLE_LIBDEFLATE` と `--without-libdeflate` を除いた。`RELEASE_NOTES.md` も同じように直した。`README.md` の `libdeflate-dev` は残した (S8)。
- 確かめたこと (環境は §4.1.2 と同じ):
  - libdeflate を使う librcsc (`~/.local`) で、CMake (Release、Qt 6.4.2 と Qt 5.15.13) は警告 0、`ctest` は 10 個のうち 9 個が通り、`monitor_client_live` は飛ばされた。
  - zlib だけの librcsc (`4291244` を `-DENABLE_LIBDEFLATE=OFF` で scratchpad にビルドしたもの) に `-DLIBRCSC_INSTALL_DIR` で向けた CMake (Release、Qt 6.4.2) も警告 0、`ctest` は同じく 9 個が通った。`rcg_loading` は gzip のケースも含めて 9 個すべて通った。
  - 手元の環境では `LD_LIBRARY_PATH` に `~/.local/lib` が入っていて、別の librcsc に向けてビルドしても、実行時には `~/.local` の librcsc が読まれた。zlib だけの librcsc での確認と計測は、`env -u LD_LIBRARY_PATH` で行い、`ldd` で読まれる librcsc を確かめた。
  - Autotools (Qt 5.15.13、作業ツリーの写し): 警告 0。`make distcheck` が通った (3 個のテスト)。tarball に新しいテストとデータが入っている。
  - ログを開く時間 (§4.1.2 と同じ測り方、3 回ずつ、同じ時に測った)。フレームの数は変更前と同じだった。zlib だけの librcsc でも、変更前 (`gzifstream`) より速い。

| ログ | 変更前 (`6c9d09c`、ms) | libdeflate を使う librcsc (ms) | zlib だけの librcsc (ms) |
|---|---|---|---|
| JSON (gzip、展開後 53.6 MB) | 142.5〜153.5 | 64.3〜72.1 | 109.8〜111.0 |
| ULG5 (gzip、28.7 MB) | 114.4〜124.5 | 57.6〜58.9 | 92.0〜96.3 |
| ULG6 (gzip、22.3 MB) | 73.7〜96.9 | 40.3〜48.9 | 58.3〜76.1 |

- (2026-11-26) CI の失敗の修正 (D14):
  - PR #99 の CI (`4d5eb8d`) で、CMake の 4 つのジョブの `rcg_loading` が異常終了した。ディレクトリを開くケースで、`gzread_file()` が `std::length_error` を投げていた。librcsc の `read_raw()` は `seekg( 0, end )` と `tellg()` で大きさを求めて `std::string::resize()` するが、通常のファイルかを確かめていない。CI の `/tmp` (ext4) ではディレクトリの `tellg()` がとても大きな値を返す。手元の `/tmp` は tmpfs で、`tellg()` が -1 になって false が返るため、気づかなかった。
  - 手元の ext4 の場所で再現した。`soccerwindow2 -l <ディレクトリ>` は、この PR の版では異常終了し (終了コード 134)、変更前の版では異常終了しない (「Failed to parse」と出す)。フォルダをドラッグ・アンド・ドロップしたときも同じになる。
  - `openRCG()` で `gzread_file()` を try/catch で囲み、例外は読み込みの失敗として、`clear()` の前に `false` を返すようにした。大きなファイルでの `std::bad_alloc` も防げる。
  - テストで、一時ディレクトリに加えて、ソースツリーの `test/data` (ディスク上のファイルシステムにあることが多い) をディレクトリとして開くようにした。直す前のコードでは、手元 (`/tmp` は tmpfs) でもこのテストが異常終了することを確かめた。
  - librcsc の `gzread_file()` の修正は、librcsc の `docs/todo.md` に記録した (librcsc のリポジトリの作業ツリーで。コミットはしていない)。
  - 確かめたこと: libdeflate を使う librcsc で CMake (Release、Qt 6.4.2 と Qt 5.15.13) は警告 0、`ctest` は 10 個のうち 9 個が通り、1 個が飛ばされた (`TMPDIR` を ext4 にしても同じ)。zlib だけの librcsc でも同じ。`soccerwindow2 -l <ext4 のディレクトリ>` は `Failed to read rcg file [...]: basic_string::_M_replace_aux` と出して、異常終了しない。
  - 修正の後の CI (`e0b1f82`) は、8 つのジョブすべてが成功した (§9)。

## 5. 段階

| # | 作業 | 完了条件 |
|---|---|---|
| 1 | メモリの経路への切り替え、テスト `rcg_loading`、文書 | CMake と Autotools のビルドで警告がない。テストが通る。ログを開く時間を記録した |
| 2 | 使えるライブラリによる読み込み方の選択、CI、文書 (段階 3 で取りやめた) | 3 つの経路のビルドで警告がなく、`rcg_loading` が通る。CI で (1) と (2) を確かめる (CMake・Autotools のジョブ)。ログを開く時間を記録した |
| 3 | `rcsc::gzread_file()` だけへの整理、CI、文書 | libdeflate を使う librcsc と zlib だけの librcsc の両方で、ビルドに警告がなく、テストが通る。ログを開く時間を記録した |
| — | develop への取り込み | CI がすべて通る。PR を Squash Merge で取り込む (ユーザ) |

## 6. テスト

- `rcg_loading` (`test/test_rcg_loading.cpp`、Qt Test、CMake だけ)
  - `openRCG` (データ駆動、6 ケース): v5・v6・JSON のそれぞれを、圧縮なしと gzip で開き、フレームの数、最初と最後のサイクル、最後のフレームのチーム名、プレイヤータイプの数 (18) を確かめる。zlib なしのビルドでは、gzip のケースを飛ばす (gzip のファイルを作れないため)。
  - `openRCGFailure`: v6 を開いた後、存在しないファイル、ディレクトリ (一時ディレクトリと、ソースツリーの `test/data`)、壊れた gzip のファイル (gzip のヘッダの後に圧縮していないデータ) を開くと `false` になり、それまでのデータ (5 フレーム) が残ることを確かめる。ディレクトリを読んだ結果はファイルシステムで違う (ext4 では `gzread_file()` が例外を投げる) ため、tmpfs にあることがある一時ディレクトリだけでなく、ソースツリーのディレクトリも開く (D14)。
  - 他のテストと同じく、HOME・XDG_CONFIG_HOME・XDG_RUNTIME_DIR を一時ディレクトリにする。
- CI: `rcg_loading` は CMake のジョブで、libdeflate を使う librcsc で動く。Autotools のジョブ (zlib だけの librcsc) では動かない (Qt Test のテストは CMake だけ) (S7)。
- 手元での動かし方:
  - CMake: `./cmake-helper.sh test` (または `ctest --test-dir build -R rcg_loading --output-on-failure`)。
  - zlib だけの librcsc で確かめるとき: librcsc を `-DENABLE_LIBDEFLATE=OFF` で別の場所 (例: `/tmp/librcsc-zlib`) にインストールし、soccerwindow2 を `-DLIBRCSC_INSTALL_DIR=/tmp/librcsc-zlib` で別のビルドディレクトリにビルドする。`LD_LIBRARY_PATH` に別の librcsc があると、実行時にそちらが読まれるので、`env -u LD_LIBRARY_PATH` で実行し、`ldd` で読まれる librcsc を確かめる。
  - ログを開く時間: HOME などを一時ディレクトリにして、`QT_QPA_PLATFORM=offscreen soccerwindow2 -l <ログ>` の出力の `RCG parser elapsed time` (2026-11-25 の段階 3 より前は `RCG paser elapsed time`) を見る (`~/.soccerwindow2` を書き換えないため、HOME を変えずに実行しない)。

## 7. 変更するファイル (見込み)

- `src/model/main_data.cpp`
- `Makefile.am` (`EXTRA_DIST`)
- `test/test_rcg_loading.cpp` (新規)、`test/data/head_v5.rcg`・`head_v6.rcg`・`head_json.rcg` (新規。librcsc から複製)、`test/CMakeLists.txt`
- `.github/workflows/build.yml`
- `RELEASE_NOTES.md`、`docs/build.md`、`README.md`
- `docs/todo.md` (新規)

## 8. 対象外

この章の項目は、`docs/todo.md` にも記録した (2026-11-25)。

- ファイルのダイアログのフィルタ (`*.rcg.gz`) は、従来どおり soccerwindow2 の zlib (`HAVE_LIBZ`) で決まる。gzip のログを読むのは librcsc なので、soccerwindow2 を zlib なしでビルドすると、librcsc が gzip を読める場合でも、フィルタに `*.rcg.gz` が出ない (「All files」で選べる)。まれな場合なので、このままにする。
- コマンドラインに置いたファイル (`-l` を付けないもの) を、`.rcg`・`.rcg.gz`・`.conf` 以外の名前でも受け付けること (D15)。`src/model/options.cpp` は拡張子で振り分け、それ以外の名前では `Unsupported file: [...]` を出して終了する。rcgverconv で JSON に変換したログを `.json` の名前にすると、中身は読めるのに開けない。rcssmonitor は、コマンドラインに置いたファイルを拡張子によらずゲームログとして扱う。回避策は、`-l` を付けて渡すか、`.rcg` (`.rcg.gz`) の名前にすること。直すなら、`.conf` 以外をすべてゲームログとして扱うか、`.json`・`.json.gz` を足す。「Open」のダイアログのフィルタ (`*.rcg *.rcg.gz`) も同じ。
- librcsc の `gzread_file()` が、通常のファイルでないもの (ディレクトリなど) を拒むようにすること。soccerwindow2 では try/catch で守った (D14)。librcsc の `docs/todo.md` にも記録した。
- ゲームログでないファイルが、0 フレームで「開けた」ことになるのは、librcsc の `Parser::create()` の判定によるもので、変更前と同じ。
- librcsc の `docs/todo.md` の該当の項目に、この計画書と PR を書き添えること。librcsc のリポジトリでの作業で、PR の番号が決まった後に、ユーザと相談して行う。

## 9. 記録

### 2026-11-25: 計画を作成し、段階 1 を実装した

- ユーザが、librcsc の `docs/todo.md` に基づいて `MainData::openRCG()` を書き換えた (D1)。
- Claude が変更を確かめ、ユーザと D2〜D4 を決めた。Claude は S1・S2 を決めた。
- `rcsc::Timer` の位置などを直し、テスト `rcg_loading` を加え、文書を更新した (§4.1.2)。
- 確かめたこと: CMake (Qt 6.4.2・5.15.13) と Autotools で警告 0、`ctest` は 9 個が通り 1 個が飛ばされた、`make distcheck` が通った。JSON のログを開く時間は 140.9〜143.2 → 66.3〜67.8 ms になった (§4.1.2)。
- ブランチ名・コミットメッセージ・PR の説明の案をユーザに示した。

### 2026-11-25: 段階 2 を実装した

- ユーザの指示で、読み込み方を、libdeflate があれば `gzread_file()`、zlib だけなら従来の実装、どちらもなければ生のファイル (gzip ならエラー) とすることにした (D7)。ユーザは軽微な修正としたが、この計画書に記録する。まだコミットしていない同じ作業なので、新しい計画書は作らず、この計画書に書き足した。
- librcsc が libdeflate を使っているかを公開していないことを確かめ (§2)、soccerwindow2 で libdeflate を探すことにした (S3)。
- `openRCG()` を 3 つの経路に分け、CMake と configure に libdeflate の検出とオプションを加え、CI に libdeflate と `test/check_libdeflate.sh` を加えた (§4.2.2)。
- 3 つの経路のテストで分かった 2 つのこと ((2) のディレクトリでデータが消える、(3) のディレクトリで例外が出て遅い) に対応した (S4、S6、§4.2.2)。
- 確かめたこと: 3 つの経路の CMake のビルドと Autotools (`--with-libdeflate`・`--without-libdeflate`) で警告 0、`rcg_loading` が通った ((3) では gzip の 3 ケースが飛ばされた)、`make distcheck` が通った。(1) で JSON のログを開く時間は 65.4〜70.0 ms、(2) は変更前と同じ (§4.2.2)。
- 文書を、3 つの読み込み方に合わせて直した。コミットメッセージと PR の説明の案を直し、ユーザに示した。

### 2026-11-25: 計画書を CLAUDE.md の書式に合わせた

- CLAUDE.md に計画書の書式と `docs/todo.md` の指示が加わったため、ユーザの指示で、この計画書を書式に合わせた。まだコミットしていない計画書なので、章の構成も組み替えた。
- 決定事項の番号の対応: D5 → S1、D6 → S2、D8 → S3、D9 → S4、D10 → S5 (Claude の判断を 3.2 に移した)。D1〜D4・D7 は変えていない。段階 2 で決めて記録にだけ書いていたことを S6 とした。
- 節の対応 (旧 → 新): §4.1 `MainData::openRCG()` → §4.1.2・§4.2.1・§4.2.2、§4.2 ビルドシステム → §4.2.1、§4.3 テスト → §4.1.2・§4.2.2・§6、§4.4 CI → §4.2.2、§4.5 文書 → §4.1.2・§4.2.2、§5 確認の結果 → §4.1.2・§4.2.2、§6 対象外・気づいたこと → §8、§7 この後の作業 → §3.3・§8、§8 実装記録 → §9。
- `.github/workflows/build.yml` のコメントにある参照を、D10 から S5 に直した。
- §8 の項目を、`docs/todo.md` (新規) に記録した。
- この後: ユーザが段階 2 とこの計画書を確かめ、(3) を CI でも確かめるかを決める (§3.3)。ユーザが GitHub で作業ブランチを作った後、手元でそのブランチに切り替えて (まだコミットしていない変更はそのまま持ち越す)、ユーザの確認の後にコミットしてプッシュする。ユーザが PR を作り、Claude が CI の結果を確かめて報告し、この計画書に記録する。取り込みの前に、取り込みの記録を PR のブランチにコミットする。

### 2026-11-25: 段階 3 を実装した (`rcsc::gzread_file()` だけに整理した)

- ユーザが、段階 2 とこの計画書を確かめた。zlib だけでも `gzread_file()` の方が速く、`gzread_file()` が中で gzip かどうか・libdeflate と zlib のどちらを使うかを判断していることをユーザが確かめ、読み込みを `gzread_file()` だけにすることにした (D11)。zlib も libdeflate もない場合は確かめないことにした (D12)。Claude は S7・S8 を決めた。
- `openRCG()` を段階 1 の形に戻し、段階 2 で加えた libdeflate の検出・オプション・`test/check_libdeflate.sh` を除いた。CI は `libdeflate-dev` の部分だけを残した。テストと文書を直した (§4.3.2)。
- `docs/todo.md` から、段階 3 で要らなくなった 2 つの項目 (librcsc が libdeflate を使っているかを知ること、(3) の読み込みを速くすること) を除き、ファイルのダイアログのフィルタの項目を書き直した (§8)。どちらも、まだコミットしていない段階のものだった。
- 確かめたこと: libdeflate を使う librcsc で CMake (Qt 6.4.2・5.15.13) は警告 0、`ctest` は 9 個が通り 1 個が飛ばされた。zlib だけの librcsc でも警告 0、`ctest` は 9 個が通った。Autotools で警告 0、`make distcheck` が通った。JSON のログを開く時間は、変更前の 142.5〜153.5 ms から、libdeflate で 64.3〜72.1 ms、zlib だけで 109.8〜111.0 ms になった (§4.3.2)。
- コミットメッセージと PR の説明の案を直し、ユーザに示した。
- ユーザが、時間の表示の綴りを `RCG paser elapsed time` から `RCG parser elapsed time` に直した (D13)。関係する文書 (`docs/build.md`、`RELEASE_NOTES.md`、`docs/todo.md`、この計画書) とコミットメッセージ・PR の説明の案を直した。
- この後: ユーザが段階 3 を確かめる。ユーザが GitHub で作業ブランチを作った後、手元でそのブランチに切り替えて (まだコミットしていない変更はそのまま持ち越す)、ユーザの確認の後にコミットしてプッシュする。ユーザが PR を作り、Claude が CI の結果を確かめて報告し、この計画書に記録する。取り込みの前に、取り込みの記録を PR のブランチにコミットする。

### 2026-11-26: コミット、PR #99、CI の失敗と修正

- ユーザが GitHub で `feature/load-rcg-in-memory` を `develop` の `6c9d09c` から作った。ユーザの確認の後、段階 1〜3 の変更を 5 つのコミット (`61c2afc` ソース、`c5e4e24` テスト、`53db88b` CI、`e8c6fa5` 文書、`4d5eb8d` 計画書と `docs/todo.md`) にした。プッシュの前に CMake の distcheck を行い、通った (tarball から作ったビルドで、10 個のうち 9 個のテストが通り、1 個が飛ばされた)。プッシュした。
- ユーザが PR #99 (`Speed up loading game logs with rcsc::gzread_file() of librcsc`) を作った。
- CI の結果 (`4d5eb8d`、run 45641172205): Autotools の 4 つのジョブは成功した (`make check` と `make distcheck` で 3 個ずつ通った。librcsc は zlib だけでビルドされた)。CMake の 4 つのジョブは、`rcg_loading` が異常終了して失敗した (ほかの 9 個は通り、または飛ばされた。librcsc は libdeflate を使ってビルドされた)。どのジョブもコンパイラの警告は 0 (Autotools のジョブの 10 件の警告は libtool・automake・autoconf のもの)。
- 原因は、ディレクトリを開くと `gzread_file()` が `std::length_error` を投げることで、この PR で入った不具合 (フォルダを開くと soccerwindow2 が異常終了する) だった。ユーザと D14 を決め、soccerwindow2 で直し、librcsc の修正は librcsc の `docs/todo.md` に記録した (§4.3.2)。この計画の `docs/todo.md` にも記録した (§8)。
- この後: ユーザの確認の後、修正をコミットしてプッシュし、Claude が CI の結果を確かめて報告し、この計画書に記録する。取り込みの前に、取り込みの記録を PR のブランチにコミットする。

### 2026-11-26: CI の結果と develop への取り込み

- ユーザの確認の後、修正を 3 つのコミット (`8eda0a7` ソース、`fb97489` テスト、`e0b1f82` 計画書と `docs/todo.md`) にして、プッシュした。
- CI の結果 (`e0b1f82`、run 45649108006): 8 つのジョブすべてが成功した。どのジョブもコンパイラの警告は 0。
  - CMake の 4 つのジョブ: `ctest` を 3 回 (通常のビルド、`cmake/FindLibrcsc.cmake` での確認、distcheck) 行い、どれも 10 個のうち 9 個が通り、`monitor_client_live` は飛ばされた。`rcg_loading` は 3 回とも通った。librcsc は libdeflate を使ってビルドされた (最初の CI が失敗してキャッシュが保存されなかったため、ビルドし直された)。
  - Autotools の 4 つのジョブ: `make check` と `make distcheck` で 3 個ずつ通った。librcsc は zlib だけでビルドされたもの (最初の CI で作ったキャッシュ) を使った。
- ユーザの報告「rcgverconv で既存のログを JSON に変換したファイルを読ませると `Unsupported file` になる。rcssmonitor では読める」を調べた。`src/model/options.cpp` が、コマンドラインに置いたファイルを拡張子で振り分けていて、`.json` の名前では中身を読む前に終了していた。変換したファイル (`test.json`、ULG6 から変換、6,469 フレーム) は `-l` を付ければ開けた。変更前の版でも同じで、この PR で入った問題ではない。ユーザと D15 を決め、今回は対応せず、§8 と `docs/todo.md` に記録した。
- PR #99 (`Speed up loading game logs with rcsc::gzread_file() of librcsc`) を、Squash Merge で develop に取り込む。この記録は、取り込みの前に PR のブランチにコミットした。
- この計画の結果:
  - ゲームログを `rcsc::gzread_file()` でメモリに読み込み、librcsc のメモリ上のパーサで解析するようにした。JSON のログ (展開後 53.6 MB) を開く時間は、約 145 ms から、libdeflate を使う librcsc で約 67 ms、zlib だけの librcsc で約 110 ms になった。
  - ディレクトリを開くと異常終了する不具合 (`gzread_file()` の例外) を、この PR の中で見つけて直した (D14)。
  - 時間の表示の綴りを直した (D13)。テスト `rcg_loading` を加え、CI の CMake のジョブで libdeflate を使う librcsc を確かめるようにした。
- この後: 手元の develop を更新する。librcsc の `docs/todo.md` の「RCG パーサの高速化」の最初の項目に、この計画書と PR #99 で対応したことを書き添え、`gzread_file()` の項目 (2026-11-26) とあわせて librcsc でコミットする (方法はユーザと相談する)。ほかに見送った作業は `docs/todo.md` にある。
