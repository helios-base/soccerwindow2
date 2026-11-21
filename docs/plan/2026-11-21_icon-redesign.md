# アイコンの刷新（明るい配色用・暗い配色用の2組）計画書

- 作成日: 2026-11-21
- ステータス: 段階3 を実施済み（コミット待ち）
- ブランチ: `feature/icon-redesign`（2026-11-21 にユーザが GitHub 上で develop から作成）
- 前の計画: `docs/plan/2026-11-19_qt6-cmake-migration.md` の Phase 10（PR #94 のマージ後に別の計画で行うことにしたもの）を引き継ぐ。

## 1. 背景と目的

Qt6 版はデスクトップの暗い配色に従うため、前の計画の Phase 5 で次の対応をした。

- 既定では明るい配色（Qt5 版と同じ色）を使い、「View > UI Color Scheme > System」でデスクトップの配色に従う。
- 暗い配色では、アイコンの黒・灰色の画素を実行時に文字色に置き換える（`UIIcon`。方式 (a)）。
- ツールバーの押下中のボタンを、ハイライト色の背景と枠で強調する（`UIStyle`。方式 (d)）。

Phase 5-2 の比較では、明るい配色用と暗い配色用の2組のアイコンを新しく作る方式 (c-2) が最も読みやすかった。一方 (a) では、白い塗りと灰色の図形のアイコン（FormationEditorWindow のもの）、sync-ball、open が、暗い配色で見づらいまま残った。

この計画では、次のことを行う。

1. 使っているアイコンをすべて SVG で描き直し、明るい配色用と暗い配色用の2組の PNG を作る。
2. PNG を Qt のリソース（`.qrc`）で埋め込み、配色に応じて切り替える。
3. アプリのアイコン（ウィンドウのアイコン）もきれいにする。
4. 最後に、配色の既定値（light）を見直す。

## 2. 現状（2026-11-21、develop の PR #94 マージ後）

### 2.1 アイコン

- アイコンは XPM（ほとんどが 16×16）で、`xpm/` に置き、ソースから `#include` している。
- ツールバーやメニューで使っているアイコンは、番号のアイコンを除いて43個（ツールバーなどの42個と、About メニューの soccerwindow2_nostr）、番号のアイコン（`num_1`〜`num_32`）が32個。どれも `UIIcon::fromXpm()` で読み込んでいる（2026-11-21 の段階0 で数え直した。当初は44個と書いていた）。

| ウィンドウ（ソース） | アイコン |
|---|---|
| FormationEditorWindow（`formation_editor_window.cpp`） | chase, data_auto_select, delete, insert, new, open, record, replace, reverse, save, symmetry, sync_ball, train |
| MainWindow（`main_window.cpp`） | logplayer_go_first, logplayer_go_last, logplayer_go_next_score, logplayer_go_prev_score, logplayer_live_mode, logplayer_one_step_back, logplayer_one_step_forward, logplayer_play_back, logplayer_play_forward, logplayer_play_or_stop, logplayer_shift_down, logplayer_shift_up, logplayer_stop, debug_server_switch, open_rcg, save, soccerwindow2_nostr（About） |
| DebugMessageWindow（`debug_message_window.cpp`） | clear, debug_log_view, debug_view_all, debug_view_ball, debug_view_comment, debug_view_line, debug_view_message, debug_view_opponent, debug_view_self, debug_view_target, debug_view_teammate, logplayer_one_step_back, logplayer_one_step_forward, open_dir, refresh, sync, num_1〜num_32 |
| LabelEditorWindow（`label_editor_window.cpp`） | open, save |

- アプリのアイコン: `soccerwindow2.xpm`（MainWindow の `setWindowIcon`。インストールもされる）、`fedit2.xpm`（FormationEditorWindow の `setWindowIcon`）、`soccerwindow2-nostr.xpm`（About メニューのアイコン）。
- `xpm/` には100個のファイルがあり、使っていない23個は `.gitattributes` の `export-ignore` で配布用の tarball から除いている。
- Phase 5-2 で試作した SVG は、作業環境が変わったため残っていない。描き直しは一から行う。
- 変換ツール: Inkscape 1.4.4（snap 版、`/snap/bin/inkscape`）をユーザがインストールした（2026-11-21）。

### 2.2 実行時の負荷の測定（2026-11-21）

「描くたびに配色を調べる」方式が負荷にならないかを測った。16×16 のアイコンを `QIcon::paint()` で2万回描いて平均をとった（Ubuntu 24.04、Qt 6.4.2、offscreen、`-O2`）。

| 方式 | 明るい配色 | 暗い配色 |
|---|---|---|
| 配色を調べる処理だけ（`UIIcon::isDark( QGuiApplication::palette() )`） | 0.012 µs | 0.012 µs |
| `QIcon( QPixmap )`（配色に応じた処理なし） | 0.57 µs | 0.55 µs |
| 今の `UIIcon::fromXpm()`（暗い配色では毎回、画素の色を置き換える） | 0.50 µs | 7.04 µs |
| 新しい方式（配色を調べ、明るい用・暗い用の `QIcon` のどちらかに任せる） | 0.49 µs | 0.46 µs |

