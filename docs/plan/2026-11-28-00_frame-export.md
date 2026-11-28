# 2D 表示の画像と動画の出力 (rcssmonitor からの移植) 計画

| 項目 | 内容 |
|---|---|
| 作成日 | 2026-11-28 |
| 最終更新日 | 2026-11-28 |
| 対象 | `develop` (`6f2b7ea`) |
| ブランチ | `feature/frame-export`。`develop` の `6f2b7ea` から、ユーザが GitHub で作った |
| 計画 ID | SW2-01 |
| 計画 Issue | https://github.com/helios-base/soccerwindow2/issues/100 |
| 関連 | 移植の元: rcssmonitor の PR https://github.com/rcsoccersim/rcssmonitor/pull/76 (rcssmonitor の `develop` に `f2898ed` で取り込み済み)。その計画書 https://github.com/rcsoccersim/rcssmonitor/blob/develop/docs/plan/2026-11-22-03_frame-export.md と、マニュアル https://github.com/rcsoccersim/rcssmonitor/blob/develop/docs/export.md |

## Phase と Issue

| Phase | 内容 | 実装の記録 | Issue |
|---|---|---|---|
| 1 | 1 枚の画像の保存と、描画の状態の保存と復元 (§4.1。D5、S3〜S5、S16) | 4.1.2 (2026-11-28) | https://github.com/helios-base/soccerwindow2/issues/101 |
| 2 | 範囲と連番画像の基本、出力のダイアログ (§4.2。D3、D7、D8、S7〜S11、S17) | 4.2.2 (2026-11-28) | https://github.com/helios-base/soccerwindow2/issues/102 |
| 3 | 範囲の便利な指定: 印、スライダーの帯、よく使う範囲、複数の範囲 (§4.3。D9〜D12、S12、S18) | 4.3.2 (2026-11-28) | https://github.com/helios-base/soccerwindow2/issues/103 |
| 4 | ffmpeg による動画 (§4.4。S15) | — | https://github.com/helios-base/soccerwindow2/issues/104 |
| 5 | コマンドラインからの出力 (§4.5。D4、S13、S14) | — | https://github.com/helios-base/soccerwindow2/issues/105 |
| 6 | 出力のマニュアル `docs/export.md` (§4.6) | — | https://github.com/helios-base/soccerwindow2/issues/106 |

## 1. 目的と方針

- rcssmonitor に PR #76 で入った画像と動画の出力を、同じ機能と UI で soccerwindow2 に入れる (D1)。soccerwindow2 には 3D 表示がないので、2D 表示の出力だけにする (D2)。
- 入れるもの: 今のフレームの 1 枚の保存 (File > Save Image of This Frame)、範囲を選んで連番画像か動画にする出力のダイアログ (File > Export Images / Video)、メインの窓での範囲の印とスライダーの帯、よく使う範囲 (区間、得点の場面、今の位置の前後)、複数の範囲、外部の ffmpeg による動画、窓を出さないコマンドラインからの出力、出力のマニュアル。
- rcssmonitor と同じく、**オフラインで描く**: 再生しながら画面を録るのではなく、出力の各コマを、フレームの番号と固定の時間の刻みから描き直す。機械の速さによらず、同じ結果になる。
- 「画面のとおり」: 出力の論理の大きさを画面に近く保ち、解像度は devicePixelRatio (dpr) で変える。フォーカス、ズーム、文字やスコアボードの大きさ、デバッグの重ね描きが、画面と同じ見た目になる。
- 移植は、rcssmonitor の実装の 2D の部分を手本にし、soccerwindow2 の構造 (`MainData`、`ViewHolder`、`src/model/options`) に合わせて書き直す (S1)。使い方と画面の文言は rcssmonitor にそろえる (S2)。

## 2. 現状

### 2.1 移植の元 (rcssmonitor、`f2898ed`)

rcssmonitor の PR #76 (2026-11-25 に取り込み) は、次を入れた。段階 5、6 (3D の描画の整理と 3D のオフスクリーンの描画) と、見送った段階 9 (Qt Multimedia の試作) を除くと、2D の出力は次のクラスでできている。

| rcssmonitor のクラス | 役割 |
|---|---|
| `Options::ViewState`、`FrameScope` | 描くときに変わる大域の状態 (尺度、中心、キャンバスの大きさ、フォーカス点、選択の番号、今の番号) の保存と復元 |
| `FieldCanvas::renderFrame()` | 任意のフレームを、任意の大きさと dpr の `QImage` に描く |
| `ExportTiming` | 出力のコマの数、各コマのフレーム、動画の長さ、画像の名前 (純粋な関数) |
| `ExportRange`、`ExportRangeList` | 位置の書き方 (`1234`、`1234.3`)、範囲の始めと終わりの番号、区間、得点、前後、範囲の一覧 |
| `FrameSink`、`ImageSequenceSink`、`FfmpegSink` | 出力先 (連番画像、ffmpeg の標準入力への生のフレーム) |
| `ExportJob` | イベントループの中で 1 コマずつ描いて出力先に渡す |
| `ExportDialog` | モードレスの出力のダイアログ |
| `LogPlayerSlider` | スライダーの範囲の帯と得点の印 |
| `ExportBatch` | 窓を出さないコマンドラインからの出力 |

ユーザの決定 (rcssmonitor の計画の D4〜D20) のうち、2D にかかわるものは次のとおり。

- rcssmonitor の D5、D6: 1 枚の保存は File > Save Image of This Frame... (Ctrl+Shift+I)、範囲の出力は File > Export Images / Video... (Ctrl+Shift+E) の別の項目にする。
- rcssmonitor の D7、D11、D12: 範囲の印は `[` と `]`。File > Export Images / Video の中に、Open the Export Dialog...、Start the Range Here、End the Range Here、Clear the Range をまとめる。ダイアログの Clear と、Now のツールチップ。
- rcssmonitor の D8、D9、D10: 得点の場面は得点の 10 秒前から 5 秒後、今の位置の前後は 5 秒前から 5 秒後。スライダーに得点の印も出す。得点の位置は、ログの再生の「次の得点へ」と同じ番号にそろえる。
- rcssmonitor の D13〜D16: 動画を途中で止めたらファイルを消す。動画の設定は画質 (CRF、既定 18) と圧縮の速さ (preset、既定 medium) だけ。「Enlarge the texts, logos and lines with the image」の名前と動き。
- rcssmonitor の D18: 出力のマニュアル `docs/export.md`。

### 2.2 soccerwindow2 の 2D 表示 (`6f2b7ea`)

- `FieldCanvas` (`src/qt/field_canvas.{h,cpp}`) は `QWidget`。`paintEvent()` は、`MainData::update( width(), height() )` を呼んでから、マウスの座標の変換 `M_transform` を作り、`draw( QPainter & )` を呼ぶ。`draw()` は `FieldPainter` の後に、描き手の列 (`M_painters`) を順に呼ぶ。最後に、右ボタンでの計測の線を描く。
- 描き手の列は、表示の形 (`Options::PaintStyle`。既定と rcssmonitor 風) で 2 とおり。チームのロゴ、ボールと選手の軌跡、選手、ボール、オフサイドライン、ボロノイ図、デバッグサーバの表示 (`DebugPainter`)、エージェントのデバッグログ (`DebugLogPainter`)、描画データ (`DrawDataPainter`)、特徴量のログ、スコアボード、マークの割り当て。フォーメーションエディタのモードでは、その描き手も描く。
- `MainData::update()` (`src/model/main_data.cpp`) は、選手の自動の選択、フォーカス点、フォーメーションエディタのボールの同期 (`feditBallSyncMove`)、`Options::updateFieldSize()` (キャンバスの大きさ、スコアボードの高さ、尺度、フィールドの中心) を行う。
- `Options::updateFieldSize()` (`src/model/options.cpp`) は、スコアボードの表示の有無と表示の形を、関数の中の static 変数 (`s_score_board`、`s_paint_style`) に覚えている。rcssmonitor では、同じ static 変数のために、別のフレームを描いた後に戻した画面の尺度が古いまま残る不具合が見つかった (rcssmonitor の計画 4.10)。
- フレームの列は `ViewHolder::monitorViewCont()`、今の番号は `MainData::viewIndex()`。番号は `MainData::setViewDataIndex()` で変える (同じ番号と範囲の外は受け付けない)。サイクルから番号へは `ViewHolder::getIndexOf( cycle )` (そのサイクルの最初のフレーム)。
- 得点の変わり目は `ViewHolder::scoreChangeIndices()` (チームの記録が届いたときのフレームの数を覚える)。ログの再生の「次の得点へ」(G、Ctrl+G) が使う。
- エージェントのデバッグログは、デバッグメッセージの窓で同期の操作をしたときに `MainData::seekDebugLogData()` でその時刻のデータを読み、`DebugLogPainter` は、今のフレームの時刻がそのデータの時刻と同じときだけ描く。ほかのデバッグの重ね描き (デバッグサーバの表示、描画データ、特徴量のログ、マークの割り当て) は、時刻ごとのデータを今のフレームの時刻で引く。

### 2.3 今の画像の保存

- File > Save Image (`ImageSaveDialog`、`src/qt/image_save_dialog.{h,cpp}`) は、モーダルのダイアログで、開くと再生を止める。大きさ (幅と高さ)、始めと終わりのサイクル (Select All)、名前の頭、形式 (`QImageWriter::supportedImageFormats()`)、ディレクトリを選ぶ。ショートカットはない。
- 終わりのサイクルも `getIndexOf()` で番号にするので、終わりのサイクルの止まっているフレーム (2 つ目以降) は含まれない。名前はログの番号 (`image-00042.png`)。同じ名前のファイルがあると、1 枚ごとに上書きを尋ねる。描くときに `MainData` の今の番号と `Options` の状態を変え、今の番号だけを戻す。
- コマンドラインの `--auto-image-save` は、起動の 500 ms 後に `ImageSaveDialog` でログ全体を保存して終わる (窓を出す)。`--image-save-dir`、`--image-name-prefix`、`--image-save-format` がダイアログの初めの値になる。

### 2.4 再生のスライダー、ショートカット、設定

