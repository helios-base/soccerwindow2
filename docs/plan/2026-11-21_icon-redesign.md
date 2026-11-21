# アイコンの刷新（明るい配色用・暗い配色用の2組）計画書

- 作成日: 2026-11-21
- ステータス: 計画中（未着手）
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
- ツールバーやメニューで使っているアイコンは、番号のアイコンを除いて44個、番号のアイコン（`num_1`〜`num_32`）が32個。どれも `UIIcon::fromXpm()` で読み込んでいる。

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
   - 使っているアイコン44個に加え、アプリのアイコン（soccerwindow2、fedit2、soccerwindow2-nostr）もきれいにする。
   - 番号のアイコン（32個）は1つずつ描かず、実行時に配色に合った色で数字を描く。
   - Phase 5-2 で見づらかったもの（FormationEditorWindow の図形のアイコン、sync-ball、open）から始め、ウィンドウごとに段階に分けて置き換える。
   - 段階ごとに、明るい配色・暗い配色での比較画像をユーザが確認してからコミットする。
6. **古いアイコン:** 削除せず、記録・記念として残す。
   - 置き換えが終わった段階（段階5）で、`xpm/` を `git mv` で `icons/legacy/xpm/` に移す。
   - 一覧のページ（`icons/legacy/README.md`）を置き、古いアイコンを並べた画像と、古いアイコンと新しいアイコンの比較画像を載せる。
   - ビルドでは使わず、`.gitattributes` の `export-ignore` で配布用の tarball からは除く。

## 4. 未決事項

| 項目 | 案 |
|---|---|
| ディレクトリ構成 | 「5.1」の案 |
| 色の対応表 | 段階0 でデザインの見本と一緒に決める（「5.2」） |
| 番号のアイコンの色 | 今の赤を引き継ぐか、文字色にするか。段階3 で比較画像を見て決める |
| アプリのアイコンのインストール | 今は `PREFIX/share/soccerwindow2/soccerwindow2.xpm` をインストールしている。新しいアイコン（PNG、SVG）に変えるか、freedesktop の場所（`PREFIX/share/icons/hicolor/...`）にも置くか。段階4 で決める |
| 色を置き換える処理の扱い | すべて置き換えた後、`UIIcon::fromXpm()` と色の置き換え（方式 (a)）を削除する（案）。段階5 で決める |
| 配色の既定値 | 最後（段階6）に、light のままにするか、System にするかを見直す |

## 5. 方針

### 5.1 ディレクトリ構成（案）

```
icons/
  svg/<name>.svg          元の SVG（明るい配色用の色で描く。Inkscape でそのまま開いて編集できる）
  light/<name>.png        明るい配色用（16×16）
  light/<name>@2x.png     明るい配色用（32×32。HiDPI 用）
  dark/<name>.png         暗い配色用（16×16）
  dark/<name>@2x.png      暗い配色用（32×32）
  app/                    アプリのアイコン（複数の大きさ）
  palette.txt             色の対応表（明るい配色用の色 → 暗い配色用の色）
  generate.sh             SVG から PNG を作るスクリプト（Inkscape を使う。実行するコマンドを表示する）
  icons.qrc               Qt のリソースの定義
  legacy/xpm/             古いアイコン（段階5 で xpm/ から移す）
  legacy/README.md        古いアイコンの一覧と、新しいアイコンとの比較
docs/icons.md             アイコンの追加・修正の手順と、.qrc の使い方・書き方
```

- SVG は PNG を作るための「元のファイル」なので、配布用の tarball にも含める（GPL の「改変に適した形式」にあたる）。

### 5.2 デザインの決まり（案。段階0 で見本を作って決める）

- 16×16 の格子（`viewBox="0 0 16 16"`）で描き、16px で画素にそろうよう、線の座標を合わせる。32px は同じ SVG を2倍で出力する。
- 線の太さ、角の丸め、余白（外周の1画素を空けるかなど）を全体で統一する。
- 色は少数の役割（線・前景、補助の灰色、塗り、強調の青・緑・赤・黄など）に限り、`palette.txt` に明るい配色用と暗い配色用の色を対にして書く。
- 元の SVG は明るい配色用の色で描く。`generate.sh` が `palette.txt` に従って色を置き換え、暗い配色用を作る。`palette.txt` にない色が SVG にあればエラーにする（色合いの統一を保つため）。
- 選手・チームを表すアイコン（debug_view_teammate など）は、soccerwindow2 のフィールドの描画色（左チームの黄、右チームの赤など）と意味が揃うようにする。