- 配色を調べる処理は約 0.01 µs で、アイコンを描く処理（約 0.5 µs）に比べて無視できる。
- 新しい方式は、配色に応じた処理をしない `QIcon` と同じ速さだった。PNG は `QIcon` が保持するので、描くたびに読み込み直すことはない。
- 今の方式も、暗い配色で1回 7 µs ほどで、ウィンドウ全体（アイコン約80個）を描き直しても 1 ms 未満。アイコンはツールバーやメニューを描き直すときだけ描かれ、フィールドの描画（ログの再生中など）のたびには描かれない。
- 結論: 描くたびに配色を調べる方式で問題ない。新しいアイコンに置き換えると、今より軽くなる。

## 3. 決定事項

ユーザと相談して決めた（2026-11-21）。

1. **デザイン:** 今のアイコンのモチーフ（何を表しているか、形の特徴）は残しつつ、線の太さや色合いを全体で統一して描き直す。
2. **作り方:** SVG で描き、開発時に Inkscape で PNG に変換して、PNG をリポジトリに置く。ビルドや CI には Inkscape は不要。
3. **埋め込み方:** PNG を Qt のリソース（`.qrc`）で埋め込む。ユーザが自分でも修正できるよう、`.qrc` の使い方・書き方を説明するドキュメントを用意する。
4. **切り替え方:** アイコンを描くたびに配色を調べ、明るい用と暗い用を選ぶ（負荷は「2.2」のとおり問題ない）。配色を切り替えると、ウィジェットが描き直されるときに自動で切り替わる。
5. **範囲と順序:**
   - 使っているアイコン42個に加え、アプリのアイコン（soccerwindow2、fedit2、soccerwindow2-nostr）もきれいにする。
   - 番号のアイコン（32個）は1つずつ描かず、実行時に配色に合った色で数字を描く。
   - Phase 5-2 で見づらかったもの（FormationEditorWindow の図形のアイコン、sync-ball、open）から始め、ウィンドウごとに段階に分けて置き換える。
   - 段階ごとに、明るい配色・暗い配色での比較画像をユーザが確認してからコミットする。
6. **古いアイコン:** 削除せず、記録・記念として残す。
   - 置き換えが終わった段階（段階5）で、`xpm/` を `git mv` で `icons/legacy/xpm/` に移す。
   - 一覧のページ（`icons/legacy/README.md`）を置き、古いアイコンを並べた画像と、古いアイコンと新しいアイコンの比較画像を載せる。
   - ビルドでは使わず、`.gitattributes` の `export-ignore` で配布用の tarball からは除く。

7. **デザインの決まりと色（段階0 で決定）:** 段階0 の見本と案（「5.2」）のとおりとする（2026-11-21、ユーザが比較画像を見て決定）。
8. **`icons.qrc` の作り方（段階0 で決定）:** `generate.sh` が SVG の一覧から作る（「5.4」）。
9. **アイコンの文書（段階0 で決定）:** 日本語版を主として `docs/icons.md`、英語版を `docs/icons.en.md` とする（ユーザの指示）。英語の `docs/build.md` からは英語版にリンクする。
10. **番号のアイコンの色（段階3 で決定）:** 今の赤を引き継ぐ（`palette.txt` の `red` と同じ色）。元の XPM、赤、文字色の比較画像を見て、ユーザが決めた。

## 4. 未決事項

| 項目 | 案 |
|---|---|
| アプリのアイコンのインストール | 今は `PREFIX/share/soccerwindow2/soccerwindow2.xpm` をインストールしている。新しいアイコン（PNG、SVG）に変えるか、freedesktop の場所（`PREFIX/share/icons/hicolor/...`）にも置くか。段階4 で決める |
| 色を置き換える処理の扱い | すべて置き換えた後、`UIIcon::fromXpm()` と色の置き換え（方式 (a)）を削除する（案）。段階5 で決める |
| 配色の既定値 | 最後（段階6）に、light のままにするか、System にするかを見直す |

## 5. 方針

### 5.1 ディレクトリ構成

```
icons/
  svg/<name>.svg          元の SVG（明るい配色用の色で描く。Inkscape でそのまま開いて編集できる）
  light/16/<name>.png     明るい配色用（16×16）
  light/32/<name>.png     明るい配色用（32×32。HiDPI 用）
  dark/16/<name>.png      暗い配色用（16×16）
  dark/32/<name>.png      暗い配色用（32×32）
  app/                    アプリのアイコン（複数の大きさ。段階4）
  palette.txt             色の対応表（明るい配色用の色 → 暗い配色用の色）
  palette.awk             色の検査と置き換え（generate.sh と test/check_icons.sh が使う）
  generate.sh             SVG から PNG を作るスクリプト（Inkscape を使う。実行するコマンドを表示する）
  icons.qrc               Qt のリソースの定義（generate.sh が作る）
  icons.am                Automake 用のファイルの一覧（generate.sh が作る）
  generated.sha256        PNG を作ったときの SVG と palette.txt のチェックサム（generate.sh が作る）
  legacy/xpm/             古いアイコン（段階5 で xpm/ から移す）
  legacy/README.md        古いアイコンの一覧と、新しいアイコンとの比較
docs/icons.md             アイコンの追加・修正の手順と、.qrc の使い方・書き方（日本語版。主とする）
docs/icons.en.md          同じ内容の英語版
test/check_icons.sh       アイコンのファイルの検査（Inkscape は不要）
```

