# フォーメーションエディタの Undo と操作マニュアル 計画

| 項目 | 内容 |
|---|---|
| 作成日 | 2026-11-28 |
| 最終更新日 | 2026-11-29 |
| 対象 | `develop` (`206160e`)。librcsc は、調べたときは `develop` の `4291244`。Phase 1 からは、前提の作業 (§4 の初め) を取り込んだ `develop` の `8684789` 以降 |
| ブランチ | `feature/formation-undo`。`develop` の `206160e` から、ユーザが GitHub で作った |
| 計画 ID | SW2-02 |
| 計画 Issue | https://github.com/helios-base/soccerwindow2/issues/109 |
| 関連 | librcsc の前提の作業の Issue https://github.com/helios-base/librcsc/issues/124 (§4 の初め)。librcsc の `rcsc/formation/` (`Formation`、`FormationData`、`FormationDT`) |

## Phase と Issue

| Phase | 内容 | 実装の記録 | Issue |
|---|---|---|---|
| 1 | 履歴の仕組みと、データとロールの Undo/Redo (§4.1。D1、D2、D5、D6、S1〜S13、S18、S19) | — | https://github.com/helios-base/soccerwindow2/issues/110 |
| 2 | フィールドの操作の Undo/Redo と、メインの窓からのキー (§4.2。D1、S11、S14、S15) | — | https://github.com/helios-base/soccerwindow2/issues/111 |
| 3 | 操作マニュアル (日本語と英語、スクリーンショット) (§4.3。D3、D4、S16、S19) | — | https://github.com/helios-base/soccerwindow2/issues/112 |

## 1. 目的と方針

- soccerwindow2 に内蔵されている FormationEditor には Undo がない。そのため、編集のミス (誤った Replace や Delete、並べ替え、ボールを動かして手で並べた選手の位置を失うなど) を打ち消す作業が煩雑だった。Undo/Redo を入れ、ミスを手早く打ち消せるようにする。
- 方式: 状態の写し (スナップショット) を履歴に積む (S1)。操作ごとの逆操作は書かない。
- 取り消せる範囲 (D1): ファイルに保存される内容 (学習データとロール) の変更と、フィールドでの手の操作。
- 操作: Edit メニューとキー (D2)。
- 1 回の操作は、すべて成功するか、何も変えないかのどちらかにする (S7)。今は一部だけ成功して黙る操作があり、履歴の 1 手と食い違うため。
- 写しと復元を単純にするため、librcsc に小さな変更 (`FormationData` のコピー、対の番号のまとめての設定、読み込んだ対の番号の整え) を入れる (D5、D6、S17、§4 の初め)。librcsc の作業は librcsc の Issue として行い、`develop` に入れてから Phase 1 を始める。
- ついでに、操作が分かりにくいため、FormationEditor の操作マニュアルを、日本語と英語の両方で、スクリーンショットをつけて書く (D3、D4)。

## 2. 現状 (`develop` の `206160e`、librcsc の `4291244`)

### 2.1 フォーメーションの仕組み

- 学習データは、ボールの位置と、それに対する 11 人の位置の組である。`FormationDT` は、ボールの位置を頂点とする Delaunay の三角形分割を作り、ボールを含む三角形の中で線形補間して、未知のボールの位置に対する配置を求める。
- `.conf` は、ロール (名前、種類、左右、対の番号) と学習データを持つ。
- FormationEditor は、ボールの位置と選手の配置の編集、それらから作る三角形分割、ボールを動かしたときの配置の確認を、画面の上で行う。

### 2.2 モデル `FormationEditData`

`src/model/formation_edit_data.{h,cpp}`。Qt を使わない (ライブラリ `model`)。

- 持つもの: `M_formation` (ロールの配列と学習した模型)、`M_formation_data` (学習データ。`std::list<Data>`)、`M_triangulation`、`M_current_state` (フィールドに出ている 1 組)、`M_current_index`、選択の状態、`M_modified`。
- 編集の操作: `addData`、`insertData`、`replaceData`/`replaceBall`/`replacePlayer` (Pair Mode では対のデータも置き換え、なければ足す)、`deleteData`、`changeDataIndex`、`updateRoleData`/`updateRoleType`、`moveBallTo`、`movePlayerTo` (Pair Mode では対の選手も動かす)、`selectObject`/`moveSelectObjectTo`/`releaseObject`、`reverseY`、`openData`。データの操作は、最後に `fitModel()` (学習、選手の位置、三角形分割) を呼ぶ。
- 調べて分かった不具合:
  - `addData()` は誤り ("Too near data" など) を捨てる。フィールドの 1 組を最後のデータに替え、学習して、成功として返す。
  - `insertData()` は、Pair Mode でボールが中央 (|y| < 0.5) にあっても対のデータを入れようとする。そのため、1 つめのデータを入れた後に "Too near data" で失敗し、学習しないまま返る。
  - `replaceData()` は、Pair Mode で対のデータを足せなくても成功として返る (一部だけ成功)。
  - `openData()` は、読み込みに失敗すると、途中まで読んだデータと学習していない模型を残す。
  - Apply は、対の入れ替えに黙って失敗する (2.3 の `setPositionPair()` の制約)。
  - `M_modified` は、`fitModel()` とロールの更新で無条件に真になる (Apply を押すだけで「変更あり」になる)。