- スライダーは、`src/qt/log_player_tool_bar.cpp` の中のクラス `CycleSlider` (`QSlider`)。ツールバーの向きに合わせて横にも縦にもなる。
- ショートカットは `MainWindow` で決め、ユーザがショートカットの窓で変えられる (`~/.soccerwindow2` の群 `Shortcut`。アクションの `objectName` で引く)。Ctrl+Shift+I、Ctrl+Shift+E、`[`、`]` は使われていない (`src/qt/*.cpp` の `setShortcut` で確かめた)。
- 設定は `~/.soccerwindow2` (`QSettings` の INI の形式) に、群 `Options`、`Shortcut` などとして書く。
- コマンドラインは librcsc の `rcsc::CmdLineParser` と `rcsc::ParamMap` で解析する。`ParamMap` の値は 1 つだけで (`int`、`double`、`bool`、`std::string` など)、同じオプションを繰り返すと最後の値が残る (librcsc の `rcsc/param/cmd_line_parser.cpp` で確かめた)。rcssmonitor は `QCommandLineParser` で、`--export-range` を何度でも指定できる。

### 2.5 テストと CI

- CI (`.github/workflows/build.yml`) は、ubuntu-24.04 と ubuntu-26.04、Qt5 と Qt6、CMake と Autotools の 8 つのジョブ。Qt Test のテストは CMake のジョブだけで走る (Autotools の `make check` はシェルのテストだけ)。
- Qt Test のテストは `test/CMakeLists.txt` の `soccerwindow2_add_qt_test()` で足す。ゲームログのテストデータは `test/data/` (`head_v5.rcg`、`head_v6.rcg`、`head_json.rcg`)。
- `soccerwindow2` を起動すると `~/.soccerwindow2` を書き直すことがあるので、GUI のテストは HOME を一時ディレクトリにしている (`test_gui_smoke.cpp` など)。

### 2.6 移植で変わるところ

| rcssmonitor | soccerwindow2 |
|---|---|
| `DispHolder` (フレームと今の番号) | `ViewHolder` (フレーム) と `MainData` (今の番号) |
| `DispHolder::getIndex( cycle )` | `ViewHolder::getIndexOf( cycle )` |
| `DispHolder::scoreChangedIndex()` | `ViewHolder::scoreChangeIndices()` (指す番号が同じかを Phase 3 で確かめる) |
| `FieldCanvas::updateFocus()` と `Options::updateFieldSize()` | `MainData::update()` (フォーメーションエディタの同期を含む) |
| `LogPlayerSlider` (独立したファイル、横だけ) | `CycleSlider` (`log_player_tool_bar.cpp` の中、横と縦) |
| `QCommandLineParser` | `rcsc::CmdLineParser` と `rcsc::ParamMap` |
| バッチのログの読み込み (`RCGHandler( nullptr, … )`) | `MainData::openRCG()` |
| 2D と 3D の描き手 (`ExportRenderer`) | 2D だけ |
| 描き手は試合の表示だけ | デバッグの重ね描き、軌跡、フォーメーションエディタも描く |
| CI の 5 つのジョブ | CI の 8 つのジョブ (Qt のテストは CMake の 4 つ) |

## 3. 決定事項

### 3.1 ユーザの決定

| # | 内容 | 決めた日 |
|---|---|---|
| D1 | rcssmonitor に PR #76 で入ったエクスポートの機能と UI を、同じように soccerwindow2 に入れる | 2026-11-28 (計画の作成の前) |
| D2 | soccerwindow2 には 3D 表示がないので、2D 表示の出力だけでよい | 2026-11-28 (計画の作成の前) |
| D3 | 今の File > Save Image (`ImageSaveDialog`) は、Phase 2 で出力のダイアログに置き換えて消す。範囲、大きさ、名前の頭、形式、ディレクトリは、出力のダイアログにすべてある (Claude の案のとおり) | 2026-11-28 (計画の確定の前。初めは Phase 2 の初めに決める予定だった) |
| D4 | 今の `--auto-image-save` は、Phase 5 で新しいバッチの仕組みに移し、ログ全体を連番画像で出して終わる動きを保つ。窓は出さない。`--image-save-dir`、`--image-name-prefix`、`--image-save-format` の値を使う (Claude の案のとおり。大きさなどの細部は Phase 5 で決める) | 2026-11-28 (計画の確定の前。初めは Phase 5 の初めに決める予定だった) |
| D5 | 画像の文字は、灰色のアンチエイリアスで描く (rcssmonitor と同じ)。画面はアンチエイリアスなしのまま。Claude の案は「画面と同じく切る」(文字も画面と画素まで一致する) だった。テストは文字を消して画面と比べる | 2026-11-28 (Phase 1 の初め) |
| D6 | CI は `develop` と `master` へのプッシュと PR のときだけ動くので、Phase をプッシュした後に、Claude が `gh workflow run build.yml --ref feature/frame-export` で手動で走らせて見張る。`build.yml` は変えない | 2026-11-28 (Phase 1 の初め) |
| D7 | `--image-save-dir`、`--image-name-prefix`、`--image-save-format` は、コマンドラインで指定したときだけ、その起動の間、保存した出力の設定より優先して出力のダイアログの初めの値にする (`--ui-color-scheme` と同じ考え方)。出力すると、使った値が保存される (Claude の案のとおり) | 2026-11-28 (Phase 2 の初め) |
| D8 | ソースの著作権表示は `Copyright (C) Hidehisa AKIYAMA` だけにし、`The RoboCup Soccer Server Maintenance Group.` を書かない。rcssmonitor から移したファイルも同じ。GPL の第 3 版以降は S17 のまま (S17 の案を変えた) | 2026-11-28 (Phase 2 の確認) |
| D9 | 範囲の始めと終わりの印のキーは `[` と `]` にする。印を消す操作はメニューだけ (rcssmonitor と同じ。Claude の案のとおり) | 2026-11-28 (Phase 3 の初め) |
| D10 | よく使う範囲の既定の秒数は、得点の場面が得点の 10 秒前から 5 秒後、今の位置の前後が 5 秒前から 5 秒後 (rcssmonitor と同じ。Claude の案のとおり)。ダイアログで変えられ、設定に保存する | 2026-11-28 (Phase 3 の初め) |
| D11 | 再生のスライダーに、選んだ範囲の帯のほかに、得点の位置の印も出す (rcssmonitor と同じ。Claude の案のとおり) | 2026-11-28 (Phase 3 の初め) |
| D12 | よく使う範囲 (得点の場面と今の位置の前後) の秒数は、再生の時間で数える: 中心のフレームから前後のフレームの数で取り (止まったフレームも 1 ステップ)、得点の場面は得点のフレームから数える。得点の後の止まったフレーム (キックオフの前の 50 フレーム) が「後」の秒数に入る。rcssmonitor はサイクルで数えていて、「5 秒後」がキックオフから 5 秒になっていた (ユーザが画面で気づいた。Claude の案 A) | 2026-11-28 (Phase 3 の確認) |
| D13 | rcssmonitor にも D12 と同じ直しが要る。初めは rcssmonitor のルール (英語、rcsoccersim の Project の 4、タイトル `[SW2-01] Follow-up: …`) で Issue を作ることにしたが、案を見た後、Claude は作らずにユーザが後で対応することにした (`docs/todo.md` に記録した) | 2026-11-28 (Phase 3 の確認) |

### 3.2 設計上の判断 (Claude)

計画の確定のときに、ユーザが確かめる。