- SVG は PNG を作るための「元のファイル」なので、配布用の tarball にも含める（GPL の「改変に適した形式」にあたる）。
- 段階0 での変更: 32px の PNG は、当初の案の `<name>@2x.png` ではなく、大きさごとのディレクトリ（`light/32/<name>.png`）に置く。Qt は名前が `@2x` で終わる画像を読み込むと自動で device pixel ratio を 2 にし（`QImageReader`）、`QIcon::addFile()` も `@2x` のファイルを自動で探すなど、Qt5 と Qt6 で扱いが異なる処理が働くため、それを避けた。

### 5.2 デザインの決まり（段階0 で決定）

- 16×16 の格子（`viewBox="0 0 16 16"`）で描く。1px の線は画素の中心（例: `x="1.5"`）に、塗りの図形の端は画素の境界（例: `x="2"`）に置き、16px で画素にそろえる。32px は同じ SVG を2倍で出力する（線は 2px になり、やはり画素にそろう）。
- 余白: ほとんどのアイコンは 1〜15 の範囲（14×14）に描く。図形に必要なら輪郭が端に触れてもよい。
- 線: 輪郭は 1px で `fg` の色。記号（矢印、×など）は 1px か 2px の線。線はなるべく水平・垂直・45° にする。
- 小さな丸（外径 7px 以下）は、画素にそろえた角丸の四角（`rect` の `rx` を幅の半分より 0.5 小さく。例: `width="4" height="4" rx="1.5"`）で描く。`circle` では 16px でぼやけるため（段階1 で追加）。大きな丸（record など）は `circle` でよい。
- ログプレイヤーの記号は、三角を高さ 10px（y 3〜13）・幅 5px（45° の辺）、棒を幅 2px にそろえる（段階2 で追加）。
- 塗り: 部品のある物（フォルダ、書類など）は `fg` の輪郭を付ける。単純な色の印（record の赤い丸など）は輪郭を付けない。黒い記号（再生の三角など）は `fg` で塗り、輪郭を付けない。
- 色は `palette.txt` の役割（線・前景 `fg`、補助の灰色 `fg2`、紙の塗り `paper`、フォルダ `folder`・`folder_back`、強調の `blue`・`red`。段階ごとに必要な役割を加える）に限る。色を薄くするために透明度を使わず、色を加える。
- 元の SVG は明るい配色用の色で描く。`generate.sh` が `palette.txt` に従って色を置き換え、暗い配色用を作る。`palette.txt` にない色が SVG にあればエラーにする（色合いの統一を保つため）。検査するのは `fill`、`stroke`、`stop-color`、`flood-color`、`lighting-color`、`color` の値（`none` と `url(...)` は許す）。
- 選手・チームを表すアイコン（debug_view_teammate など）は、soccerwindow2 のフィールドの描画色と意味が揃うようにする（段階3）。
- 段階0 で決めた色（明るい配色用 → 暗い配色用）: `fg` #303030 → #e0e0e0、`fg2` #7a7a7a → #a0a0a0、`paper` #ffffff → #505050、`folder` #f4c542 → #e3b23c、`folder_back` #c99a1e → #9c7716、`blue` #3b78d8 → #5a96f0、`red` #d93a2f → #ef5345。
- 段階1 で加えた色: `yellow` #f5c800 → #ffd700（選手。FormationEditorWindow とフィールドの選手の色 #ffd700 に合わせた。明るい背景では少し濃くした）、`field` #1fa01f → #3cbf4c（学習データの中の、今のフィールドの状態。フィールドの色 #1fa01f に合わせた）。
- 段階2 で加えた色: `orange` #e8730c → #ff9a3c（デバッグサーバ。元のアイコンの色を引き継いだ）。
- 段階3 で加えた色: `pink` #e0247a → #ff5aa5（デバッグ表示のボール。フィールドの #ff007f に合わせた）、`green` #18a84e → #42d67a（デバッグ表示の味方。フィールドの #00ff5f に合わせた）、`field_line` #f8f8f8 → #f8f8f8（フィールドの白線。両方の配色で白のままにする。LIGHT の色は `paper` の #ffffff と重複できないので #f8f8f8 にした）。デバッグ表示の自分・ターゲット・図形は `orange`、敵は `blue` を使う（フィールドの色に合わせた）。

### 5.3 PNG の作り方