### 2.3 librcsc (`4291244`)

- `FormationData` はコピーできない (コピーのコンストラクタと代入が `delete`。コメントは「not used」)。データを足す操作は、既存のデータから 0.5 m 未満のボールの位置と、128 個を超えるデータを拒む。
- `Formation::setPositionPair()` は、ほかの選手がすでに対にしている番号を拒み、相手の欄も書き換える。そのため、対の入れ替え (2↔3、4↔5 から 2↔4、3↔5 へ) を 1 人ずつ設定すると失敗する。`setRole()` は、名前と種類を書いた後に失敗を返す。11 人分の対をまとめて設定する関数はない。
- `FormationData` の `data()`、`replaceData()`、`removeData()`、`changeDataIndex()` は、添字の範囲を `size() < idx` で調べるので、`idx == size()` を受け付けて末尾の次を触る (未定義の動作)。soccerwindow2 からは起きない。
- (2026-11-28、librcsc の前提の作業の中で分かった) フォーメーションのファイルの対の番号 `-1` は、「相手がこの選手を指す」という印である。
  - パーサは `setPositionPair()` を 1 人ずつ呼び、`setPositionPair()` は相手の欄にも番号を書く。
  - そのため、`-1` の選手が、それを指す選手より後にあると、相手の欄に書かれた番号を `-1` が上書きし、片方向の対が残る (例: 2 の対が 3 で、3 が -1)。
  - JSON の形式のパーサは、対の整合を確かめない。
  - 片方向の対が残ると、FormationEditor のパネルに `-1` が出て、何も変えずに Apply しても、窓の整合の確かめ (`checkConsistency()`) で "Inconsistency detected." になる。写しの対を、L2 の関数 (S17) で戻すこともできない。
- soccerwindow2 の CI は、librcsc の `develop` を取ってきてビルドする。`configure` と CMake は librcsc の版を調べず、文書は最新の librcsc (`develop`) を求めている (`docs/plan/2026-11-25-00_rcg-load-in-memory.md` の D4 と同じ)。

### 2.4 窓 `FormationEditorWindow` とフィールド

- 別の窓 (`QMainWindow` に `Qt::Dialog`) である。Edit メニューには、Undo/Redo の置き場のコメントだけがある。各スロットは、番号の範囲、一覧、パネルの更新と `editorUpdated()` を、それぞれ繰り返している。
- 番号のスピンボックスの `setRange()` は値を丸めて `valueChanged` を出す。それが `selectData()` → `setCurrentIndex()` を呼び、Player Auto Move がオンならフィールドの 1 組をデータで上書きする。一覧の `updateData()` の行の削除も、`currentItemChanged` → `dataSelected` で同じことを起こす。最後のデータを消すと番号がずれる、などの不具合のもとになっている。
- フィールド (`FieldCanvas`、メインの窓) の操作: 左ボタンで選手とボールのドラッグ、右ボタンでボールの移動、Ctrl+左ドラッグで視点の移動。
  - 左ボタンを押すと、`selectObject()` が、押した位置 (丸めもスナップもしない、最大 1.5 m ずれた位置) をそのまま書く。
  - 離したときはシグナルを出さない。
  - 手の操作はメインの窓で行うので、キーの入力もメインの窓に行く。
- Ball Sync Move がオンなら、フィールドを描くたびに `MainData::syncFormationEditBall()` が `moveBallTo()` を呼ぶ。
- New、Open formation、Clear All は、`FormationEditData` をまるごと作り直す。Open training data は、同じオブジェクトの `openData()` を呼ぶ。

### 2.5 文書、テスト、CI

- FormationEditor のマニュアルはない (README に「Provides a formation editor.」とあるだけ)。`docs/export.md` は英語だけ、`docs/icons.md` と `docs/icons.en.md` は日英。
- テストは、`gui_smoke` の FormationEditor のアイコンの確認と、`field_canvas_render` の「出力でデータが変わらない」だけで、編集のテストはない。
- CI は `develop` と `master` へのプッシュと PR で動く。作業ブランチでは `workflow_dispatch` で手動で走らせる (`docs/plan/2026-11-28-00_frame-export.md` の D6 と同じ)。

