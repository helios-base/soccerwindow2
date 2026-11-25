# 後で対応する作業

計画書で対象外にしたり見送ったりした作業と、後で対応することになった作業を記録する。
各項目には、記録した日付と、元の計画書の節や決定事項の番号を書く。
対応したら、その項目に、対応した計画書や PR を書き添える。

## ゲームログ (RCG) の読み込みの高速化 (`docs/plan/2026-11-25-00_rcg-load-in-memory.md`)

- (2026-11-25) ファイルのダイアログのフィルタ (`*.rcg.gz`) は、soccerwindow2 の zlib (`HAVE_LIBZ`) で決めている。gzip のログを読むのは librcsc (`rcsc::gzread_file()`) なので、soccerwindow2 を zlib なしでビルドすると、librcsc が gzip を読める場合でも、フィルタに `*.rcg.gz` が出ない (「All files」で選べば読める)。まれな場合のため見送った。(§8)
- (2026-11-26) librcsc の `rcsc::gzread_file()` が、通常のファイルでないもの (ディレクトリなど) を拒むようにすること。ext4 のディレクトリでは `std::length_error` を投げる。soccerwindow2 では `MainData::openRCG()` で try/catch で守った。librcsc の作業で、librcsc の `docs/todo.md` に記録した。(D14、§8)
- (2026-11-25) ゲームログでないファイルが、0 フレームで「開けた」ことになる (librcsc の `Parser::create()` がバージョン 1 と判定する)。変更前と同じで、この計画では扱わなかった。(§8)
- (2026-11-25) librcsc の `docs/todo.md` の「RCG パーサの高速化」の最初の項目 (soccerwindow2 の読み込みの切り替え) に、この計画書と PR を書き添える。PR の番号が決まった後に、librcsc のリポジトリで行う。(§8)