- `icons/generate.sh [NAME...]` で、SVG から明るい用・暗い用の 16px と 32px の PNG を作る（名前を省略するとすべて）。
- Inkscape には SVG を標準入力から渡し、PNG を標準出力で受け取る（`inkscape --pipe --export-type=png --export-filename=- --export-background-opacity=0 -w 16 -h 16 < svg/open.svg > light/16/open.png`）。snap 版の Inkscape は隠しディレクトリのファイルを読めず、シェルのカレントディレクトリも使わないため、Inkscape にファイルを直接読み書きさせない。暗い配色用は `awk -v mode=dark -f palette.awk palette.txt svg/open.svg | inkscape --pipe ...` で作る。1回 0.3 秒ほど。
- Inkscape のコマンドは環境変数 `INKSCAPE` で指定できる（引数付きも可。例: `flatpak run org.inkscape.Inkscape`）。
- `palette.txt` が変わっていれば、名前を指定してもすべてのアイコンを作り直す。
- PNG を作った後、`icons.qrc`、`icons.am`、`generated.sha256` を更新する。`--lists` で一覧（`icons.qrc`、`icons.am`）だけを更新できる（Inkscape は不要）。
- 作った PNG はコミットする。ビルドと CI では SVG の変換を行わない。

### 5.4 埋め込み方

- `icons/icons.qrc` に PNG を並べ、`:/icons/light/16/open.png` のようなパスで読み込む。
- `icons.qrc` は `generate.sh` が SVG の一覧から作る（段階0 で決定）。アイコンごとに4行を手で加える手間と、書き忘れを避けるため。書式は手で書く場合と同じで、`docs/icons.md` で説明する。ほかのファイルを埋め込むときは、別の `.qrc` を手で書いて同じように加える。
- CMake: `icons.qrc` を `soccerwindow2_gui` に加え、`AUTORCC` で変換する。`soccerwindow2_gui` は静的ライブラリなので、リソースを使う前に `Q_INIT_RESOURCE( icons )` を呼ぶ必要がある。これを呼ぶ `UIIcon::initResources()` を用意する。
  - 段階0 での変更: `main()` とテストの `main()` から呼ぶ案をやめ、`UIIcon::fromName()` が初めて呼ばれたときに自動で呼ぶ（関数内の static 変数の初期化で1回だけ）。呼び忘れを防ぐため。`:/icons/...` のパスを直接使う場合に備えて、`initResources()` は公開する。
- Autotools: `m4/sw2_check_qt.m4` で rcc を探し（moc と同じく、Qt6 は `QT_HOST_LIBEXECS`、Qt5 は `QT_HOST_BINS`。変数 `QT_RCC` で指定もできる）、`src/qt/Makefile.am` に `qrc_icons.cpp` を作るルールを加える（`rcc --name icons`）。
  - ファイルの一覧は `generate.sh` が作る `icons/icons.am` に置き、トップの `Makefile.am`（`EXTRA_DIST` に SVG と PNG を加える）と `src/qt/Makefile.am`（`qrc_icons.cpp` が PNG に依存する）の両方から `include` する。パスの前に付けるディレクトリは、それぞれの `Makefile.am` で `ICONS_DIR` に設定する。
- rcc は、すでに必要な `qt6-base-dev-tools` / `qtbase5-dev-tools` に含まれるので、依存パッケージは増えない。

### 5.5 切り替え方

- `UIIcon` に、名前でアイコンを作る `UIIcon::fromName( "open" )` を加える。リソースに PNG がなければ、警告を出して空のアイコンを返す。
- アイコンのエンジン（`SchemeIconEngine`）は、明るい用と暗い用の画像（16px と 32px の `QPixmap`）を持つ。描くたびに `UIIcon::isDark()` で配色を調べ、どちらかの画像を使う。
  - 段階0 での変更: 当初の案（明るい用・暗い用の `QIcon` に任せる）では、内側の `QIcon` も device pixel ratio を掛けるため、HiDPI で大きさが二重に変わる。そのため、画像を直接持って選ぶ。
- 画像の選び方: 描く大きさ（論理サイズ）は、要求された大きさに収まる最大の画像の大きさとし（16 か 32。拡大はしない）、その大きさに device pixel ratio を掛けた大きさの画像を使う。例えば、16px のアイコンは、普通の画面では 16px の画像、device pixel ratio が 2 の画面では 32px の画像を、縮小・拡大せずに使う。ちょうどの画像がなければ（device pixel ratio が 1.5 など）、近い大きい画像を縮小する。ツールバーのアイコンの大きさが 24 の場合（FormationEditorWindow と LabelEditorWindow は大きさを指定しておらず、Fusion の既定の 24 になる）も、今と同じく 16px で描く。
- Qt のバージョンによる違い: Qt6 は `QIconEngine::scaledPixmap()`、Qt5 は `virtual_hook()` の `ScaledPixmapHook` で device pixel ratio を受け取る。Qt 6.8 から `scaledPixmap()` に渡される大きさが論理サイズに変わった（それより前は device pixel ratio を掛けた大きさ）ため、実行時の Qt のバージョン（`QLibraryInfo::version()`）で扱いを分ける。
- 無効（Disabled）・選択中（Selected）の見た目は、今と同じくスタイルに作らせる。
- まだ描き直していないアイコンは `UIIcon::fromXpm()`（色の置き換え）のままにし、段階的に置き換える。