## 3. 決定事項

### 3.1 ユーザの決定

| # | 内容 | 決めた日 |
|---|---|---|
| D1 | 取り消せる範囲は、データとロールの変更 (データの追加・挿入・置換・削除・並べ替え、一覧での座標の編集、学習データの読み込み、Apply) と、フィールドの手の操作 (選手とボールのドラッグ (1 回で 1 手)、右ボタンでのボールの移動、ReverseY)。Ball Sync Move による移動と、データの選択 (一覧と番号) は含めない (Claude の案のとおり) | 2026-11-28 (計画の作成の前) |
| D2 | Undo/Redo は Edit メニューとキーだけにし、ツールバーのボタン (新しいアイコン) は置かない (Claude の案のとおり) | 2026-11-28 (計画の作成の前) |
| D3 | マニュアルは、日本語の `docs/formation-editor.md` と英語の `docs/formation-editor.en.md`。節の並びを同じにする (Claude の案のとおり) | 2026-11-28 (計画の作成の前) |
| D4 | マニュアルにスクリーンショットを入れる (`docs/images/`) (Claude の案のとおり) | 2026-11-28 (計画の作成の前) |
| D5 | librcsc は、必要に応じて変えてよい (計画の案を確かめたときのユーザの指示)。入れる変更は S17 | 2026-11-28 (計画の作成の前) |
| D6 | L2 の関数の条件 (正の対は互いを指す) はゆるめない。片方向の対 (2.3) が残らないように、librcsc の読み込みを直す (S17 の L4) | 2026-11-28 (librcsc の前提の作業の中で、ユーザが指示した) |

### 3.2 設計上の判断 (Claude)

計画の確定のときに、ユーザが確かめる。