### 5.3 PNG の作り方

- `icons/generate.sh [NAME...]` で、SVG から明るい用・暗い用の 16px と 32px の PNG を作る（名前を省略するとすべて）。`inkscape --export-type=png --export-filename=... -w 16 -h 16` のように、Inkscape をコマンドラインで使う。
- 作った PNG はコミットする。ビルドと CI では SVG の変換を行わない。

### 5.4 埋め込み方

- `icons/icons.qrc` に PNG を並べ、`:/icons/light/open.png` のようなパスで読み込む。
- CMake: `icons.qrc` を `soccerwindow2_gui` に加え、`AUTORCC` で変換する。`soccerwindow2_gui` は静的ライブラリなので、リソースを使う前に `Q_INIT_RESOURCE( icons )` を呼ぶ必要がある。これを呼ぶ関数（例: `UIIcon::initResources()`）を用意し、`main()` とテストの `main()` から呼ぶ。
- Autotools: `m4/sw2_check_qt.m4` で rcc を探し（moc と同じく、Qt6 は `QT_HOST_LIBEXECS`、Qt5 は `QT_HOST_BINS`。変数 `QT_RCC` で指定もできる）、`src/qt/Makefile.am` に `qrc_icons.cpp` を作るルールを加える。PNG と SVG を `EXTRA_DIST` に加える。
- rcc は、すでに必要な `qt6-base-dev-tools` / `qtbase5-dev-tools` に含まれるので、依存パッケージは増えない。

### 5.5 切り替え方

- `UIIcon` に、名前でアイコンを作る関数（例: `UIIcon::fromName( "open" )`）を加える。
- アイコンのエンジンは、明るい用と暗い用の `QIcon`（それぞれ 16px と 32px の PNG）を持つ。描くたびに `UIIcon::isDark()` で配色を調べ、どちらかの `QIcon` に任せる。
- 無効（Disabled）・選択中（Selected）の見た目は、今と同じくスタイルに作らせる。
- まだ描き直していないアイコンは `UIIcon::fromXpm()`（色の置き換え）のままにし、段階的に置き換える。

### 5.6 番号のアイコン

- DebugMessageWindow の番号のボタン（1〜32）のアイコンは、実行時に `QPainter` で数字を描いて作る（16px と 32px）。配色に応じて色を選ぶ。

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
- `docs/icons.md` を書く（アイコンの追加・修正の手順、SVG の描き方の決まり、`generate.sh` の使い方、`.qrc` の書き方と使い方、CMake と Autotools での扱い）。
- デザインの見本として、数個（例: open、save、logplayer_play_forward）を描き、線の太さ・色の対応表をユーザと決める。
- テストを加える（「7. テスト」）。

### 段階1: FormationEditorWindow（13個）

- chase、data_auto_select、delete、insert、new、open、record、replace、reverse、save、symmetry、sync_ball、train。
- open と save は LabelEditorWindow・MainWindow でも使っているので、ここで置き換わる。

### 段階2: MainWindow（14個）

- logplayer_*（13個）、debug_server_switch、open_rcg。

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
  - `icons/svg/` の SVG ごとに、明るい用・暗い用の 16px と 32px の PNG があること。
  - PNG の大きさが正しいこと、`icons.qrc` に載っている PNG と実際のファイルが一致すること。
  - SVG に、`palette.txt` にない色が使われていないこと。
- GUI のテスト（`test/test_gui_smoke.cpp`）
  - 名前で作ったアイコンが、16px と 32px のどちらでも作れること（リソースが埋め込まれていること）。
  - 明るい配色と暗い配色で、違う画像が使われること。配色を切り替えると、ツールバーのアイコンが変わること。
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