| # | 内容 | 理由 |
|---|---|---|
| S1 | rcssmonitor の実装 (`f2898ed`) の 2D の部分を手本にし、soccerwindow2 の構造に合わせて書き直す。クラスとファイルの名前は rcssmonitor にそろえる | 同じ作者の、確かめられた設計とテストを使える。対応が取りやすく、片方の直しをもう片方に移しやすい |
| S2 | 使い方は rcssmonitor にそろえる: メニューの名前と構成、ショートカット (Ctrl+Shift+I、Ctrl+Shift+E、`[`、`]`)、ダイアログの欄と文言、範囲の書き方、ファイルの名前の決め方、設定の既定値、ffmpeg の引数、コマンドラインのオプションの名前。rcssmonitor のユーザの決定のうち 2D にかかわるもの (2.1) を引き継ぐ。ただし、3D の選択 (ダイアログの View、`--export-view`) は置かない | 両方を使う人が迷わない。マニュアルをほぼ共通にできる。キーは soccerwindow2 で空いている (2.4) |
| S3 | オフラインで描く。サーバに接続している間は範囲の出力を無効にし、切断した後に残ったフレームは出力できる。1 枚の保存は接続している間も使える | rcssmonitor の S1。1 枚の保存は、結果が機械の速さによらないという理由に当たらない |
| S4 | `Options::ViewState` (新規) と `FrameScope` (新規) で、`MainData` の今の番号と、`Options` の尺度、フィールドの中心、キャンバスの大きさ、スコアボードの高さ、フォーカス点、選択の番号を、1 コマごとに保存して戻す。`Options::updateFieldSize()` の static 変数をメンバにして `ViewState` に入れる | 画面の状態を壊さない。soccerwindow2 にも rcssmonitor で不具合になった static 変数がある (2.2) |
| S5 | 出力のために描くときは、`MainData::update()` のうちフォーメーションエディタのボールの同期を行わない (`update()` を、描画の状態の更新と同期に分ける) | 出力で、編集中のフォーメーションのデータを変えない |
| S6 | エージェントのデバッグログ (`DebugLogHolder`) の重ね描きは、画面と同じく、同期したフレームでだけ描く。出力のコマごとには追従させない (§8、`docs/todo.md`)。ほかのデバッグの重ね描き、軌跡は、時刻ごとのデータなので、そのままコマごとに描かれる | 画面でも、再生中は同期の操作をしないと追従しない (2.2)。追従させると、ログのファイルの読み直しと、デバッグメッセージの窓の状態の変更が要る |
| S7 | 描き手の型 (rcssmonitor の `ExportRenderer`) は作らず、`ExportJob` は `FieldCanvas::renderFrame()` を直接呼ぶ | 3D がない (D2)。rcssmonitor でも 3D のために後から足したもの |
| S8 | 出力のダイアログはモードレスにし、開いたまま再生の位置を動かして範囲を取れるようにする。出力している間だけ、進捗のダイアログをアプリケーションのモーダルにする | rcssmonitor の S8。今の `ImageSaveDialog` はモーダルで、範囲を決めるのにサイクルを先に調べておく必要がある (2.3) |
| S9 | 位置の書き方は rcssmonitor と同じ `1234` と `1234.3` にする。soccerwindow2 のスコアボードの書き方 `1234,3` も受け付ける。終わりにサイクルを書くと、そのサイクルの止まっているフレームまで含む | 画面に出ている書き方をそのまま入れられる。今の `ImageSaveDialog` では、終わりのサイクルの止まっているフレームが入らない (2.3) |
| S10 | 出力の設定は `~/.soccerwindow2` の新しい群 `Export` に、出力を始めるときに書く | 今の設定と同じファイル。rcssmonitor の S7 |
| S11 | 新しいクラスは `src/qt/` (ライブラリ `soccerwindow2_gui`) に置く。純粋な関数のクラス (`ExportTiming`、`ExportRange`) も同じ。Autotools の `src/qt/Makefile.am` にも足す (moc のファイルを含む) | テストは `soccerwindow2_gui` にリンクする。rcssmonitor と同じ並びにする |
| S12 | スライダーの `CycleSlider` を `src/qt/log_player_slider.{h,cpp}` の `LogPlayerSlider` に切り出し、範囲の帯と得点の印を描く。縦の向きでも描く | テストできるようにする。soccerwindow2 のスライダーは縦にもなる (2.4) |
| S13 | `--export-range` は rcssmonitor と同じく何度でも指定できるようにする。`ParamMap` には、ヘルプに出し、解析の誤りにしないために登録し、値は `ExportBatch` が引数の列から全部集める | rcssmonitor と同じ書き方を使える。librcsc を変えずに済む (2.4) |
| S14 | バッチ (`--export-video` か `--export-images`) では、アプリケーションを作る前にプラットフォームを offscreen にし、窓を作らずに `MainData::openRCG()` でログを読み、`~/.soccerwindow2` に何も書かない。既定は決まった値 (1280×720、尺度は高さ ÷ 720、10 images/s、速さ 1、CRF 18、preset medium、名前の頭 `image-`、PNG、ffmpeg は PATH) にし、ダイアログの設定は使わない。あるファイルは尋ねずに上書きする | rcssmonitor の S16、S17、S19。ディスプレイのない計算機でも動く。スクリプトの結果がダイアログの操作に左右されない |
| S15 | CI: CMake の Qt6 のジョブ (ubuntu-24.04 と ubuntu-26.04) に ffmpeg を入れ、`SOCCERWINDOW2_REQUIRE_FFMPEG=1` で本物の ffmpeg のテストを必ず走らせる。ほかのジョブでは、ffmpeg がなければ飛ばす | rcssmonitor と同じ。偽の ffmpeg のテストはすべての CMake のジョブで走る |
| S16 | 新しいテストは、HOME と XDG_CONFIG_HOME を一時ディレクトリにする | `~/.soccerwindow2` に書かない (2.5) |
| S17 | rcssmonitor から移したファイルには、rcssmonitor の著作権表示 (`The RoboCup Soccer Server Maintenance Group.` と `Hidehisa AKIYAMA`) と、GPL の第 3 版以降を書く。Phase 1 の `frame_scope.{h,cpp}` と `test_field_canvas_render.cpp` (第 2 版以降にしていた) も、Phase 2 で直した (Phase 2 で決めた)。著作権表示は、ユーザの指示で `Hidehisa AKIYAMA` だけにした (D8)。GPL の第 3 版以降はこのまま | 移す元のファイルが GPL の第 3 版以降。soccerwindow2 の `COPYING` も第 3 版 |
| S18 | テストのために `MainWindow` の public のメンバを増やさない。テストは子のオブジェクトを `findChild()` (と `objectName`) で取り、窓に依らない処理は純粋な関数 (`FrameImage` など) に出して直接確かめる。ゲームログを開く `openRCG( const QString & )` だけは、File > Open、ドロップ、コマンドラインと同じ入口として public に残す (Phase 3 で決めた。ユーザの指摘と決定) | public のメンバが増えると、開発者がどこから使われているかを確かめる手間が増える |

### 3.3 Phase の初めに決めること

- Phase 1: 画像の文字のアンチエイリアス (決めた: D5)。
- Phase 2: `--image-save-dir`、`--image-name-prefix`、`--image-save-format` の値を、出力のダイアログの初めの値に使うか (決めた: D7。指定したときだけ優先する)。今の Save Image の扱いは D3 で決めた。
- Phase 3: 範囲の印のキー、得点の場面と今の位置の前後の既定の秒数、スライダーの得点の印を出すか (決めた: D9、D10、D11)。
- Phase 5: `--auto-image-save` (D4) の画像の大きさと名前の決め方 (案: 大きさは窓の大きさのオプション、名前は今と同じログの番号)。
- Phase 6: マニュアルの言語 (案: 英語の `docs/export.md` だけ。rcssmonitor と同じ。`docs/icons.md` のように日本語版も作るか)。

## 4. 設計

各 Phase の実装の記録 (`#### 4.N.2 実装 (YYYY-MM-DD)`) は、その Phase の実装と同じコミットで足す。

他のリポジトリの作業は要らない。依存する librcsc は変えず (S13)、soccerwindow2 に依存するリポジトリへの影響もないので、追従 Issue は作らない。

### 4.1 Phase 1: 1 枚の画像の保存と、描画の状態の保存と復元

#### 4.1.1 設計

- `Options::ViewState` (新規、`src/model/options.h`): 尺度、フィールドの中心、キャンバスの幅と高さ、スコアボードの高さ、フォーカス点、選択の番号、尺度を合わせたときのスコアボードの表示の有無と表示の形 (今の static 変数をメンバにしたもの)。`viewState()`、`setViewState()`。ズームの有無とフォーカスの種類は、描くときに変わらないので含めない (S4)。
- `MainData::update()` を、描画の状態の更新 (選手の自動の選択、フォーカス点、`updateFieldSize()`) と、フォーメーションエディタのボールの同期に分ける。`paintEvent()` は今までどおり両方を呼び、出力は前者だけを呼ぶ (S5)。
- `src/qt/frame_scope.{h,cpp}` (新規): RAII のガード `FrameScope`。作るときに `MainData` の今の番号と `Options::viewState()` を覚えて番号を変え、消えるときに両方を戻す。フレームの外の番号は無視する。
- `FieldCanvas::renderFrame( index, pixels, dpr )` (新規、public): `QImage( pixels, Format_RGB32 )` をフィールドの色で塗り、dpr を設定し、`FrameScope` を開き、論理の大きさで描画の状態を更新して `draw()` を呼ぶ。
  - QPainter の window と viewport を論理の大きさで明示する。dpr が 1 より小さいときは、画像の dpr を 1 にし、論理の window を画像の画素の viewport に写す (rcssmonitor の S15。QPainter は 1 より小さい dpr を 1 として描く)。
  - ズームしているときは、論理の高さの比で尺度を合わせ、画面と同じ範囲を写す。
  - 描画のヒントは画面に合わせる (文字は §3.3)。右ボタンの計測の線は描かない。マウスの座標の変換 `M_transform` は変えない。
  - 描き手が論理の大きさを `painter.window()`、`painter.viewport()`、`Options::canvasWidth()` のどれで取るかを調べ、どれでも画面と同じ配置になるようにする。
- `MainWindow`: File > Save Image of This Frame... (Ctrl+Shift+I、`objectName` は `save_image`)。
  - 選んだときの今のフレームを先に画像にしてから、ファイルのダイアログを出す (ダイアログの間も再生が進むため)。大きさは、2D 表示の大きさ × 画面の dpr。
  - 既定の名前は、ログの名前 (`.rcg`、`.rcg.gz` を除く。ライブでは `soccerwindow2`) とサイクル (止まっているときは `1234.3`)。拡張子がなければ PNG。形式は拡張子で決まる。保存したディレクトリは、起動している間だけ覚える。
  - フレームがあれば有効 (ライブの間も)。フレームがなければ、ステータスバーに知らせる。
  - File のメニューの、今の Save Image の前に置く (今の Save Image は Phase 2 で置き換える。D3)。
- `RELEASE_NOTES.md` に日付つきで足す。

#### 4.1.2 実装 (2026-11-28)

4.1.1 のとおり実装した。計画から決めたことも書く。

- `Options::ViewState` (`src/model/options.h`): 尺度、フィールドの中心、キャンバスの幅と高さ、スコアボードの高さ、尺度を合わせたときのスコアボードの表示の有無と表示の形、フォーカス点、選択の番号。`viewState()` と `setViewState()`。`updateFieldSize()` の static 変数 `s_score_board`、`s_paint_style` を、メンバ `M_sized_with_score_board`、`M_sized_paint_style` にした (初めの値は static 変数と同じ `true` と `PAINT_DEFAULT`)。
- `MainData::updateViewState( width, height )` (新規): 選手の自動の選択、フォーカス点、`updateFieldSize()`。`MainData::update()` は、これと `syncFormationEditBall()` (新規、private。フォーメーションエディタのボールの同期) を呼ぶ。同期の順は `updateFieldSize()` の前から後に変わったが、互いに関係しない。
- `FrameScope` (`src/qt/frame_scope.{h,cpp}`): 作るときに `MainData::viewIndex()` と `Options::viewState()` を覚え、フレームの中の番号なら `setViewDataIndex()` で変える。消えるときに両方を戻す。`setViewDataIndex()` はシグナルを出さないので、ほかの窓は動かない。
- `FieldCanvas::renderFrame( index, pixels, dpr )`: 4.1.1 のとおり。画像はフィールドの色 (`DrawConfig::fieldBrush()`) で塗る。描画のヒントは、図形のアンチエイリアスを画面と同じく `Options::antiAliasing()` に従わせ、文字のアンチエイリアスを入れる (D5)。`draw()` が変える `M_redraw_all` は戻す。
- 描き手を調べた: 配置は `painter.window()` (`FieldPainter`、`DebugPainter`) と `Options` のキャンバスの幅とスコアボードの高さ (`ScoreBoardPainter`、`TeamGraphicPainter`) で決まる。フレームをまたぐ状態を持つ描き手はない (`ScoreBoardPainter` の `M_last_*` は使われていない)。`FormationEditorPainter` だけが `painter.viewport()` を使っていたので、`painter.window()` に直した (画面では同じ値。画面より小さい画像ではずれる)。
- `MainWindow`:
  - File > Save Image of This Frame... (Ctrl+Shift+I、`save_image`)。File のメニューの今の Save Image の前に置いた。いつも有効で、フレームがなければステータスバーに「There is no frame to save.」を出す。
  - `renderImage()`: 2D 表示の大きさ × `devicePixelRatioF()` の画像。`defaultImageName()`: ログのファイルの名前 (`.gz`、`.rcg` を除く) とサイクル。サーバから受けたフレーム (接続のときに名前を消す) と匿名モード (`--anonymous-mode`。ログの名前にはチームの名前が入る) では `soccerwindow2`。`saveImage( path, error )` (公開、テストのため) と、ファイルのダイアログを出す `saveImage()`。ディレクトリの初めは最後にログを開いたディレクトリ (なければホーム) で、保存したディレクトリを起動している間だけ覚える。
  - テストからログを開くために、`openRCG( const QString & )` を public にした。