### 5.6 番号のアイコン

- DebugMessageWindow の番号のボタン（1〜32）のアイコンは、実行時に `QPainter` で数字を描いて作る（16px と 32px）。配色に応じて色を選ぶ。
- 段階3 での決定: 数字はフォントではなく、画素にそろえた 2px の線の図形（`UIIcon::fromNumber()`、形は `ui_icon.cpp` の `DIGIT_SHAPES`。SVG のパスと同じ書き方）で描く。同じウィンドウのアイコンの文字（ALL、S、B など）と形がそろい、環境のフォントに左右されず、16px でもぼやけないため。数字は高さ 10px（y 3〜13）、幅 6px、間隔 1px で、3桁以上は縮めて収める。
- 明るい用・暗い用の 16px と 32px の画像を作ってから `SchemeIconEngine` に渡すので、切り替えは `fromName()` と同じ。
- 番号のアイコンは、名前（`debug_text_NN`）のないデバッグレベルにだけ使われる（既定ではレベル1〜21 に名前があり、22〜32 が番号のアイコン）。メニューと、隠れたツールバーに表示される。

### 5.7 アプリのアイコン

- soccerwindow2（MainWindow）、fedit2（FormationEditorWindow）、soccerwindow2-nostr（About）を SVG で描き直す。
- ウィンドウのアイコンは、複数の大きさ（16、32、48、64、128、256px）の PNG を `QIcon` に加えて `setWindowIcon` に渡す。
- ウィンドウのアイコンはウィンドウマネージャが表示するものなので、明るい用・暗い用の切り替えはしない（1組）。

## 6. 実装計画

各段階で、明るい配色と暗い配色の比較画像（古いアイコンと新しいアイコン）を offscreen で書き出してユーザに見てもらい、確認を受けてからコミットする。

### 段階0: 基盤とデザインの見本

- ディレクトリ（「5.1」）、`palette.txt`、`generate.sh`、`icons.qrc` を用意する。
- CMake と Autotools に rcc の処理を加える（「5.4」）。
- `UIIcon::fromName()` と `UIIcon::initResources()` を加える（「5.5」）。
- `docs/icons.md`（日本語版）と `docs/icons.en.md`（英語版）を書く（アイコンの追加・修正の手順、SVG の描き方の決まり、`generate.sh` の使い方、`.qrc` の書き方と使い方、CMake と Autotools での扱い）。
- デザインの見本として、数個（例: open、save、logplayer_play_forward）を描き、線の太さ・色の対応表をユーザと決める。
- テストを加える（「7. テスト」）。

### 段階1: FormationEditorWindow（13個）

- chase、data_auto_select、delete、insert、new、open、record、replace、reverse、save、symmetry、sync_ball、train。
- open と save は LabelEditorWindow・MainWindow でも使っているので、ここで置き換わる（LabelEditorWindow の open、MainWindow の save の2か所）。

### 段階2: MainWindow（15個）

- logplayer_*（13個）、debug_server_switch、open_rcg。
- logplayer_one_step_back と logplayer_one_step_forward は DebugMessageWindow でも使っているので、ここで置き換わる。

### 段階3: DebugMessageWindow（14個と番号のアイコン）

- clear、debug_log_view、debug_view_*（9個）、open_dir、refresh、sync。
- 番号のアイコンを実行時に描く方式に変える（「5.6」）。

### 段階4: アプリのアイコン

- soccerwindow2、fedit2、soccerwindow2-nostr を描き直す（「5.7」）。
- インストールするアイコンの扱いを決めて、CMake と Autotools の両方に反映する（「4. 未決事項」）。

### 段階5: 後片付け

- 古いアイコンを `icons/legacy/` に移し、一覧のページを作る（「3. 決定事項」の6）。
- 使わなくなった `#include` と、色を置き換える処理（「4. 未決事項」による）を削除する。
- `.gitattributes`、`Makefile.am` の `EXTRA_DIST` を整理する。

### 段階6: 配色の既定値の見直しと仕上げ

- 新しいアイコンで、配色の既定値（light）を見直す（ユーザと相談）。
- `RELEASE_NOTES.md`、`docs/build.md`、`README.md` を更新する。
- PR のタイトルと説明の案を作る。

## 7. テスト

- `test/check_icons.sh`（CTest と `make check` の両方で実行する。Inkscape は不要）
  - `palette.txt` の書式が正しいこと。
  - `icons/svg/` の SVG ごとに、明るい用・暗い用の 16px と 32px の PNG があること。SVG のない PNG がないこと。
  - PNG の大きさが正しいこと（PNG のヘッダを `od` で読む）、`icons.qrc` と `icons.am` に載っているファイルと実際のファイルが一致すること。
  - SVG に、`palette.txt` にない色が使われていないこと。
  - PNG が今の SVG と `palette.txt` から作られていること（`generated.sha256` のチェックサムが一致すること）。SVG を直して `generate.sh` を忘れた場合を見つける。