| # | 内容 | 理由 |
|---|---|---|
| S1 | スナップショット方式: 1 手ごとに、学習データの写し、ロール (名前、種類、対)、フィールドの 1 組、今の番号を積む | Pair Mode で 2 つのデータに触れる操作、置換で足す場合、学習のやり直しなど、逆操作を正しく書くのが難しい。データは最大 128 個 × 12 点 (1 手で約 25 KB) と小さい |
| S2 | 履歴は `FormationEditData` の中に持つ。New、Open formation、Clear All では、作り直すので空になる。Open training data は 1 手 "Open Training Data" として取り消せる | ファイルを開き直したら、前の履歴は意味を持たない。学習データの読み込みは、全データを置き換える破壊的な操作 |
| S3 | 履歴は、Qt を使わない C++ のクラス `FormationEditHistory` (`src/model/formation_edit_history.{h,cpp}`、新規) にする。`QUndoStack` は使わない | model のライブラリは Qt を使わない。窓なしで直接テストできる |
| S4 | 学習データの写しと復元は、`FormationData` のコピー (S17 の L1) で行う。復元の手順: 写しのコピーを学習データにする → ロールを戻す (S5) → 学習と三角形分割をやり直す → フィールドの 1 組と番号を写しのとおりに戻す (`updatePlayerPosition()` は呼ばない) | データを 1 つずつ入れ直す方法は、librcsc の `addData()` の制約に頼ることになる。コピーなら、制約によらず同じものに戻る |
| S5 | ロールの対の番号は、11 人分をまとめて確かめて設定する librcsc の関数 (S17 の L2) で戻す。名前と種類は 1 人ずつ戻す。Apply も、パネルの 11 人分の対を、同じ関数でまとめて設定する (対の入れ替えが効くようになる) | 1 人ずつ設定すると、`setPositionPair()` の制約で、対の入れ替えが黙って失敗する (2.3) |
| S6 | 履歴の上限は 100 手。超えたら古いものから捨てる。新しい編集をすると、Redo の履歴を捨てる | 記憶は最大約 2.5 MB |
| S7 | 1 回の操作は、すべて成功するか、何も変えないかのどちらかにする。記録の始めに写しを取り、操作の一部が失敗したら写しに戻して誤りを出す。`addData()` は最初の誤りを返す。`insertData()` は、Pair Mode でもボールが中央 (\|y\| < 0.5) なら対を入れない (`addData()` と同じ)。Pair Mode の置換で対のデータを足せないときは、置換の全体を誤りにする | 一部だけ成功すると、1 手の内容と画面が食い違う。今の不具合 (2.2) も直る |
| S8 | 1 回の操作を 1 手にする (Pair Mode の 2 つのデータ、Apply の 11 人分のロールと位置)。内容もフィールドも変わらない操作 (失敗した操作、変化のない Apply、Fit Model) は積まない。記録は入れ子にでき (深さを数え、外側の終わりで 1 手にする)、記録の途中の Undo は断る | 1 回の Undo で 1 回の操作が戻る。空の手で Undo が効かないように見えるのを防ぐ。Apply の中でロールの更新が呼ばれる |
| S9 | 変更の有無 (`isModified()`) は、最後に開いた・保存した内容 (データとロール。データの通し番号 `index_` は比べない) と今の内容が違うかで決める。新規は、保存するまで変更あり | Undo で保存した状態に戻ったら、閉じるときに保存を尋ねない。Apply を押すだけで変更ありになる今の動きも直る |
| S10 | Edit メニューの先頭に Undo と Redo を置く (`setShortcuts( QKeySequence::Undo / Redo )`。Linux では Ctrl+Z と Ctrl+Shift+Z)。Ctrl+Y は足さない。項目の名前に、次に取り消す操作の名前を出し ("Undo Add Data")、取り消せないときは無効にする。`objectName` を付ける | 標準のキー。Ctrl+Y は Linux の入力欄が受け取らないので、入力中に Redo が動いてしまう。テストは `findChild()` で取る (public のメンバを増やさない) |
| S11 | ドラッグの間と、一覧の値を編集している間は、Undo/Redo をしない | 途中の状態と履歴が食い違う。開いている編集欄の古い値が、後から書き込まれる |
| S12 | 操作と Undo/Redo の後の表示の更新を 1 つの関数にまとめ、既存のスロットもそれを使う。番号のスピンボックスと一覧のシグナルを止めて (`QSignalBlocker`) から、一覧、番号の範囲と値、パネル、Undo/Redo の項目を更新し、`editorUpdated()` を出す。Undo/Redo の項目は、窓が隠れていても更新する | `setRange()` と一覧の更新が選択のシグナルを出し、戻した状態を上書きする (2.4)。今の番号のずれも直る |
| S13 | 窓のテストは、`test/data/` のフォーメーションの `.conf` を、コマンドラインの `--fedit-conf` と同じ入口で開く (細部は Phase 1 で決める)。テストは、窓を作った後に Pair Mode などの値を決め、終わりに戻す | New はモーダルのダイアログを出す。窓のコンストラクタが、設定から値を読み直す |
| S14 | Phase 2 で、フィールドの操作を次のように記録する。ドラッグは、押したときに記録を始め、離したときに状態が変わっていれば 1 手にする ("Move Ball"、"Move Player 7")。左ボタンを押すだけでは位置を書かない (最初のマウスの移動で、スナップと Pair Mode を通して置く)。離したときにシグナルを出し、Undo の項目を更新する。右ボタンでのボールの移動は、記録するモデルの関数を新しく作って呼ぶ。`moveBallTo()` の中では記録しない。ReverseY は 1 手にする | 動かさないクリック (ダブルクリックを含む) で 1 手が積まれない。`moveBallTo()` は Ball Sync Move と Apply からも呼ばれる (D1) |
| S15 | Phase 2 で、Undo/Redo のキーを、FormationEditor を表示している間はメインの窓 (フィールド) でも効くようにする (`Qt::ApplicationShortcut`)。窓を隠すときは項目を無効にする。入力欄、スピンボックス、一覧の編集欄の中では、それらの Undo が優先する | 手の操作はメインの窓で行う (2.4)。Ctrl+Z は今どの窓でも使っていない (Z はズーム)。メニューバーがネイティブのとき (グローバルメニュー) は、隠れていても項目が見えている扱いになる |
| S16 | マニュアルの例と画面の文言は、実際のアプリで確かめて書く。書く中で見つけた、動きと文言の食い違い (例: Insert Data のステータスの文は「今のデータの後に入れる」だが、実際は前に入る) はユーザに示し、直すか `docs/todo.md` に記録するかを決める。Ball Sync Move がオンのときは、Undo の後もボールがログに合わせて動くことを書く | マニュアルと実物を一致させる |
| S17 | librcsc に入れる変更 (§4 の初め): L1 `FormationData` のコピーのコンストラクタと代入を既定のものにする。L2 `Formation` に、11 人分の対の番号をまとめて確かめて設定する関数を足す (範囲 -1〜11、自分を指さない、正の対は互いを指す。正しくなければ何も変えずに false)。L3 `FormationData` の添字の範囲の誤り (2.3) を直す。L1 と L2 は追加なので、librcsc を使うほかのプログラムには影響しない。<br>2026-11-28 (librcsc の前提の作業の中で) の追加: L4 フォーメーションのファイルから読んだ対の番号を、必ず互いを指す形にする。`FormationParser::checkPositionPair()` で、指されている `-1` を、指している選手の番号に置き換え、L2 の関数で確かめる。JSON の形式のパーサもこれを呼ぶ (D6)。L4 で、`-1` の選手が後にあるファイルの対は互いを指す形で読まれ、対が整合しない JSON のファイルは読めなくなる (helios-base のフォーメーションのファイル 42 個は、変更の前後で同じに読める)。<br>実際の形: L1 は、移動のコンストラクタと代入も既定のものにした。L2 は `bool Formation::setPositionPairs( const std::array< int, 11 > & pairs )` で、誰からも指されない `-1` は認め、指されている `-1` は拒む | L1、L2 で、soccerwindow2 の側の回避策 (データの入れ直し、対を 0 にしてからの入れ直し) が要らなくなる。L3 は調べる中で見つけた不具合で、同じファイルなので一緒に直す。L4: 片方向の対が残ると、写しの対を L2 の関数で戻せない (2.3) |
| S19 | L4 の後は、開いたフォーメーション、New の既定のロール、Apply のどれでも、対の番号は L2 の条件を満たす。Apply の前の窓の整合の確かめ (`checkConsistency()`) は、L2 の条件に左右の条件を加えたもので、今のまま使う。そのため、写しの対の復元 (S5) は失敗しない前提で作り、soccerwindow2 の側で対の番号を整える処理は書かない。復元が失敗したら、内部の誤りとして何も変えない。L4 による読み込みの変化 (`-1` の選手が後にあるファイルは互いを指す形で読まれ、何も変えない Apply が誤りにならない。対が整合しない JSON のファイルは開けない) を、RELEASE_NOTES とマニュアルの「困ったとき」に書く | 写しはどれも L2 の条件を満たすので、整える処理は要らない。読み込みの変化は、使う人に見える |
| S18 | soccerwindow2 は、librcsc の版を `configure` と CMake で調べない。L1、L2 のない古い librcsc ではコンパイルに失敗することを、RELEASE_NOTES と `docs/build.md` に書く | `docs/plan/2026-11-25-00_rcg-load-in-memory.md` の D4 と同じ。文書は既に最新の librcsc (`develop`) を求めている (2.3) |