- テスト `field_canvas_render` (新規。HOME などを一時ディレクトリにし、offscreen で動かす): 画面と同じ画像 (文字を消して `QWidget::render()` とも `grab()` とも画素で一致。文字は選手の背番号、種類、スタミナ、スタミナの容量、スコアボード)、別のフレーム、範囲の外の番号、状態が戻る (ボールのフォーカス、ズーム、自動の選択、`FrameScope` だけでも)、フォーメーションエディタのボールが動かない (画面の `update()` では動く)、dpr 2 と 0.5 (スコアボードとゴールラインの位置)、ズーム、空の大きさ、メニューの項目 (ショートカット、名前、File のメニュー、フレームがないとき、既定の名前、画面と同じ画像、PNG と JPEG、書けない場所)。画像が違うときは、違う画素の数と範囲を出す (`SOCCERWINDOW2_TEST_DUMP_DIR` を決めると、2 つの画像も書き出す)。
- 確かめたこと (2026-11-28):
  - CMake の Qt 6.4.2 と Qt 5.15.13 のビルドで、警告なしにビルドでき、`ctest` のテスト 11 個 (新しい `field_canvas_render` の 10 の試験を含む。`monitor_client_live` は飛ばす) がすべて通った。`gui_smoke` のショートカットの重なりの試験も通った。
  - Autotools (Qt 5) で、作業ツリーの写しを `./bootstrap`、`./configure --with-qt=5` からビルドでき (警告は前からある `configure.ac` の 1 つだけ)、`make check` の 3 つと `make distcheck` が通った。配布物に `frame_scope.{h,cpp}` と、`EXTRA_DIST` に足した `test/test_field_canvas_render.cpp` が入る。
  - 実際の試合のログ (`202607050404-HELIOS2026_0-vs-YuShan2026_1.rcg.gz`) のサイクル 1000 を、1000×650 の画面の大きさで dpr 1、2、0.5 と、1920×1080 で画像にして見た。dpr 2 は同じ配置で文字とチームのロゴが 2 倍の解像度、dpr 0.5 は同じ配置で半分の大きさになり、スコアボードは上にあった。

### 4.2 Phase 2: 範囲と連番画像の基本、出力のダイアログ

#### 4.2.1 設計

rcssmonitor の段階 2 (rcssmonitor の計画 4.3〜4.6、4.11) を移す。

- `src/qt/export_timing.{h,cpp}` (新規、純粋な関数): 1 コマで進むフレームの数 `per_image = speed × 1000 / (step_ms × fps)`、コマの数 `N = floor( (last − first) / per_image + ε ) + 1`、コマ n のフレーム `min( last, floor( first + n × per_image + ε ) )` (足し算を重ねず n から計算する)、動画の長さ `N / fps`、画像の名前 `PREFIX%0Wd.ext` (`W = max( 5, N−1 の桁数 )`)。既定は 10 images/s、速さ 1 (1 フレームが 1 コマ)。ステップはログのサーバのパラメータ (`simulator_step`)。
- `src/qt/export_range.{h,cpp}` (新規、純粋な関数): `ExportPosition` (サイクルと止まった番号) と `ExportRange`。位置の解釈と書き出し (S9)、`MonitorViewData::Cont` に対する始めと終わりの番号 (始めはその位置以後の最初のフレーム、終わりはその位置以前の最後のフレーム。ないときは寄せて、寄せたことを返す)、試合の時刻。
- `src/qt/frame_sink.{h,cpp}` (新規): インタフェース `FrameSink` (`open`、`write` の ACCEPTED / BUSY / FAILED、`finish`、`abort`、シグナル `readyForFrame()`、`finished( ok, message )`)。
- `src/qt/image_sequence_sink.{h,cpp}` (新規): ディレクトリ (なければ作る) に、与えた名前で 1 枚ずつ `QImageWriter` で書く。形式は PNG と JPEG。中止したとき、書いた画像は残す。
- `src/qt/export_job.{h,cpp}` (新規): イベントループの中で 1 コマずつ `FieldCanvas::renderFrame()` で描いて出力先に渡す (S7)。BUSY なら `readyForFrame` を待つ。進み具合と終わりのシグナル。出力の前に再生を止める。
- `src/qt/export_dialog.{h,cpp}` (新規): `ExportDialog` (モードレス。S8)。
  - 範囲: 始めと終わりの欄 (空なら最初と最後のフレーム)、Now (今の位置)、Clear。欄の右に、寄せた先の位置と試合の時刻、誤りを出す。範囲はこの Phase では 1 つ。
  - 画像: 大きさ (As the view、1280×720、1920×1080、3840×2160、任意)。「Enlarge the texts, logos and lines with the image」(dpr を「画像の高さ ÷ 2D 表示の高さ」にする)。fps と速さ。
  - 出力: ディレクトリ、名前の頭、形式。「Name the images by the cycles」(1 フレームが 1 コマのときだけ。`PREFIXc01234.00.png`)。
  - 要約: コマとフレームの数、試合の時刻の区間、大きさ、動画にしたときの長さ。誤りのときは理由を出し、Export を押せなくする。
  - Export: すでにある名前のファイルを一度に数え、1 回だけ尋ねる (上書き / 別のディレクトリ / やめる)。最初の画像の圧縮した大きさからディスクの量を見積もり、空き (`QStorageInfo`) より多ければ警告する。設定を保存し、再生を止めて `ExportJob` を始める。進捗はアプリケーションのモーダルの `QProgressDialog` (Stop で中止、2 秒を過ぎたら残りの時間)。
  - サーバに接続している間は、Export を無効にして理由を出す (S3)。
- `Options::ExportSettings` と、群 `Export` の読み書き (S10)。
- `MainWindow`: File > Export Images / Video... (Ctrl+Shift+E、`objectName` は `export`)。フレームがあり、接続していないときだけ有効 (ログを開いたとき、接続と切断のときに更新する)。ダイアログは初めて開くときに作り、表示の更新で要約を直す。
- 今の File > Save Image と `ImageSaveDialog` を消す (D3)。`--auto-image-save` は Phase 5 まで今の `ImageSaveDialog` の処理を使うので、その処理は Phase 5 まで別の場所に残すか、Phase 5 で消す (実装のときに決める)。
- `RELEASE_NOTES.md` に日付つきで足す。

#### 4.2.2 実装 (2026-11-28)

4.2.1 のとおり、rcssmonitor の PR の段階 2 のコミット https://github.com/rcsoccersim/rcssmonitor/pull/76/commits/d7adf47f25ee61d49f026c5ce7fc1dfd61bd1477 のファイルを移した。rcssmonitor でその後に決めたことのうち、2D で Phase 2 にかかわるものは初めから入れた。計画から決めたことも書く。

- `src/qt/export_timing.{h,cpp}`: rcssmonitor のとおり。3D がないので、`frames2D()` を `frames()`、`DEFAULT_FPS_2D` を `DEFAULT_FPS` にした。
- `src/qt/export_range.{h,cpp}`: フレームは `MonitorViewData::Cont`、時刻は `rcsc::GameTime`。無効な番号は `ExportRange::INVALID_INDEX`。位置は `1234,3` も受け付ける (S9)。
- `src/qt/frame_sink.{h,cpp}`、`src/qt/export_job.{h,cpp}`: rcssmonitor の段階 2 のとおり (複数の出力先の「部分」は Phase 4 で足す)。`src/qt/image_sequence_sink.{h,cpp}`: 名前にディレクトリを含められる形 (rcssmonitor の段階 3) を初めから入れた。
- `src/qt/export_settings.{h,cpp}` (新規): 出力の設定 `ExportSettings` (rcssmonitor の `Options::ExportSettings`)。model のライブラリは Qt を使わないので、`src/qt` に置き、`~/.soccerwindow2` の群 `Export` を読み書きする (S10)。D7 のため、ディレクトリ、名前の頭、形式は `Options` の `imageSaveDir()`、`imageNamePrefix()`、`imageSaveFormat()` を使う: `MainWindow::readSettings()` (コマンドラインを読む前) で保存した値を `Options` に読み (`readImageOptions()`)、コマンドラインで指定したオプションがそれを上書きし、ダイアログはその値で始める。出力を始めるときに、使った値を `Options` と `~/.soccerwindow2` に書く。`MainWindow::saveSettings()` (終わるとき) には書かない。`Options` の形式は今のオプションと同じ大文字 (`PNG`、`JPG`) にする (`--auto-image-save` の `ImageSaveDialog` が形式の名前と比べるため)。
- `src/qt/export_dialog.{h,cpp}`: rcssmonitor の段階 2 のダイアログ。`DispHolder` を `MainData` に、ステップを `rcsc::ServerParam::i().simulatorStep()` にした。rcssmonitor の後の決定から: 範囲の「All」を「Clear」に (rcssmonitor の D12)、「Scale the texts with the image」を「Enlarge the texts, logos and lines with the image」とその説明に (D16。「in 2D」は除いた)、進捗の 2 秒の後の残りの時間 (段階 4)。枠の名前は「Images」(3D がないので「(2D view)」を付けない)。位置の欄の説明に、スコアボードの書き方 `1234,3` を書いた。
- `UIPalette` (追加): 誤りの文字の色 `errorText()`、`isDark()`、`markWrongText()` を rcssmonitor から移した。色の配色を変えたときは、ダイアログの `changeEvent()` で塗り直す。
- `MainWindow`: File > Export Images / Video... (Ctrl+Shift+E、`export`) を Save Image of This Frame の次に置き、今の File > Save Image (`show_image_save_dialog`) とそのスロットを消した (D3)。`ImageSaveDialog` は、Phase 5 まで `--auto-image-save` (`saveImageAndQuit()`) が使うので残した。`updateExportAction()` (フレームがあり、接続していないときだけ有効) を、ログを開いたとき (失敗も)、接続と切断のときに呼ぶ。`showExportDialog()` は初めて開くときにダイアログを作り、再生を止める信号、終わりのステータスバー、`viewUpdated` での要約の更新をつなぐ。
- 作業中に見つけて直したこと: `MainWindow::init()` は、前に開いたログを `QTimer::singleShot( 100, [this]() { ... } )` で開き直すが、タイマーにコンテキストのオブジェクトがなく、窓が先に消えると消えた窓で `openRCG()` を呼んだ。窓を続けて作るテストで落ちた (Qt 5)。コンテキストに `this` を渡した (アプリでは窓は 1 つなので、画面の動きは変わらない)。
- テスト (新規。rcssmonitor から移した):
  - `export_timing`: 1 コマで進むフレーム、コマの数、各コマのフレーム、長さ、名前の桁。
  - `export_range`: 位置の解釈 (`1234,3` を足した) と書き出し、始めと終わりの番号 (止まっているフレーム、欠けたサイクル、ログの外、寄せ)、区間、試合の時刻。
  - `image_sequence_sink`: 名前どおりのファイル (PNG、JPEG、ディレクトリ、名前の中のディレクトリ)、作れないディレクトリ、名前より多い画像、`ExportJob` (画像が `renderFrame()` と同じ、`start()` の後に描く、画面の状態が変わらない、中止、出力先の誤り、フレームなし)。
  - `export_dialog`: メニュー (ショートカット、Save Image of This Frame の次、今の Save Image がない、ログを開くと有効、モードレス、1 つだけ)、範囲 (空の欄、Now、Clear、寄せ、`1,0`、誤りで押せない)、出力 (ファイルの名前と大きさ、再生を止める、画面が変わらない、設定の保存と `~/.soccerwindow2`)、サイクルでの名前、大きさと拡大、すでにあるファイル、接続しているとき、D7 (保存した値を `Options` に読む、コマンドラインの値が優先する、出力すると使った値を保存する)。
  - HOME と offscreen の設定を `test/test_environment.h` (新規) にまとめ、`field_canvas_render` もそれを使う形にした。`gui_smoke` から、消した `show_image_save_dialog` の試験を除いた。
