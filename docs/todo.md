# 後で対応する作業

計画書で対象外にしたり見送ったりした作業と、後で対応することになった作業を記録する。
各項目には、記録した日付と、元の計画書の節や決定事項の番号を書く。
対応したら、その項目に、対応した計画書や PR を書き添える。

## ゲームログ (RCG) の読み込みの高速化 (`docs/plan/2026-11-25-00_rcg-load-in-memory.md`)

- (2026-11-25) ファイルのダイアログのフィルタ (`*.rcg.gz`) は、soccerwindow2 の zlib (`HAVE_LIBZ`) で決めている。gzip のログを読むのは librcsc (`rcsc::gzread_file()`) なので、soccerwindow2 を zlib なしでビルドすると、librcsc が gzip を読める場合でも、フィルタに `*.rcg.gz` が出ない (「All files」で選べば読める)。まれな場合のため見送った。(§8)
- (2026-11-26) librcsc の `rcsc::gzread_file()` が、通常のファイルでないもの (ディレクトリなど) を拒むようにすること。ext4 のディレクトリでは `std::length_error` を投げる。soccerwindow2 では `MainData::openRCG()` で try/catch で守った。librcsc の作業で、librcsc の `docs/todo.md` に記録した。(D14、§8)
- (2026-11-26) コマンドラインに置いたファイル (`-l` を付けないもの) を、`.rcg`・`.rcg.gz`・`.conf` 以外の名前でも受け付けること。`src/model/options.cpp` は拡張子で振り分けていて、rcgverconv で JSON に変換したログを `.json` の名前にすると、`Unsupported file: [...]` を出して終了する (rcssmonitor は拡張子によらず読む)。回避策は、`-l` を付けて渡すか、`.rcg` (`.rcg.gz`) の名前にすること。直すなら、`.conf` 以外をすべてゲームログとして扱うか、`.json`・`.json.gz` を足し、「Open」のダイアログのフィルタ (`*.rcg *.rcg.gz`) も直す。今回は対応しないことにした。(D15、§8)
- (2026-11-25) ゲームログでないファイルが、0 フレームで「開けた」ことになる (librcsc の `Parser::create()` がバージョン 1 と判定する)。変更前と同じで、この計画では扱わなかった。(§8)
- (2026-11-25) librcsc の `docs/todo.md` の「RCG パーサの高速化」の最初の項目 (soccerwindow2 の読み込みの切り替え) に、この計画書と PR を書き添える。PR の番号が決まった後に、librcsc のリポジトリで行う。(§8)

## 2D 表示の画像と動画の出力 (`docs/plan/2026-11-28-00_frame-export.md`)

- (2026-11-28) 3D 表示の出力。soccerwindow2 には 3D 表示がないため対象外にした。(D2、§8)
- (2026-11-28) Qt Multimedia (Qt 6.8 以上) による直接の動画化。移植の元の rcssmonitor でも見送り、rcssmonitor の `docs/dev/todo.md` の T1 にした。外部の ffmpeg だけにした。(§8)
- (2026-11-28) 再生しながらの画面の録画。OBS などの画面の録画のソフトに任せる。(§8)
- (2026-11-28) 動画への音声。(§8)
- (2026-11-28) 止まっているフレームを飛ばす出力。(§8)
- (2026-11-28) 複数の範囲を 1 つの動画にまとめるときの、つなぎの効果 (フェード、字幕など)。後からの ffmpeg での加工を、出力のマニュアルの Tips に書く。(§8)
- (2026-11-28) 範囲の一覧をファイルに保存すること。(§8)
- (2026-11-28) エージェントのデバッグログ (`DebugLogHolder`) の重ね描きを、出力のコマごとに追従させること。画面と同じく、デバッグメッセージの窓で同期したフレームでだけ描く。追従させるには、出力のコマごとに `MainData::seekDebugLogData()` でログを読み直し、終わった後にデバッグメッセージの窓の状態を戻す必要がある。(S6、§8)
- (2026-11-28) rcssmonitor にも、得点の場面と今の位置の前後の秒数を再生の時間 (中心のフレームから前後のフレームの数。止まったフレームも 1 ステップ) で数える直しが要る。rcssmonitor はサイクルで数えていて、得点の後の止まった 50 フレームが数えられず、「5 秒後」がキックオフから 5 秒になる (得点の場面が 201 フレーム)。直す所は、`ExportRange::around()` とその呼び出し元 (`ExportDialog` の Presets、`ExportBatch` の `goals[:BEFORE:AFTER]`)、テスト (`test_export_range`、`test_export_dialog`、`test_export_batch`)、`docs/export.md`。rcssmonitor の Issue は、ユーザが後で対応する。(D12、D13)