### 3.3 Phase の初めに決めること

- Phase 1: Undo の項目に出す操作の名前の文言。窓のテストでフォーメーションを開く入口の細部 (S13)。
- Phase 3: スクリーンショットの撮り方 (offscreen で撮る仕組みを作るか、ユーザの画面で撮るか) と、使うフォーメーション。

## 4. 設計

各 Phase の実装の記録 (`#### 4.N.2 実装 (YYYY-MM-DD)`) は、その Phase の実装と同じコミットで足す。soccerwindow2 に依存するリポジトリはないので、追従 Issue は作らない。

librcsc の前提の作業:

- 内容: S17 の L1〜L4 と、librcsc でのテスト。規模が小さいので、librcsc には計画書を作らず、Issue https://github.com/helios-base/librcsc/issues/124 として扱う。L4 は、作業の中で足した (D6)。
- 結果: librcsc の `develop` の `8684789` に取り込まれた。経過は librcsc の Issue に記録されている。
- 進め方: 作業は librcsc のリポジトリで、librcsc の規則に従って行う (Claude が作業範囲への追加を受けるか、librcsc での別のセッションで行う)。
- Issue の結び方: librcsc の Issue を計画 Issue のサブ Issue にし (この計画と一体の作業のため)、Phase 1 の Issue に blocked by を付ける。Phase 1 は、librcsc の Issue が閉じて `develop` に取り込まれてから始める (soccerwindow2 の CI が librcsc の `develop` を使うため)。
- 手元の確かめ: 取り込みの前に、librcsc の作業ブランチをビルドして `~/.local` に入れ、soccerwindow2 の試しのビルドで確かめてよい。Phase 1 の前に、`~/.local` の librcsc が `8684789` 以降であることを確かめる。

### 4.1 Phase 1: 履歴の仕組みと、データとロールの Undo/Redo

#### 4.1.1 設計

- `FormationEditHistory` (新規):
  - 1 手は {名前、写し}。関数は `push()`、`canUndo()`/`canRedo()`、`undoLabel()`/`redoLabel()`、`takeUndo( 今の写し )`/`takeRedo( 今の写し )`、`clear()`、上限 (S6)。
  - 写しの型 `FormationEditState` は、学習データのコピー (`std::shared_ptr< const FormationData >`)、ロールの 3 つの配列、フィールドの 1 組、今の番号を持つ。
  - 内容 (データとロール) を比べる関数も置く (`RoleType` には `==` がないので、種類と左右を比べる)。