- S17、D8 (著作権表示): 移したファイルと新しいファイルの見出しを `Copyright (C) Hidehisa AKIYAMA` と GPL の第 3 版以降にした (soccerwindow2 のほかのソースと同じ書き方)。Phase 1 の `frame_scope.{h,cpp}` と `test_field_canvas_render.cpp` (第 2 版以降にしていた) も直した。
- 確かめたこと (2026-11-28):
  - CMake の Qt 6.4.2 と Qt 5.15.13 のビルドで、警告なしにビルドでき、`ctest` のテスト 15 個 (新しい 4 つを含む。`monitor_client_live` は飛ばす) が、それぞれ 3 回ともすべて通った。新しいテストの試験の数は、`export_timing` 19、`export_range` 31、`image_sequence_sink` 10、`export_dialog` 10 (初めと終わりを含む)。
  - Autotools (Qt 5) で、作業ツリーの写しから、ビルド (警告は前からある `configure.ac` の 1 つだけ)、`make check`、`make distcheck` が通った。配布物に新しいファイル 19 個が入る。
  - 実際の試合のログ (`202607050404-HELIOS2026_0-vs-YuShan2026_1.rcg.gz`) で、ダイアログからサイクル 1000〜1299 を 1920×1080 の PNG で出した。止まっているフレームを含む 330 フレームが 330 枚になり、12.5 秒 (1 枚 38 ms)、記憶の最大は約 104 MB、ディスクは 49 MB だった。画像は画面を拡大した配置だった。ダイアログを画像にして見た (終わりのメッセージで、ダイアログが縦に伸びる)。

### 4.3 Phase 3: 範囲の便利な指定

#### 4.3.1 設計

rcssmonitor の段階 3 (rcssmonitor の計画 4.4、4.12) を移す。

- `src/qt/export_range_list.{h,cpp}` (新規): 範囲の一覧のモデル `ExportRangeList` (`QObject`) と `ExportRangeSpec` (始めと終わりの位置の文と名前)。足す、消す、並べ替える、今の範囲、`resolve()`、`newName()`、`safeName()`。メインの窓が持ち、印、ダイアログ、スライダーが同じ一覧を見る。別のログを開くか、サーバに接続すると消す。
- `MainWindow`: File > Export Images / Video の中に、Open the Export Dialog... (Ctrl+Shift+E)、Start the Range Here (`[`)、End the Range Here (`]`)、Clear the Range を置く (rcssmonitor の D7、D11)。今の範囲の始めと終わりを今の位置にし、ステータスバーに出す。
- `src/qt/log_player_slider.{h,cpp}` (新規): `CycleSlider` を切り出した `LogPlayerSlider` (S12)。今のクリックでの移動はそのまま。`setBands()` と `setGoals()` で、溝の上に範囲の帯 (今の範囲は濃く) と得点の印 (配色に合わせた橙) を描く。横と縦の両方。
- `ExportRange` (追加):
  - `periods()`: ログのサーバのパラメータ (`half_time`、`nr_normal_halfs`、`extra_half_time`、`nr_extra_halfs`) から求めたサイクルの境で、前半、後半、延長の各ハーフに分ける。PK 戦は最初の PK のプレイモードから後 (rcssmonitor の 4.12。rcssserver 19 はハーフの終わりのプレイモードを記録しない)。
  - `goals()`: `ViewHolder::scoreChangeIndices()` の番号ごとに、チーム、時刻、スコア。その番号が rcssmonitor の `scoreChangedIndex()` と同じフレーム (新しいスコアの直前) を指すかを確かめ、ログの再生の「次の得点へ」と同じ番号にそろえる (rcssmonitor の D10)。
  - `around()`: あるサイクルの前後の秒数。ログの端で切る。
- `ExportDialog` (追加): 範囲の一覧 (Add、Remove、Up、Down)、Presets (全体、区間、得点ごと、すべての得点、今の位置の前後)、秒数の欄、複数の範囲の出し方 (範囲ごとに別のディレクトリ、または 1 つの連番)。
- `Options::ExportSettings` (追加): `separate_ranges`、`goal_before`、`goal_after`、`around_before`、`around_after`。
- 得点と区間のテストに使うログは、`test/data/` に小さなログを足すか、テストの中でフレームを作る (実装のときに決める)。
- `RELEASE_NOTES.md` に日付つきで足す。

#### 4.3.2 実装 (2026-11-28)

4.3.1 のとおり、rcssmonitor の PR の段階 3 のコミット https://github.com/rcsoccersim/rcssmonitor/pull/76/commits/d7a5cd183a768b1cd87dde0a5f6523821ae16cf5 のファイルを移した。計画から決めたことも書く。

- `ExportRange` (追加): `cyclesOf()`、`periods()`、`goals()`、`around()` と、`ExportPeriod`、`ExportGoal`。フレームは `MonitorViewData::Cont`、プレイモードは `rcsc::PlayMode`、チームとスコアは `leftTeam()`、`rightTeam()`。区間を決めるサーバのパラメータは、新しい構造体 `ExportGameParam` (`half_time`、`nr_normal_halfs`、`extra_half_time`、`nr_extra_halfs`、ステップ) で渡し、`ExportGameParam::current()` が `rcsc::ServerParam` から作る (`rcsc::ServerParam` はテストで値を変えられないため)。
- 得点の番号を確かめた: `ViewHolder::scoreChangeIndices()` は、rcssmonitor の `DispHolder::scoreChangedIndex()` と同じく、新しいスコアの直前のフレームを指す (実際のログで、2979 番がサイクル 2710 の 0-0、2980 番がゴールのプレイモードと 0-1)。`goals()` は rcssmonitor のまま (その次のフレームからチームとスコアを読む) で、ログの再生の「次の得点へ」(G) と同じ番号になる。
- `src/qt/export_range_list.{h,cpp}` (新規): rcssmonitor のまま (フレームの型だけを替えた)。
- `src/qt/log_player_slider.{h,cpp}` (新規): `log_player_tool_bar.cpp` の中のクラス `CycleSlider` を `LogPlayerSlider` に切り出し (S12)、rcssmonitor の帯と得点の印の描き方 (`setBands()`、`setGoals()`、`positionOf()`、`bandColor()`、`goalColor()`、`paintEvent()`) を移した。クリックでの移動などの今の動きは変えない (`CycleSlider` のクリックの処理はもともと無効)。`positionOf()` はスタイルの向き (`upsideDown`) に従うので、縦の向きでもつまみと同じ位置に描く。`LogPlayerToolBar::cycleSlider()` で取り出す。
- `ExportDialog`: rcssmonitor の段階 3 の版から、Phase 2 と同じ置き換えで移し直した。範囲の一覧 (Add、Remove、Up、Down、名前)、Presets (全体、区間、得点ごと、すべての得点、今の位置の前後)、秒数の欄、範囲の出し方 (範囲ごとのディレクトリか 1 つの連番)。範囲を消すボタンの名前は `clear_range` になった。ソースのコメントの rcssmonitor の決定の番号は、この計画の番号 (D9、D10) か「rcssmonitor's D12」にした。
- D12 (よく使う範囲の秒数は再生の時間で数える): `ExportRange::around()` を、サイクルではなくフレームの番号を中心に取る形にし、中心から前後の秒数のフレームの数 (`cyclesOf()`) を直接取って、ログの端で切る。止まったフレームも 1 ステップで、出力の画像 (`ExportTiming`: 1 倍速で 1 フレームが 1 枚) と同じ数え方になる。ライブで欠けたサイクルは、画像と同じく数えない。`ExportGoal::frame` (新規。得点のフレーム、つまり新しいスコアの最初のフレーム) を足し、得点の場面はそこを中心にする。今の位置の前後は今のフレームを中心にする。2023 年のログで、得点の場面 (10 秒前から 5 秒後) は、201 フレーム (キックオフの後の 50 サイクルまで) から 151 フレーム (得点の後の止まった 50 フレームまで。例: 337〜437.50) になった。
- `ExportSettings` (追加): `separate_ranges`、`goal_before`、`goal_after`、`around_before`、`around_after`。
- `MainWindow`: `ExportRangeList` を 1 つ持つ。File > Export Images / Video をサブメニューにし、Open the Export Dialog... (Ctrl+Shift+E)、Start the Range Here (`[`)、End the Range Here (`]`)、Clear the Range を置いた (rcssmonitor の D11)。印は今の位置を範囲の始めか終わりにし、ステータスバーに出す。有効になる条件は出力と同じ。範囲の一覧が変わるたびと `updateExportAction()` のときに、スライダーに帯と得点の印を渡す (`updateRangeMarks()`。接続している間は出さない)。別のログを開くときと接続するときに範囲を消す。`logPlayerSlider()` (テストのため)。`M_log_player_tool_bar` を初期化子で `nullptr` にした (ツールバーより先に作るアクションから `updateRangeMarks()` が呼ばれるため)。
- S18 (テストのための public を増やさない): Phase 1〜3 でテストのために `MainWindow` に足した public のメンバを見直した。
  - `renderImage()` と、範囲の印を描くための `logPlayerSlider()` は private にした。
  - `exportDialog()` と `exportRanges()` は消した。範囲の一覧とダイアログに `objectName` (`export_ranges`、`export_dialog`) を付け、テストは `findChild()` で取る。スライダーも `findChild< LogPlayerSlider * >()` で取る。
  - `defaultImageName()` と `saveImage( path, error )` は、新しい `src/qt/frame_image.{h,cpp}` の純粋な関数 `FrameImage::defaultName( ログの名前, 時刻 )` と `FrameImage::write( image, path, error )` に出した。`MainWindow` は、名前に使うログの名前 (サーバからのフレームと匿名モードでは空) と今のフレームを選び、ファイルのダイアログを出すだけになった (`defaultImageName()` は private)。
  - `openRCG( const QString & )` は public のまま残し、File > Open、ドロップ、コマンドラインと同じ入口であることを注釈に書いた。
  - テスト `field_canvas_render` の Save Image の試験を、メニューの項目の試験 (`saveImage`) と `FrameImage` の試験 (`frameImage`。名前、画面と同じ画像、PNG と JPEG、フレームなし、書けない場所) に分けた。`export_dialog` はアクセサの代わりに `findChild()` を使う。