- GUI のテスト（`test/test_gui_smoke.cpp`）
  - 名前で作ったアイコンが、16px と 32px のどちらでも作れること（リソースが埋め込まれていること）。描く大きさの選び方（24 → 16、48 → 32、8 → 8）。
  - 明るい配色と暗い配色で、それぞれの PNG と同じ画像が使われること。配色を切り替えると、ツールバーのアイコンが変わること。
  - device pixel ratio が 2 の描画で 32px の画像が使われること（Qt6 では 1.5 で 24px になることも）。無効の見た目がスタイルで作られること。
  - 埋め込まれたすべてのアイコンに、明るい用・暗い用の 16px と 32px の画像があり、明るい用と暗い用が異なること。
  - ない名前を指定すると、警告を出して空のアイコンを返すこと。
  - 番号のアイコンが、配色に応じた色で描かれること。
  - ウィンドウのアイコンが、複数の大きさを持つこと。
- CI（GitHub Actions の8ジョブ）は今のまま。PNG はコミットされているので、Inkscape は不要。

## 8. 検証方法

1. 段階ごとに、offscreen で比較画像を書き出し、ユーザが確認する。
2. ユーザが実機（明るい配色と、System を選んだ暗い配色）で見え方を確認する。
3. `./cmake-helper.sh test qt5` と `./cmake-helper.sh test qt6`、Autotools の `make check` と `make distcheck`、CMake の `distcheck` が通ること。
4. CI の8ジョブが成功すること。

## 9. 実装記録

（各段階の完了時に、実施内容・計画からの変更点・検証結果を追記する）

### 計画の作成（2026-11-21）

- 前の計画の Phase 10 の内容をもとに、ユーザと相談して決定事項（「3.」）をまとめた。
- 描くたびに配色を調べる方式の負荷を測り（「2.2」）、問題ないことを確かめた。
- 古いアイコンの残し方は案A（`icons/legacy/` に移して一覧のページを置く）に決めた。ほかの案は、削除してタグを付ける案B、案A と案B の両方を行う案C だった。
- ユーザが `feature/icon-redesign` ブランチを作成した。

### 段階0: 基盤とデザインの見本（2026-11-21）

- 実施内容
  - `icons/` を作り、`palette.txt`、`palette.awk`、`generate.sh` を用意した。`generate.sh` が PNG と `icons.qrc`、`icons.am`、`generated.sha256` を作る。
  - デザインの見本として、new、open、save、record、logplayer_play_forward の5個を描いた（「5.2」の案に従う）。試作の比較で、再生の三角は 45° の辺（`M5,2 L11,8 L5,14`）、record は輪郭なしの半径6の丸が最もすっきり見えたので、それにした。暗い配色の赤は、最初の案（#ff6b5e）がピンクに見えたので #ef5345 にした。
  - `UIIcon::fromName()`、`UIIcon::initResources()`、エンジン `SchemeIconEngine` を加えた（`src/qt/ui_icon.h/.cpp`）。ソースのアイコンはまだ置き換えていない（段階1 から）。
  - CMake: `icons.qrc` を `soccerwindow2_gui` に加え、`AUTORCC` を有効にした。テスト `check_icons` を加えた。
  - Autotools: `m4/sw2_check_qt.m4` で rcc を探す（`QT_RCC`）。`Makefile.am` と `src/qt/Makefile.am` に、`icons/icons.am` の `include`、`qrc_icons.cpp` のルール、`EXTRA_DIST`、`make check` の `test/check_icons.sh` を加えた。
  - `test/check_icons.sh` と、GUI のテスト5項目（`uiIconFromName`、`uiIconFromNameHighDpi`、`uiIconFromNameNotFound`、`uiIconResources`、`uiIconFromNameInToolBar`）を加えた。
  - `docs/icons.md`（日本語版）と `docs/icons.en.md`（英語版）を書き、`docs/build.md` のテストと configure の変数（`QT_RCC`）の説明を更新した。
- 計画からの変更点
  - 32px の PNG の置き場所（`@2x` → 大きさごとのディレクトリ）、`icons.qrc` の作り方（`generate.sh` が作る）、`Q_INIT_RESOURCE` の呼び方（`fromName()` から自動で）、エンジンの作り（画像を直接持つ）。理由は「5.1」「5.4」「5.5」に記した。
  - `palette.awk`、`icons.am`、`generated.sha256` を加えた。
  - アイコンの数を数え直した（ツールバーなど42個。段階2 は15個）。
- ユーザの確認（比較画像）
  - デザインの決まりと色は案のとおりとする。`icons.qrc` は `generate.sh` で作る方式とする。
  - 文書は、日本語版を主として `docs/icons.md`、英語版を `docs/icons.en.md` とする（最初は英語版だけを `docs/icons.md` として書いていた）。