- `FormationEditData` に足すもの:
  - 写しを作る関数と戻す関数 (S4、S5)
  - 記録の始めと終わり (入れ子、失敗したら戻す、変わったときだけ積む。S7、S8)
  - `undo()`、`redo()`、`canUndo()` など
  - 保存した内容の写しと、`isModified()` (S9)
- 記録する操作: 追加、挿入、置換、座標の置換、削除、並べ替え、学習データの読み込み。`addData()` と `insertData()` と置換を直す (S7)。
- `FormationEditorWindow::applyToField()`: ロールと位置の変更を 1 手 "Apply" にし、対の番号は S5 の関数でまとめて設定する。
- 窓:
  - Undo/Redo の項目 (S10)
  - 表示の更新の関数 (S12)
  - 一覧の編集中の判定 (`FormationDataView` に小さな関数を足す。S11)
  - テストのための `objectName` (Undo、Redo、Add Data、番号のスピンボックス、一覧)
- ビルド: `src/model/CMakeLists.txt`、`src/model/Makefile.am`、`test/CMakeLists.txt`、ルートの `Makefile.am` の `EXTRA_DIST` (テストのファイルとデータ)。
- `RELEASE_NOTES.md` に日付つきで足す (Undo/Redo、直した不具合、必要な librcsc (`develop` の `8684789` 以降)、L4 による読み込みの変化。S18、S19)。`docs/build.md` に必要な librcsc を書く。

### 4.2 Phase 2: フィールドの操作の Undo/Redo と、メインの窓からのキー

#### 4.2.1 設計

- フィールドの操作の記録 (S14): `selectObject()`、`releaseObject()`、`FieldCanvas` の押す・離すの処理、右ボタンで使う記録するボールの移動の関数、ReverseY。
- キーの文脈と、窓を隠すときの無効化 (S15)。ドラッグ中の判定 (S11)。
- `RELEASE_NOTES.md` に足す。

### 4.3 Phase 3: 操作マニュアル

#### 4.3.1 設計

- `docs/formation-editor.md` (日本語) と `docs/formation-editor.en.md` (英語)。冒頭で互いにリンクする (`docs/icons.md` と同じ)。
- 章:
  - 概要 (考え方: ボールの位置に対する配置、三角形分割、線形補間、ファイル)
  - 起動 (Editor > Formation Editor、Ctrl+Alt+F、`--fedit-conf`/`--fedit-data`/`--fedit-back`/`--fedit-opacity`)
  - 画面の構成 (ツールバー、パネル、データの一覧、フィールド)
  - フィールドでの操作
  - モード (Ball Sync Move、Player Auto Move、Data Auto Select、Pair Mode、Snap とグリッド)
  - データの編集 (Add、Insert、Replace、Delete、並べ替え、一覧での座標の編集、Fit Model)
  - ロールの編集 (Pair、Type、Side、Role Name、Apply、Reset、整合の条件)
  - Undo/Redo
  - 表示 (View)
  - ファイル (New、Open、Save、学習データ、背景のフォーメーション、自動のバックアップ)
  - 作業の流れの例
  - キーの一覧
  - 困ったとき ("Too near data"、"Too many data"、"Inconsistency detected."、対が整合しない JSON のファイルを開けないこと (S19) など)
- スクリーンショットは `docs/images/formation-editor-*.png` (撮り方は §3.3)。
- `README.md` に節とリンクを足し、ルートの `Makefile.am` の `EXTRA_DIST` に文書と画像を足す。

## 5. Phase

どの Phase も、ビルドでき、CI が通る。Phase 1 の前に、librcsc の前提の作業 (§4 の初め) が librcsc の `develop` に入っている。

| # | 作業 | 完了条件 |
|---|---|---|
| 1 | `FormationEditHistory`、`FormationEditData` の記録・復元・`isModified()`、操作の all-or-nothing、Apply の記録と対の番号のまとめての設定、表示の更新の関数、Edit > Undo/Redo | 各操作の Undo で前の状態 (データ、ロール、フィールドの 1 組、番号、学習した配置) に戻り、Redo で後の状態に戻るテストが、CMake の Qt5 と Qt6 で通る。Autotools でもビルドできる。ユーザが画面で確かめる |
| 2 | ドラッグ、右ボタンのボールの移動、ReverseY の記録、メインの窓からのキー | フィールドの操作のテストが通る。ユーザが、フィールドで動かした後、メインの窓で Ctrl+Z を押すと戻ることを確かめる |
| 3 | `docs/formation-editor.md`、`docs/formation-editor.en.md`、スクリーンショット、README | 書いた操作と画面の文言が、実際のアプリと一致する。ユーザが読んで確かめる |

## 6. テスト