- テスト:
  - `export_range` (追加): 区間 (2 つのハーフ、延長と PK 戦、後半から始まるログ、最後の境より後、ハーフの時間なし、フレームなし)、得点 (番号の付け方、得点のフレーム、新しいスコアのフレームを指す番号、増えない変わり目、範囲の外の番号)、前後 (D12: 止まったフレームが 1 ステップを取る、得点の後の 50 の止まったフレームで 5 秒後が終わる、欠けたサイクルは受け取ったフレームの数、ログの端、50 ms のステップ、フレームの外の番号)。フレームは `MonitorViewData` をプレイモードとチームから作る。
  - `export_range_list` (新規)、`log_player_slider` (新規。縦の向きの試験を足した)。
  - `export_dialog` (追加): サブメニュー、印 (`[` と `]`、メニュー、ダイアログとスライダーへの反映、印を消す、別のログで消える)、Presets、複数の範囲。Presets の試験の試合はフレームがとびとびのサイクルにしかないので、得点と今の位置の前後の秒数を 0.1〜0.2 秒 (1〜2 フレーム) にして確かめる (D12)。Presets の試験の試合は、rcssmonitor ではライブのフレーム (JSON) で作っていたが、soccerwindow2 には 1 つずつ足す口がないので、`test/data/head_v5.rcg` の見出しと `show` の行を使ってテストの中でゲームログ (rcg v5) に書き、`MainData::openRCG()` で読む。
- 確かめたこと (2026-11-28):
  - CMake の Qt 6.4.2 と Qt 5.15.13 のビルドで、警告なしにビルドでき、`ctest` のテスト 17 個 (新しい 2 つを含む。`monitor_client_live` は飛ばす) が、それぞれ 3 回ともすべて通った。試験の数は、`export_range` 34、`export_range_list` 6、`log_player_slider` 5、`export_dialog` 13、`field_canvas_render` 11 (初めと終わりを含む)。S18 の直しの後にも、Qt 6 と Qt 5 で 3 回ずつ、テスト 17 個がすべて通り、Autotools のビルド、`make check`、`make distcheck` が通った。
  - Autotools (Qt 5) で、作業ツリーの写しから、ビルド (警告は前からある `configure.ac` の 1 つだけ)、`make check`、`make distcheck` が通った。
  - rcssmonitor の確かめに使われた実際のログ (rcssmonitor の作業ツリーにあるものを読んだ) で、区間と得点が rcssmonitor の記録と一致した: 2022 年の延長のある試合は前半が 0〜3147 番、後半、延長 1、延長 2 と、6713 の HELIOS2022 の得点。2023 年の Oxsy 対 YuShan2023 の得点は 437、797、1075、3536。2024 年の CYRUS 対 HELIOS2024 では、延長 2 つと PK 戦 (8705 番から) と 4 つの得点。
  - 2023 年のログで、ダイアログの「すべての得点」から 4 つの範囲 (各 201 枚、計 804 枚) を 1280×720 で範囲ごとのディレクトリに 13.8 秒で出した。ダイアログ、メインの窓、横と縦のスライダー (範囲の帯、今の範囲の濃い帯、得点の印) を画像にして見た。

### 4.4 Phase 4: ffmpeg による動画

#### 4.4.1 設計

rcssmonitor の段階 4 (rcssmonitor の計画 4.5、4.13) を移す。

- `src/qt/ffmpeg_sink.{h,cpp}` (新規): `FfmpegSink`。
  - 引数 (静的な関数で組み、リストで渡してシェルを通さない):
    ```
    -hide_banner -loglevel error -nostats
    -f rawvideo -pixel_format bgr0 -video_size WxH -framerate F -i pipe:0
    -an -c:v libx264 -preset P -crf C -tune animation
    -vf scale=out_color_matrix=bt709:out_range=tv,format=yuv420p
    -colorspace bt709 -color_primaries bt709 -color_trc bt709
    [.mp4/.mov なら -movflags +faststart] -y OUT
    ```
  - フレームは `QImage::Format_RGB32` の行をそのまま標準入力に書く (リトルエンディアンでは `bgr0`)。動画の大きさは偶数にする。
  - 背圧: 書き待ちが 2 コマ分以上なら `write()` は BUSY を返し、`bytesWritten` で下回ったら `readyForFrame()` を出す。
  - 終わり: 標準入力を閉じて ffmpeg の終わりを待つ。誤り (起動の失敗、0 でない終了、入力の途中の終了) は、標準エラーの末尾を添えて知らせ、書きかけのファイルを消す。中止は ffmpeg を止めてファイルを消す (rcssmonitor の D13)。
  - 場所と検出: 空なら PATH の `ffmpeg`。`-version` と `-encoders` で、libx264、libopenh264、mpeg4 の順に選ぶ (rcssmonitor の D14)。
- `ExportJob`: 出力を「部分」(出力先とそのコマの列) の列にする。中止したら、今の部分だけを中止し、終わった部分は残す。
- `ExportDialog` (追加): Images と Video (ffmpeg) の選択。動画のファイル (拡張子がなければ `.mp4`。`.mp4`、`.mov`、`.mkv` のほかは誤り)、Quality (CRF、既定 18)、Encoding speed (preset、既定 medium)、ffmpeg のパスと状態 (動画を選んだときに初めて調べる)。範囲ごとの動画 (`FILE-名前.ext`) か 1 本。「As the Playback」(速さを `simulator_step / Options::timerInterval()` にする)。
- `Options::ExportSettings` (追加): `video`、`video_file`、`crf`、`preset`、`ffmpeg`。
- CI (S15)。`docs/build.md` に、実行時の任意の ffmpeg を書く。
- `RELEASE_NOTES.md` に日付つきで足す。

### 4.5 Phase 5: コマンドラインからの出力

#### 4.5.1 設計

rcssmonitor の段階 7 (rcssmonitor の計画 4.7、4.16) を移す。

- `src/qt/export_batch.{h,cpp}` (新規): `ExportBatch` (S14)。
  - オプション: `--export-video FILE`、`--export-images DIR`、`--export-range RANGE` (何度でも。S13)、`--export-join`、`--export-fps`、`--export-speed`、`--export-size WxH`、`--export-scale R`、`--export-image-format png|jpg`、`--export-image-prefix`、`--export-crf`、`--export-preset`、`--ffmpeg PATH`。`Options::parseCmdLine()` に新しい群 (Export Options) として足す。`--export-view` は置かない (D2)。
  - `RANGE` の書き方: `START-END` (どちらかの端は空でよい)、`all`、`first-half`、`second-half`、`half-N`、`extra-N`、`penalties`、`game`、`goals[:BEFORE:AFTER]`。指定がなければ `all`。範囲が複数なら範囲ごと (`FILE-NAME.ext`、`DIR/NAME`)、`--export-join` で 1 つにまとめる。
  - `run()`: ログを `MainData::openRCG()` で読み、範囲、コマ、大きさ (動画は偶数)、出力先を作り、`ExportJob` をイベントループで回す。進捗は標準エラーに、1 割ごとか 1 秒ごとに出す。終わりに出力先と時間を出して 0 を返す。誤りは `soccerwindow2: 理由` と 1。
- `main()` (`src/qt/main.cpp`): `--export-video` か `--export-images` があれば、アプリケーションを作る前にプラットフォームを offscreen にする。オプションを読んだ後、窓を作らずに `ExportBatch::run()` の終了コードで終わる。
- `--auto-image-save` (D4): `ExportBatch` で、ログ全体を連番画像で出して終わる。窓は出さない。ディレクトリ、名前の頭、形式は `--image-save-dir`、`--image-name-prefix`、`--image-save-format` から取る。大きさと名前の決め方は §3.3 で決める。`ImageSaveDialog` の残りの処理を消す。
- `RELEASE_NOTES.md` に日付つきで足す。README に、コマンドラインからの出力の例を 1 つ書く。

### 4.6 Phase 6: 出力のマニュアル

#### 4.6.1 設計

rcssmonitor の段階 8 とその Tips (rcssmonitor の計画 4.17) を移す。

- `docs/export.md` (新規): rcssmonitor の `docs/export.md` から 3D の節を除き、soccerwindow2 の画面とオプションに合わせる。必要なもの、1 枚の保存、出力のダイアログ (範囲、メインの窓の印、よく使う範囲、画像の大きさと速さ、画像と動画、出力の流れ)、速さと長さ、コマンドライン (オプション、範囲の書き方、例、ディスプレイなしで動かす)、ファイルの大きさの目安、Tips (ffmpeg での後からの加工)、困ったとき。soccerwindow2 に固有のこととして、デバッグの重ね描きの扱い (S6) と、今の `--auto-image-save` との関係を書く。
- README に、要点とマニュアルへのリンクを書く。`docs/build.md` (ffmpeg の行) からマニュアルを指す。
- マニュアルの例は、書いたとおりに実際のログで実行して確かめる。