- 検証
  - `./cmake-helper.sh test qt6`、`./cmake-helper.sh test qt5`: 3つのテスト（soccerwindow2_help、check_icons、gui_smoke）がすべて成功。
  - Autotools（作業ツリーのコピーで、Qt6 と Qt5）: `./bootstrap`、`./configure`、`make`、`make check` が成功。rcc は Qt6 で `/usr/lib/qt6/libexec/rcc`、Qt5 で `/usr/lib/qt5/bin/rcc` が見つかった。PNG を更新すると `qrc_icons.cpp` が作り直されることも確かめた。`make distcheck` も成功し、tarball に SVG・PNG・一覧が入ることを確かめた。
  - CMake の `distcheck`（一時的なクローンでコミットして実行）: 成功。
  - `test/check_icons.sh` の異常系（PNG の欠落、大きさの違い、SVG のない PNG、PNG でないファイル、一覧の不一致、SVG の編集後の作り忘れ、パレットにない色、`palette.txt` の変更）を、コピーを壊して確かめた。`palette.awk` は mawk（Ubuntu の既定）と busybox awk で同じ結果になった。

### 段階1: FormationEditorWindow（2026-11-21）

- 実施内容
  - 残りの9個（chase、data_auto_select、delete、insert、replace、reverse、symmetry、sync_ball、train）を描き、段階0 の4個（new、open、record、save）と合わせて13個にした。
  - `palette.txt` に `yellow`（選手）と `field`（フィールドの状態）を加えた。選手は黄、ボールは `paper`（白）、データの操作の矢印は `red`。
  - `formation_editor_window.cpp` の13個、`label_editor_window.cpp` の open、`main_window.cpp` の save（2か所）を `UIIcon::fromName()` に置き換え、使わなくなった XPM の `#include` を外した。XPM のファイルは段階5 まで残す。
  - GUI のテストに `uiIconInWindows`（18行）を加えた。各ウィンドウのアクションのアイコンが、明るい配色・暗い配色でリソースの PNG と同じ画像になることを確かめる。
  - `docs/icons.md` と `docs/icons.en.md` の描き方の決まりに、小さな丸の描き方を加えた。
- デザインの検討
  - 半径2 の `circle` は 16px でぼやけたので、画素にそろえた角丸の四角にした（`rx` を 1、1.5、2 で比べ、1.5 が 16px と 32px の両方で丸く見えた）。
  - sync_ball: 円弧の矢印（1px、1.5px）は 16px でぼやけたので、角を丸めた 2px の輪と三角の矢じりにした（ボールを囲んで回る矢印というモチーフは残した）。
  - train: 3層のネットワークや線が灰色の案は 16px で線が混み合ったので、2-2-1 のネットワーク（節点は 3px の角丸の四角、線は `fg`）にした。
  - symmetry: FormationEditorWindow ではペアの選手を緑で描くが、モチーフを残して黄にした。
  - FormationEditorWindow と LabelEditorWindow のツールバー（24px）でも、アイコンは今と同じく 16px で描かれることを、実際のウィンドウの画像で確かめた。
- ユーザの確認と、insert・replace・reverse の描き直し
  - sync_ball、train、symmetry は案のとおりとする。
  - ユーザの指摘: reverse、insert、replace は、アイコンだけでは何の機能か想像しづらい（元のデザインの時点からの問題）。元のデザインは「データ＝丸」で描いていたため、選手やボールと区別がつかなかった。
  - 機能を確かめた: ReverseY は今のフィールドの状態（ボールと選手）を横の中心線（y=0）について上下に反転する。Insert Data は今のフィールドの状態を今の番号の次に挿入し、Replace Data は今の番号のデータを今のフィールドの状態で置き換える。
  - 学習データの列を「行が並んだリスト」（白い行）、今のフィールドの状態を「緑（フィールドの色）の行」で表す案を作った。A（insert: 緑の行に矢印 → が入る、replace: 緑の行と入れ替えの矢印 ⇄、reverse: 破線の中心線をはさんで黄の三角と反転した白の三角と上下の矢印 ↕。画像編集ソフトの「上下反転」の形）と、B（insert: 挿入位置の線、replace: 回転の矢印、reverse: 選手の反転）などを比べ、ユーザが A に決めた。
  - Add Data（record の赤い丸）は、「今の状態を記録する」として一般的で insert とも見分けやすいので、そのままとした。
- 検証
  - `./cmake-helper.sh test qt6`、`./cmake-helper.sh test qt5`: 3つのテストがすべて成功（GUI のテストは63項目）。
  - 変更前（`41fcd11`）と変更後のソースで本物のウィンドウを作り、ツールバーの画像を明るい配色・暗い配色で比べた。

### 段階2: MainWindow（2026-11-21）