- 新しいテストは `soccerwindow2_add_qt_test( NAME )` で足す。手元では `./cmake-helper.sh test qt6` か、ビルドのディレクトリで `ctest -R formation --output-on-failure` で動かせる。
- HOME と XDG_CONFIG_HOME は一時ディレクトリにする。
- Pair Mode、Snap とグリッド、Player Auto Move、Data Auto Select、Ball Sync Move (オフ)、自動のバックアップ (オフ) は、テストの中で値を決める。
- librcsc の L1〜L4 のテストは、librcsc の側で足した (`run_test_formation`)。
- 窓のテストで開くフォーメーションの `.conf` には、`-1` の選手が、それを指す選手より後にある対を 1 組入れる。開いた直後に、何も変えない Apply が誤りを出さず、1 手も積まないことを確かめる (L4、S19)。

| テスト | Phase | 内容 |
|---|---|---|
| `formation_edit_history` | 1、2 | 各操作の Undo と Redo で状態が戻る。Pair Mode の 2 つのデータが 1 手になる。失敗した操作は何も変えず、積まない (近すぎるデータ、中央での Pair Mode の挿入、対を足せない置換)。変化のない操作は積まない。入れ子の記録。Undo の後の新しい操作で Redo が消える。上限。操作の名前。対の入れ替えの復元と Apply。`isModified()` (開く・保存・編集・Undo で保存の状態に戻る・新規)。復元した模型の配置が前と同じ。学習データの読み込みの取り消し。Phase 2: ドラッグ (動かさないクリックは 0 手、Pair Mode で対の選手も戻る)、右ボタン、ReverseY |
| `formation_editor_window` | 1、2 | Edit メニューの先頭の Undo/Redo、キー、有効・無効、項目の名前。Undo/Redo で一覧の数と番号の範囲が戻る。Player Auto Move がオンでも、Undo の後のフィールドの 1 組が上書きされない。New で履歴が消える。開いた直後の何も変えない Apply (`-1` の対を含むファイル)。Phase 2: キーの文脈、窓を隠すと無効、メインの窓からの Undo |

手で確かめること: 実際のフォーメーション (HELIOS-Base の `.conf`) で、データの操作、ロールの変更 (対の入れ替えを含む)、ドラッグを混ぜて Undo/Redo する。保存の後に Undo して閉じるときの確認。Qt5 と Qt6。

## 7. 変更するファイル (見込み)

| ファイル | 内容 |
|---|---|
| `src/model/formation_edit_history.{h,cpp}` (新規) | 履歴と写し |
| `src/model/formation_edit_data.{h,cpp}` | 記録、復元、ロールの設定、`isModified()`、操作の all-or-nothing |
| `src/qt/formation_editor_window.{h,cpp}` | Undo/Redo の項目、Apply、表示の更新 |
| `src/qt/formation_data_view.{h,cpp}` | 編集中の判定、`objectName` |
| `src/qt/field_canvas.cpp` | ドラッグと右ボタンの記録、離したときのシグナル (Phase 2) |
| `src/model/CMakeLists.txt`、`src/model/Makefile.am`、`Makefile.am` | ビルドと配布物 |
| `test/CMakeLists.txt`、`test/test_formation_edit_history.cpp`、`test/test_formation_editor_window.cpp` (新規)、`test/data/` | §6 |
| `docs/formation-editor.md`、`docs/formation-editor.en.md`、`docs/images/` (新規)、`README.md`、`docs/build.md`、`RELEASE_NOTES.md`、`docs/todo.md` | 文書 |
| (librcsc) `rcsc/formation/formation_data.{h,cpp}`、`rcsc/formation/formation.{h,cpp}`、`rcsc/formation/formation_parser.{h,cpp}`、`rcsc/formation/formation_parser_json.cpp` と librcsc のテスト | librcsc の前提の作業 (librcsc の Issue。この計画のコミットには含まない。`8684789` で取り込まれた) |

## 8. 対象外

次は扱わない。`docs/todo.md` に記録した。

- ツールバーの Undo/Redo のボタン (D2)。
- 履歴の一覧の表示 (`QUndoView` のようなもの)。
- 窓のタイトルに、変更があることの印 (`*`) を出すこと。
- 履歴をファイルに残して、次の起動で戻すこと。
- 背景のフォーメーションの読み込みと、表示やモードの切り替えの取り消し。
- Ball Sync Move による移動と、データの選択の取り消し (D1)。
- soccerwindow2 の `configure` と CMake で、librcsc の版や関数を調べること (S18)。

## 9. リスク