## 5. Phase

どの Phase も、ビルドでき、CI が通る。

| # | 作業 | 完了条件 |
|---|---|---|
| 1 | `Options::ViewState`、`MainData::update()` の分割、`FrameScope`、`FieldCanvas::renderFrame()`、File > Save Image of This Frame | 画面と同じ画像になる、描いた後に状態が戻る、のテストが CMake の Qt5 と Qt6 で通り、Autotools でもビルドできる。ユーザが HiDPI を含めて画面で確かめる |
| 2 | `ExportTiming`、`ExportRange` の基本、`FrameSink`、`ImageSequenceSink`、`ExportJob`、`ExportDialog` (範囲は 1 つ、画像)、出力の前の確認、`Export` の設定、今の Save Image の置き換え (D3) | 1 試合と、選んだ範囲の PNG の連番を出せる。終わりのサイクルの止まっているフレームも入る。中止できる。出力の後も画面が変わらない |
| 3 | `ExportRangeList`、メインの窓の印、`LogPlayerSlider` の帯と得点の印、よく使う範囲、複数の範囲 | 範囲のテストが通る。ユーザが実際のログで、得点の場面をまとめて出せることと、スライダーの帯 (横と縦) を確かめる |
| 4 | `FfmpegSink`、ffmpeg の検出、ダイアログの動画の部分、CI の ffmpeg | 偽の ffmpeg と本物の ffmpeg のテストが通る。mp4 が再生でき、長さが N/fps (まとめたときは和)。ffmpeg がないときに分かりやすい表示が出る |
| 5 | `ExportBatch`、`main()` の offscreen、`--auto-image-save` の移行 (D4) | offscreen でのバッチのテストが通る。ディスプレイなしで動き、`~/.soccerwindow2` を書かない。誤りで終了コード 1 と理由が出る。`--auto-image-save` が窓を出さずにログ全体を保存して終わる |
| 6 | `docs/export.md`、README、`docs/build.md` | 書いた操作と画面の文言が実際のアプリと一致する (書いた例を実行して確かめる)。ユーザが読んで確かめる |

## 6. テスト

新しいテストは `test/test_NAME.cpp` を `soccerwindow2_add_qt_test( NAME )` で足し、CMake のジョブで走らせる。手元では `./cmake-helper.sh test qt6` (Qt5 は `qt5`) か、ビルドのディレクトリで `ctest -R 'export|field_canvas|slider|ffmpeg' --output-on-failure` で動かせる。

| テスト | Phase | 内容 |
|---|---|---|
| `field_canvas_render` | 1 | 文字を消すと、`renderFrame()` と画面の `grab()` が画素で一致する。別のフレームを描いた後も画面と今の番号と `Options::viewState()` が変わらない (ボールのフォーカス、ズーム、自動の選択、スコアボードの表示を変えた直後も)。dpr 2 と 0.5 でスコアボードとゴールラインの位置。フォーメーションエディタのデータが変わらない。メニューの項目 (ショートカット、名前、PNG と JPEG、書けない場所、フレームがないとき) |
| `export_timing` | 2 | 1 コマで進むフレーム (速さ、fps、ステップ)、コマの数、各コマのフレーム、長さ、名前の桁 |
| `export_range` | 2、3 | 位置の解釈 (`1234`、`1234.3`、`1234,3`、誤り) と書き出し。止まっているフレーム、欠けたサイクル、ログの外での始めと終わりと寄せ。区間 (ハーフ、延長、PK 戦、後半から始まるログ)。得点 (番号の指すフレーム、増えない変わり目)。前後 (ログの端) |
| `image_sequence_sink` | 2 | 名前どおりのファイル (PNG、JPEG、ディレクトリを作る)、作れないディレクトリ。`ExportJob` の画像が `renderFrame()` と同じ、中止、出力先の誤り |
| `export_dialog` | 2〜4 | メニュー、範囲 (空の欄、Now、Clear、寄せ、誤り)、出力 (ファイルの名前と大きさ、再生を止める、画面が変わらない、設定の保存)、あるファイルの確認が 1 回、接続しているとき。Phase 3 で印、Presets、複数の範囲。Phase 4 で動画の欄、ffmpeg なし、代わりのエンコーダ、再生の速さのボタン |
| `export_range_list` | 3 | 編集、足す・消す・動かす、名前、文からフレームの区間 |
| `log_player_slider` | 3 | 帯 (今の範囲は濃い)、得点の印の色 (明暗)、縦の向き、帯の外は変わらない |
| `ffmpeg_sink` | 4 | 引数 (libx264、mpeg4、libopenh264、`.mkv`)、場所と検出、偽の ffmpeg (テストの中で一時ディレクトリにシェルのスクリプトを作る。実行ファイルはコミットしない) への書き込み、背圧、開けないとき、誤り、中止でファイルが消える、複数の動画。本物の ffmpeg (なければ飛ばす。`SOCCERWINDOW2_REQUIRE_FFMPEG=1` なら失敗にする) は ffprobe でコーデック、大きさ、コマの数 |
| `export_batch` | 5 | オプションの既定と誤り、`--export-range` の繰り返し、範囲の書き方と誤り、画像の出力、範囲ごとと `--export-join`、偽の ffmpeg での動画、できないとき (ログなし、読めない、範囲、ffmpeg なし) の終了コード 1 と理由、`~/.soccerwindow2` を書かない |

手で確かめること: HiDPI (`QT_SCALE_FACTOR` 1.25、1.5、2) での出力と画面。実際の試合のログ (得点、延長、PK 戦のあるもの) での区間と得点。デバッグの重ね描き (デバッグサーバの表示、描画データ) が出力に入ること。縦のスライダー。ブラウザと VLC での再生と色。1 試合の出力で記憶が増え続けないこと。中止。ffmpeg がないとき。ディスプレイなしのバッチ。

## 7. 変更するファイル (見込み)

| ファイル | 内容 |
|---|---|
| `src/model/options.{h,cpp}` | `ViewState`、static 変数のメンバ化、`ExportSettings`、出力のコマンドラインのオプション |
| `src/model/main_data.{h,cpp}` | `update()` の分割 |
| `src/qt/frame_scope.{h,cpp}`、`src/qt/export_timing.{h,cpp}`、`src/qt/export_range.{h,cpp}`、`src/qt/export_range_list.{h,cpp}`、`src/qt/frame_sink.{h,cpp}`、`src/qt/image_sequence_sink.{h,cpp}`、`src/qt/ffmpeg_sink.{h,cpp}`、`src/qt/export_job.{h,cpp}`、`src/qt/export_dialog.{h,cpp}`、`src/qt/export_batch.{h,cpp}` (新規) | 出力の仕組み |
| `src/qt/log_player_slider.{h,cpp}` (新規)、`src/qt/log_player_tool_bar.{h,cpp}` | スライダーの切り出し、範囲の帯と得点の印 |
| `src/qt/field_canvas.{h,cpp}` | `renderFrame()` |
| `src/qt/main_window.{h,cpp}`、`src/qt/main.cpp` | メニュー、範囲の印、設定、バッチ |
| `src/qt/image_save_dialog.{h,cpp}` | 消す (D3、D4) |
| `src/qt/CMakeLists.txt`、`src/qt/Makefile.am` | ビルド |
| `test/CMakeLists.txt`、`test/test_*.cpp`、`test/data/` | §6 |
| `.github/workflows/build.yml` | ffmpeg (S15) |
| `README.md`、`docs/build.md`、`docs/export.md` (新規)、`RELEASE_NOTES.md` | 文書 |

## 8. 対象外

次は扱わない。`docs/todo.md` に記録した。

- 3D 表示の出力 (soccerwindow2 に 3D 表示がない。D2)。
- Qt Multimedia による直接の動画化 (rcssmonitor でも見送り、rcssmonitor の `docs/dev/todo.md` の T1 にした)。
- 再生しながらの画面の録画 (OBS などに任せる)。
- 動画への音声。
- 止まっているフレームを飛ばす出力。
- 複数の範囲を 1 つの動画にまとめるときの、つなぎの効果 (フェード、字幕など)。後からの加工はマニュアルの Tips に書く。
- 範囲の一覧をファイルに保存すること。
- エージェントのデバッグログの重ね描きを、出力のコマごとに追従させること (S6)。

## 9. リスク

- 描画の状態: `MainData::update()` と `Options` の大域の状態のほかに、描き手が別の状態を持っていると、出力の後に画面が変わる。Phase 1 で描き手を調べ、テストで画面が変わらないことを確かめる。
- dpr のある `QImage` への QPainter の扱いと、整数の画素への丸め (rcssmonitor の 4.1、S15)。
- `--export-range` の繰り返し (S13): `ParamMap` に登録したオプションを `ExportBatch` が別に集めるので、両者の解釈がずれないようにテストする。
- `~/.soccerwindow2` の書き直し: テストとバッチで書かないことを確かめる (S14、S16)。
- libx264 のない ffmpeg: `-encoders` にあるものだけを使う。
- PNG の連番のディスクの量 (2D の 1080p で 1 試合 1〜2 GB)。

## 10. 記録

### 2026-11-28: 計画の作成

- ユーザの求め (D1、D2) を受けて、rcssmonitor の PR #76 (計画書、マニュアル、ソースの一覧) と、soccerwindow2 の 2D 表示 (`FieldCanvas`、`MainData::update()`、`Options::updateFieldSize()`)、今の画像の保存 (`ImageSaveDialog`、`--auto-image-save`)、スライダー、ショートカット、設定、コマンドラインの解析 (librcsc の `CmdLineParser` と `ParamMap`)、テストと CI を調べ、設計を立てた (§2、§4)。
- 調べて分かったこと: `Options::updateFieldSize()` に rcssmonitor で不具合になった static 変数がある (S4)。`MainData::update()` がフォーメーションエディタのデータを変える (S5)。エージェントのデバッグログは同期したフレームでだけ描かれる (S6)。`ParamMap` は同じオプションの繰り返しを受けられない (S13)。スライダーは縦にもなる (S12)。
- 対象外にしたもの (§8) を `docs/todo.md` に記録した。
- この後: ユーザが計画書を確かめる。確かめられたら、計画 Issue とサブ Issue の案を示す。