- 実施内容
  - logplayer_*（13個）、debug_server_switch、open_rcg の15個を描いた。
  - ログプレイヤーの記号は、元のアイコンのモチーフ（|◀◀、◀G、|◀、■▶、▶|、▶G、▶▶|、▼−、▲+、■、◀、▶）を残し、三角の高さ 10px・幅 5px（45°）、棒の幅 2px にそろえた。「G」（ゴール）は 2px の線で描いた。
  - logplayer_play_forward（段階0 の見本）は高さ 12px だったが、ログプレイヤーのメニューでほかの記号と並ぶので、高さ 10px に変えた。
  - logplayer_live_mode: 元のアイコンと同じく「LIVE」の文字を画素にそろえて描き、生中継らしく赤（`red`）にした。
  - debug_server_switch: 元のアイコン（オレンジの丸い図形）は何を表すか分かりにくかったので、色（オレンジ）は残して、「デバッグ」を表す虫の絵にした。`palette.txt` に `orange` を加えた。
  - open_rcg: 元のアイコンは open とほぼ同じ絵だったので、フォルダに ▶ の印を付けて「試合のログを開いて再生する」ことを表した（フィールドやボールの印も試したが、旗や虫眼鏡にも見えた）。
  - `main_window.cpp` の15個（open_rcg は Open rcg file と Open draw data の2か所）と、`debug_message_window.cpp` の logplayer_one_step_back・logplayer_one_step_forward を `UIIcon::fromName()` に置き換え、使わなくなった XPM の `#include` を外した。
  - GUI のテスト `uiIconInWindows` に、MainWindow の16個のアクションと DebugMessageWindow の2個のアクションを加えた（合わせて36行）。
  - `docs/icons.md` と `docs/icons.en.md` の描き方の決まりに、ログプレイヤーの記号の大きさを加えた。
- 検証
  - `./cmake-helper.sh test qt6`、`./cmake-helper.sh test qt5`: 3つのテストがすべて成功（GUI のテストは81項目）。
  - 変更前（`41fcd11`）と変更後のソースで、本物の MainWindow と DebugMessageWindow のツールバーの画像を明るい配色・暗い配色で比べた。LIVE とデバッグサーバのボタンは、ライブ接続がない状態では無効なので灰色で表示される。
- ユーザの確認: 比較画像を見て、案のとおりとした。

### 段階3: DebugMessageWindow（2026-11-21）

- 実施内容
  - clear、debug_log_view、debug_view_*（9個）、open_dir、refresh、sync の14個を描いた。これでアイコンは42個になった。
  - デバッグ表示の9個は、元のアイコンと同じく「頭文字（ALL、S、B、T、O、C、L、T、M）＋印」とした。頭文字は LIVE と同じく 2px の線で画素にそろえて描き、印の色をフィールドのデバッグ表示の色に合わせた（自分・ターゲット・図形はオレンジ、ボールはピンク、味方は緑、敵は青）。元のアイコンではボールの印が青だったが、フィールドに合わせてピンクにした。
  - C（コメント）と M（メッセージ）の点は、最初は灰色（`fg2`）にしたが、押下状態のボタン（既定）の背景では薄く見えたので `fg` にした。
  - ユーザの指摘: debug_view_self の「S」が「5」に、debug_view_ball の「B」が「8」に見えることがある。S は角を斜めに落とし、上の端を右下へ、下の端を左上へ巻き込ませて曲線らしくした。B は左側の縦線と角を直角のままにし、上の膨らみを下より 1px 小さくして、右の中央にくびれを付けた。あわせて、debug_view_opponent の「O」も数字の 0 と同じ形だったので、角を 2px 落として丸みを出した。`fromNumber()` の 5・8・0 と並べた比較画像で、区別できることを確かめた。
  - debug_log_view: 赤い四角と青い丸（元のモチーフ）。open_dir: 閉じたフォルダ。refresh: sync_ball と同じ形の輪の矢印を青で（ボールなし）。sync: フィールド（緑の地と白線）。clear: 元は白紙だったので、白紙に赤い × を付けて「データを消す」ことを表した（new と区別するため）。
  - `palette.txt` に `pink`、`green`、`field_line` を加えた。
  - `UIIcon::fromNumber()` を加え、番号のアイコン（32個の XPM）を実行時に描く方式に変えた。色は赤（`red` と同じ色）とした（ユーザが比較画像を見て決めた）。
  - `debug_message_window.cpp` の14個を `UIIcon::fromName()` に、番号のアイコンを `UIIcon::fromNumber()` に置き換え、使わなくなった XPM の `#include`（番号の32個を含む）を外した。
  - GUI のテスト: `uiIconInWindows` に DebugMessageWindow の14個を加え（合わせて50行）、`uiIconFromNumber`（16px と 32px、明暗の色、余白、数字ごとに違う画像、名前のないデバッグレベルのアクションのアイコン）を加えた。
  - `docs/icons.md` と `docs/icons.en.md` に、番号のアイコンと、両方の配色で同じ色にする方法（`field_line`）を加えた。
- 検証
  - `./cmake-helper.sh test qt6`、`./cmake-helper.sh test qt5`: 3つのテストがすべて成功（GUI のテストは96項目）。
  - 変更前（`41fcd11`）と変更後のソースで、本物の DebugMessageWindow のツールバーの画像を明るい配色・暗い配色で比べた。番号のアイコンは、元の XPM、赤、文字色の3通りを並べた比較画像を作った。
- ユーザの確認: S・B の調整の後、比較画像を見て案のとおりとした。番号のアイコンの色は赤とした。