- librcsc の前提の作業が `develop` に入るまで、Phase 1 を始められない (CI が librcsc の `develop` を使う)。前提の作業は `8684789` で取り込まれた。
- 古い librcsc ではビルドできなくなる (S18)。RELEASE_NOTES と `docs/build.md` に書く。
- L4 で、対が整合しない JSON のフォーメーションのファイルは、FormationEditor でも開けなくなる (S19)。背景のフォーメーションも同じパーサで読むので、同じく開けない。RELEASE_NOTES とマニュアルに書く。
- 操作を all-or-nothing にすると、今まで一部だけ成功していた操作 (Pair Mode の置換など) が誤りになる (S7)。RELEASE_NOTES とマニュアルに書く。
- 左ボタンを押すだけでは位置を書かなくなる (S14)。クリックで選手を寄せていた使い方が変わる。
- `Qt::ApplicationShortcut` が、ほかの窓の入力とぶつかる可能性がある (S15)。

## 10. 記録

### 2026-11-28: 計画の作成

- FormationEditor のモデル、窓、フィールドの操作、データの一覧、librcsc の `Formation` と `FormationData`、テストと文書を調べ、設計を立てた (§2、§4)。
- 調べて分かった不具合 (2.2〜2.4) を、Undo の 1 手と画面を食い違わせないために、この計画で直すことにした (S5、S7、S12、S17)。
- 計画の作成の前に、ユーザが D1〜D4 を選んだ。計画の案を確かめたときに、ユーザが librcsc を変えてよいと指示し (D5)、librcsc の前提の作業 (S17) を入れた。
- 対象外にしたもの (§8) を `docs/todo.md` に記録した。
- この後: ユーザが計画書を確かめる。確かめられたら、計画 Issue、Phase のサブ Issue、librcsc の Issue の案を示す。

### 2026-11-28: 計画の確定と Issue

- ユーザが GitHub で、`develop` の `206160e` から `feature/formation-undo` を作った。
- ユーザが計画書を確かめた。計画 ID を SW2-02 にした。
- 計画 Issue https://github.com/helios-base/soccerwindow2/issues/109 (ラベル `plan`) と、Phase 1〜3 のサブ Issue https://github.com/helios-base/soccerwindow2/issues/110 〜 https://github.com/helios-base/soccerwindow2/issues/112 を、日本語で作った。
- librcsc の前提の作業の Issue https://github.com/helios-base/librcsc/issues/124 を、librcsc の規則に従って日本語で作り、計画 Issue のサブ Issue にした。タイトルは、規則どおり `[SW2-02] Follow-up: …` にした (前提の作業だが、語は `Follow-up` のままにする。ユーザの指示)。Phase 1 の Issue に、librcsc の Issue への blocked by を付けた。
- 5 つとも常設の Project (helios-base の 9) に追加し、Status を Todo にした。冒頭の表、「Phase と Issue」の表、§4 の初めに URL を書いた。
- この後: librcsc の前提の作業 (https://github.com/helios-base/librcsc/issues/124) を、librcsc のリポジトリで行う。librcsc の `develop` に取り込まれた後、ユーザの確認を受けて Phase 1 を始める。

### 2026-11-29: librcsc の前提の作業 (L4 の追加) の反映

- librcsc の前提の作業は、librcsc での別のセッションで行われ、librcsc の `develop` の `8684789` に取り込まれた。作業の中で、片方向の対の問題 (2.3) が見つかり、ユーザの指示で L4 が足された (D6)。
- librcsc の `develop` のコードを読んで確かめた。
  - L2 は `bool Formation::setPositionPairs( const std::array< int, 11 > & pairs )`。すべての値を確かめてから書き、正しくなければ何も変えずに false を返す。
  - L4 の `FormationParser::checkPositionPair()` は、指されている `-1` を、指している選手の番号に置き換えてから、L2 の関数で確かめる。v1、v2、v3、CSV、JSON の形式のパーサが呼ぶ (static の形式は対を持たない)。
  - 保存 (`Formation::printRoles()`) は、対の番号をそのまま書く。
- soccerwindow2 への影響を調べた。
  - `FormationEditData` は、フォーメーションと背景のフォーメーションを `FormationParser::parse()` で読むので、L4 の整えを受ける。
  - New の既定のロールは互いを指す。窓の `checkConsistency()` は、指されている `-1` を拒む。
  - 以上から、写しの対はいつも L2 の条件を満たすと判断し、S19 にした。
- 反映した節: 冒頭の表、「Phase と Issue」の表の内容、§1、§2.3、D6、S17 (経緯の書き足し)、S19、§4 の初め、§4.1.1、§4.3.1、§6、§7、§9。
- `~/.local` の librcsc に `setPositionPairs()` があることを確かめた。Phase 1 の blocked by (https://github.com/helios-base/librcsc/issues/124) は閉じている。
- この後: ユーザの確認を受けて Phase 1 を始める。