### 2026-11-28: 早めに決めた点とブランチ

- ユーザが、Claude の案のとおりに決めた: D3 (今の Save Image は Phase 2 で出力のダイアログに置き換えて消す)、D4 (`--auto-image-save` は Phase 5 で新しいバッチの仕組みに移す)。初めは Phase の初めに決める予定だった (§3.3)。§3.3、§4.2.1、§4.5.1、§5、§7 を直した。
- ユーザが GitHub で、`develop` の `6f2b7ea` から `feature/frame-export` を作った。
- 英語版の計画書は作らない (ユーザ)。
- この後: ユーザが計画書を確かめる。確かめられたら、計画 Issue とサブ Issue を作り、URL を冒頭の表と「Phase と Issue」の表に書いて、計画書をコミットする。

### 2026-11-28: 計画の確定と Issue

- ユーザが計画書を確かめた。
- 計画 Issue https://github.com/helios-base/soccerwindow2/issues/100 (ラベル `plan`) と、Phase 1〜6 のサブ Issue https://github.com/helios-base/soccerwindow2/issues/101 〜 https://github.com/helios-base/soccerwindow2/issues/106 を、日本語で作った。7 つとも常設の Project (helios-base の 9) に追加し、Status を Todo にした。冒頭の表と「Phase と Issue」の表に URL を書いた。
- 他のリポジトリの作業は要らないことを §4 の初めに書いた (追従 Issue は作らない)。
- この後: Phase 1 を、始める前にユーザに確かめる (§3.3 の Phase 1 の項目を決める)。

### 2026-11-28: Phase 1 の開始と実装

- ユーザの指示で Phase 1 を始めた。Phase の初めにユーザが決めた: D5 (画像の文字は灰色のアンチエイリアス)、D6 (CI は Phase のプッシュの後に手動で走らせる)。
- 4.1.2 のとおり実装した。テスト `field_canvas_render` を足した。
- 調べて計画に足したこと: `FormationEditorPainter` が `painter.viewport()` で配置していたので、`painter.window()` に直した。ログの名前は匿名モードでも使わない。
- テストで見つけて直したこと: スタミナの数字も文字なので、画面と比べるときに消す。フォーメーションエディタのボールの位置は 0.5 に丸められる。半分の大きさの画像では線が芝の色と混ざるので、線の位置を明るさで探す。
- 確かめたこと: 4.1.2 のとおり (CMake の Qt 6 と Qt 5 でテスト 11 個がすべて通る、Autotools でビルドと `make check`、実際のログの画像)。手元の in-tree の Autotools の設定は古く (`config/test-driver` がない)、そこでの `make` は失敗したので、作業ツリーの写しで確かめた。
- この後: ユーザが画面で Save Image of This Frame を確かめる (HiDPI を含む。Phase 1 の完了条件)。その後、Phase 1 のコミットとプッシュ、CI (D6)。

### 2026-11-28: Phase 2 の開始と実装

- ユーザが画面で Phase 1 を確かめた。Phase 1 のコミット、プッシュ、CI の後、ユーザが Phase 1 の完了を確かめ、ユーザの指示で Phase 2 を始めた。Phase の初めにユーザが決めた: D7 (画像のコマンドラインのオプションは、指定したときだけ優先する)。
- 4.2.2 のとおり実装した。テスト `export_timing`、`export_range`、`image_sequence_sink`、`export_dialog` を足した。
- 移植の元には、rcssmonitor の最終版ではなく、段階 2 のコミットのファイルを使った (Phase 2 の範囲にちょうど合うため)。GitHub から読むだけで取り出した。
- 調べて決めたこと: S17 (移したファイルの著作権表示)。model のライブラリは Qt を使わないので、出力の設定は `src/qt/export_settings` に置いた。D7 は、`MainWindow` がコマンドラインより先に設定を読む順を使って実現した。
- テストで見つけて直したこと: `MainWindow::init()` のタイマーにコンテキストがなかった (4.2.2)。D7 のテストを、`MainWindow` を作ってからコマンドラインの値を入れる順にした。
- 確かめたこと: 4.2.2 のとおり (CMake の Qt 6 と Qt 5 でテスト 15 個が 3 回ともすべて通る、Autotools でビルドと `make check` と `make distcheck`、実際のログでの出力)。
- この後: ユーザが画面で出力のダイアログを確かめる。その後、Phase 2 のコミットとプッシュ、CI (D6)。

### 2026-11-28: 著作権表示 (D8)

- ユーザが、ソースの著作権表示から `The RoboCup Soccer Server Maintenance Group.` を除き、`Hidehisa AKIYAMA` だけにするよう指示した (D8)。S17 で付けた 21 個のファイル (src/qt の出力のファイル、`frame_scope.{h,cpp}`、5 つのテスト) を直した。GPL の第 3 版以降はそのまま。
- この後: ユーザが画面で出力のダイアログを確かめる。その後、Phase 2 のコミットとプッシュ、CI (D6)。

### 2026-11-28: 計画 ID

- CLAUDE.md に計画 ID と Issue のタイトルの規則が加わり、ユーザの指示でこの計画を合わせた。
- このリポジトリの計画 Issue (ラベル `plan`) には計画 ID の付いたものがなかったので、この計画を SW2-01 とし、冒頭の表に書いた。
- 7 つの Issue のタイトルの先頭に計画 ID を付けた: 計画 Issue は `[SW2-01] Plan: 2D 表示の画像と動画の出力 (rcssmonitor からの移植)`、サブ Issue は `[SW2-01] Phase N: <名前>` (閉じた Phase 1 も)。本文、ラベル、親子関係、Project の Status は変えていない。
- この後: Phase 2 の確認。develop への PR のタイトルの先頭には `[SW2-01]` を付ける。

### 2026-11-28: Phase 3 の開始と実装

- ユーザが Phase 2 の完了を確かめ、ユーザの指示で Phase 3 を始めた。Phase の初めにユーザが決めた: D9 (印のキーは `[` と `]`)、D10 (既定の秒数は rcssmonitor と同じ)、D11 (スライダーに得点の印を出す)。
- 4.3.2 のとおり実装した。テスト `export_range_list`、`log_player_slider` を足し、`export_range`、`export_dialog` に Phase 3 の分を足した。
- 調べて決めたこと: 得点の番号は rcssmonitor と同じものを指す (4.3.2)。区間のサーバのパラメータは `ExportGameParam` で渡す。Presets の試験の試合は、テストの中でゲームログに書く。
- テストで見つけて直したこと: 範囲の一覧の試験のフレームの作り方を、元のテストと同じサイクル 1 からにした。
- 確かめたこと: 4.3.2 のとおり (CMake の Qt 6 と Qt 5 でテスト 17 個が 3 回ともすべて通る、Autotools でビルドと `make check` と `make distcheck`、実際のログで区間と得点が rcssmonitor と一致、得点の場面の出力)。
- この後: ユーザが実際のログで、得点の場面をまとめて出せることと、スライダーの帯 (横と縦) を確かめる (Phase 3 の完了条件)。その後、Phase 3 のコミットとプッシュ、CI (D6)。

### 2026-11-28: テストのための public の見直し (S18)

- ユーザが、`MainWindow::logPlayerSlider()` がテストのためだけに public になっていないかを尋ね、public のメンバが増えると依存を確かめる手間が増えると指摘した。調べると、Phase 1〜3 で足した public のメンバ (`renderImage()`、`defaultImageName()`、`saveImage( path, error )`、`exportDialog()`、`exportRanges()`、`logPlayerSlider()`) は、`main_window.cpp` の外ではテストからしか使われていなかった。
- Claude の案のとおりに直すことをユーザが決めた (S18)。`openRCG( const QString & )` は public に残す案 (a) をユーザが選んだ。4.3.2 のとおり直した。
- この後: ユーザが Phase 3 を画面で確かめる。その後、Phase 3 のコミットとプッシュ、CI (D6)。

### 2026-11-28: 得点の場面の秒数 (D12、D13)

- ユーザが画面で確かめ、得点の場面の「後」の秒数が、得点からではなく次のキックオフからの時間になっていることに気づいた。調べると、秒数をサイクルで数えていて、得点の後の止まった 50 フレーム (サーバの時計が止まっている間) が数えられていなかった。rcssmonitor も同じ作り (得点の場面が 201 フレーム) で、計画の D10 ではどちらの時計で数えるかを決めていなかった。
- ユーザが決めた: 再生の時間で数える (D12。Claude の案 A)。rcssmonitor にも同じ直しの Issue を、rcssmonitor のルールで作る (D13)。
- 4.3.2 のとおり直した。CMake の Qt 6 と Qt 5 で、テスト 17 個が 3 回ずつすべて通った。2023 年のログで、得点の場面が 151 フレームになり、得点の後の止まったフレームの終わり (例: 437.50) で終わることを確かめた。
- この後: rcssmonitor の Issue の案をユーザに示す。ユーザが Phase 3 を画面で確かめる。その後、Phase 3 のコミットとプッシュ、CI (D6)。

### 2026-11-28: 前後の秒数をフレームの数で取る (D12 の直し)

- ユーザが、`around()` で止まったフレームを数えて位置を作り直すのは要るのか、中心のフレームから前後のフレームの数を直接取ればよいのではないかと尋ねた。答え: 位置は 1 フレームごとに 1 ずつ増えるので、すべてのサイクルがあるログでは同じ結果になり、違うのはライブで欠けたサイクルのあるログだけ。そこでも、出力の画像は 1 フレームを 1 ステップとして作るので、フレームの数で取るほうが動画の時間と合う。位置の方式は要らなかった。
- 中心から前後のフレームの数を直接取る形に直した (4.3.2)。`export_range` の欠けたサイクルの試験と、`export_dialog` の Presets の試験 (秒数を 1〜2 フレームに) を直した。CMake の Qt 6 と Qt 5 で、テスト 17 個が 3 回ずつすべて通った。2023 年のログの得点の場面は、151 フレーム (例: 337〜437.50) のまま変わらない。
- rcssmonitor の Issue の案 (英語。今の動き、直し方、やること) を示したが、ユーザが、Issue は作らずに後で自分で対応すると決めた (D13)。`docs/todo.md` に記録した。
- この後: Phase 3 のコミットとプッシュ、CI (D6)。
