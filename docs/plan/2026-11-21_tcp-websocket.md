# rcssserver の TCP / WebSocket 接続への対応 計画書

- 作成日: 2026-11-21
- ステータス: 段階 3 を実施済み（ユーザの確認待ち）
- ブランチ: `feature/support-tcp-ws`（案。ユーザが GitHub 上で develop から作成する）
- 参考にする実装: rcssmonitor の PR #68（`feature/support-tcp-ws`、origin/develop の a6df084 にマージ済み）と、その計画書 `docs/plan/2026-11-06_transport.md`
  - 手元の rcssmonitor の作業ツリー（`~/work/src/rcsoccersim/rcssmonitor`）は別ブランチでマージ作業中なので、チェックアウトせず `git show a6df084:<path>` で読む。

## 1. 背景と目的

rcssserver が TCP と WebSocket の接続に対応することが正式に決まり、develop にマージされた（rcssserver 8f1097c、asio への移行）。rcssmonitor も対応済みである。
soccerwindow2 のモニタ機能（`MonitorClient`）は rcssmonitor とほぼ同じ作りなので、rcssmonitor の実装を移植して、soccerwindow2 も UDP / TCP / WebSocket のいずれでも rcssserver に接続できるようにする。

- 既定は従来どおり UDP とし、オプションを指定しない限り挙動を変えない。
- 通信路を「1 メッセージ単位で送受信するトランスポート」として抽象化する（rcssmonitor の `MonitorTransport` をそのまま使う）。
- WebSocket には Qt WebSockets を使い、ビルド時の任意依存とする。見つからなくても UDP / TCP 版はビルドできる。

## 2. 前提: 新サーバの仕様（rcssserver develop 8f1097c）

rcssmonitor の計画書 §2 と、rcssserver の `src/serverparam.cpp` で確かめた。

| 方式 | サーバパラメータ | 既定値 | モニタの接続先 |
|---|---|---|---|
| UDP | `server::port` | 6000 | プレイヤーと共用 |
| TCP | `server::tcp_port` | 0（無効。-1 で `server::port` と同じ番号） | プレイヤーと共用 |
| WebSocket | `server::ws_port` | 0（無効） | パス `/monitor`（モニタ専用）または `/player` |

| 方式 | サーバ → モニタ | モニタ → サーバ |
|---|---|---|
| UDP | 1 データグラム | 1 データグラム |
| TCP | 末尾に必ず `\0` | `\0` または改行で区切る |
| WebSocket | 1 テキストメッセージ（`\0` なし） | 1 メッセージ（末尾の `\0`・改行は除去される） |

- TCP / WebSocket はバイナリを通せないので、`(dispinit version 1)` / `(dispinit version 2)` は拒否される。
- 接続後 10 秒以内に dispinit が必要。`server::max_monitors` を超えると `(error no_more_monitor)` が返る。
- `server::ws_allowed_origins` は Origin ヘッダがあるときだけ検査される（`QWebSocket` は origin を指定しなければ Origin を送らない）。
- TLS（wss://）にはサーバ自体が対応していない。

## 3. 現状の soccerwindow2

| 箇所 | 現状 | 課題 |
|---|---|---|
| `src/qt/monitor_client.{h,cpp}` | `QUdpSocket` を直接持つ。rcssmonitor の変更前とほぼ同じ構造 | 方式を差し替えられない |
| 受信処理 | バージョンごとに同じ形のループが 3 つ。応答元ポートへの追従も重複 | |
| 受信バッファ | `char buf[8192]` に読み、NUL 終端せずに `MainData::receiveMonitorPacket()` に渡す | サーバがデータグラムに `\0` を含めるので動いているだけ。8192 バイト以上は切り捨て（既存の不具合） |
| 接続開始 | コンストラクタで同期 DNS 解決と `bind(0)`、`isConnected()` で成否を判定 | TCP / WebSocket の接続結果は非同期に届く |
| dispinit | `MainWindow::connectMonitorTo()` が生成直後に送る | 接続確立前に送ってしまう |
| 生存確認 | `--wait-seconds`（既定 5、0 で無効）の無通信で `timeout()`。`--auto-quit-mode` ならアプリを終了 | ソケットの切断を検知する経路がない。終了時のログが `M_waited_msec * 1000` 秒と表示される（既存の不具合） |
| 削除 | `timeout()` のスロット内で `MainWindow` が `delete` する | 自分のシグナルの処理中に削除される |
| オプション | `Options` は Qt に依存しない `model` ライブラリにあり、`rcsc::ParamMap` で `--host` / `--port` / `--client-version` などを解析する | rcssmonitor の `Options`（Qt 依存、`MonitorTransport::Type` を直接持つ）をそのまま移せない |
| 設定の保存 | host / port は `~/.soccerwindow2` に保存しない。「Connect to ...」で入力したホストは `M_last_connected_host` に実行中だけ保持 | |
| 「Connect to ...」 | `QInputDialog` でホスト名だけを入力 | 方式・ポート・パスを選べない |
| サーバの起動 | `startServer()` / `restartServer()`（ランチャーダイアログ、メニュー）が rcssserver を起動し、1 秒後に `connectMonitor()`（127.0.0.1）で接続 | TCP / WebSocket ポートは既定で無効なので、そのままでは TCP / WebSocket で接続できない |
| 自動再接続 | なし（rcssmonitor にはある） | |

## 4. 決定事項

ユーザと相談して決めた（2026-11-21）。

1. **オプション名:** `--transport udp|tcp|ws` と `--ws-path PATH` とする。soccerwindow2 の既存の `--host` / `--port` にそろえる（`--server-path` / `--server-pid` は rcssserver のプロセスを指すので、`server-` を付けない）。
2. **設定の保存:** 接続方式・ポート番号・WebSocket のパスは `~/.soccerwindow2` に保存しない。host / port と同じく、コマンドラインと「Connect to ...」ダイアログの値を実行中だけ保持し、再接続とサーバの再起動に使う。
3. **サーバ起動時の引数:** soccerwindow2 から rcssserver を起動するとき、接続方式が TCP なら `server::tcp_port=PORT`、WebSocket なら `server::ws_port=PORT` を引数に自動で加える。引数に同じパラメータが既に含まれていれば加えない。

Claude が決めたこと（rcssmonitor の決定を踏襲したもの、soccerwindow2 に合わせたもの）:

4. **ポート番号:** 全方式で `--port` を共通に使う（既定 6000）。方式ごとの既定値は設けない（新サーバの TCP / WebSocket ポートに既定値がないため）。
5. **WebSocket のパス:** 既定は `/monitor`。
6. **無通信タイムアウト:** TCP / WebSocket でも `--wait-seconds` の生存確認を残す（サーバが固まった場合はタイムアウトでしか気づけない）。
7. **切断時の動作:** トランスポートの切断（サーバの終了、接続の拒否など）は、タイムアウトと同じに扱う。`--auto-quit-mode` ならアプリを終了し、そうでなければ切断してステータスバーと標準エラーに理由を出す。TCP / WebSocket では試合終了後のサーバの終了がすぐに分かるので、自動終了も待たずに行われる。
8. **プロトコルのバージョン:** TCP / WebSocket で v1 / v2 が指定されたら、接続前にエラーにする。範囲外のバージョンは、従来どおり接続失敗とする（rcssmonitor のように丸めない）。
9. **トランスポートの種類の持ち方:** `model` は Qt に依存させないので、`Options` は正規化した名前の文字列（`"udp"` / `"tcp"` / `"ws"`）を持つ。大文字小文字を区別せず、`websocket` も `ws` の別名として受け付け、不正な値は `parseCmdLine()` でエラーにする。`qt` 側は rcssmonitor と同じ `MonitorTransport::Type` と `parse_monitor_transport_type()` を使って変換する（トランスポートのコードを rcssmonitor と同じに保ち、今後の修正を移しやすくする）。
10. **計画書は日本語版のみ**（CLAUDE.md に従う。rcssmonitor のような英語版は作らない）。

## 5. 設計

### 5.1 構成

```
MainWindow
  └── MonitorClient            dispinit、コマンド生成、受信の振り分け、生存確認
        └── MonitorTransport   抽象クラス: 1 メッセージ単位の送受信
              ├── UdpMonitorTransport   QUdpSocket（応答元ポートへの追従を含む）
              ├── TcpMonitorTransport   QTcpSocket + MessageSplitter
              └── WsMonitorTransport    QWebSocket（HAVE_QT_WEBSOCKETS のときだけ）
```

### 5.2 rcssmonitor からほぼそのまま移すもの（`src/qt/` に置く）

| ファイル | 変更点 |
|---|---|
| `monitor_transport.{h,cpp}` | インクルードガードの名前（`SOCCERWINDOW2_QT_...`）と著作権表示のみ |
| `udp_monitor_transport.{h,cpp}` | 同上 |
| `tcp_monitor_transport.{h,cpp}` | 同上。Qt6 対応済みのもの（`errorOccurred()` を関数でつなぐ）を使う |
| `ws_monitor_transport.{h,cpp}` | 同上。QWebSocket のエラーシグナルは Qt 6.5 以上で `errorOccurred()`、それより前は `error()` |
| `message_splitter.{h,cpp}` | 同上（Qt のソケットに依存しない） |
| `connect_dialog.{h,cpp}` | ホスト・方式・ポート・WebSocket のパスを入力するダイアログ。WebSocket 非対応ビルドでの表示を確かめる |

移植の際は rcssmonitor の a6df084 の版を元にし、差分は上記の名前と soccerwindow2 のコーディングスタイルに限る。

### 5.3 MonitorClient（soccerwindow2 用に書き換える）

```cpp
MonitorClient( QObject * parent,
               MainData & main_data,
               MonitorTransport * transport, // 所有権は MonitorClient に移る
               const int version );

bool start( const QString & host, quint16 port, QString * error );
void disconnect();
bool isConnected() const;   // transport->isOpen()
QString peerName() const;   // 例: "tcp://127.0.0.1:6000"

signals:
    void received();
    void disconnectRequested();               // 旧 timeout() を改名
    void errorMessage( const QString & msg ); // 新規。ステータスバー表示用
```

- **開始:** `start()` でバージョンの範囲と、トランスポートとの組み合わせ（TCP / WebSocket で v1 / v2 は不可）を検査し、`open()` を呼ぶ。生存確認のタイマはここで始める（TCP の接続待ちもこのタイマで打ち切れる）。
- **dispinit:** トランスポートの `connected()` を受けてから送る。`MainWindow` からの直接呼び出しはやめる。
- **受信:** `messagesAvailable()` で `popMessage()` を全件取り出し、1 件ずつ振り分ける。1 件以上あればタイマをリセットし、`received()` を 1 回出す。
  - 接続後の最初のメッセージが `(error` で始まるときは接続失敗とし、`errorMessage()` を出して切断を要求する。2 件目以降の `(error ...)` はログに出すだけ。
  - JSON（-1）と v3 以上は `MainData::receiveMonitorPacket( msg.constData(), version )`。
  - v2 / v1 は、ゼロ初期化した `rcsc::rcg::dispinfo_t2` / `dispinfo_t` に `min( size, sizeof )` バイトをコピーして渡す。
- **コマンド送信:** 既存の `sendKickOff()` などはそのまま残す。`sendCommand( MonitorCommand / TrainerCommand )` は `toCommandString()` で作った文字列を `transport->sendMessage()` に渡す（区切り文字はトランスポートが付ける）。
- **生存確認:** `--wait-seconds` > 0 で、その秒数メッセージがなければ、`--auto-quit-mode` なら終了、そうでなければ `errorMessage()` と `disconnectRequested()`。終了時のログの秒数の誤りを直す。
- **切断:** トランスポートの `disconnected( reason )` を受けたら、生存確認のタイムアウトと同じ処理をする（決定事項 7）。接続前の失敗と接続後の切断で文言を分ける。
- **終了:** `disconnect()` はタイマを止め、トランスポートからの通知を切り、接続中なら `(dispbye)` を送ってから `close()` する。

### 5.4 MainWindow

- `connectMonitorTo( const char * hostname )`:
  1. `Options` の方式とパスから `create_monitor_transport()` でトランスポートを作る。失敗したら（WebSocket 非対応ビルドなど）理由を表示して終える。
  2. `MonitorClient` を作り、シグナルをつないでから `start()`。false なら理由を表示してクライアントを削除する。
  3. 成功したら `M_last_connected_host` を更新し、データの消去、アクションの有効化、デバッグサーバの再起動、ライブモードへの切り替えを従来どおり行う。`sendDispInit()` の呼び出しは削除する。
  4. 「Connect to ...」の表示とログに `peerName()` を使う。
- `disconnectMonitor()`: シグナルを切ってから `disconnect()` し、`delete` を `deleteLater()` に替える（自分のスロットから呼ばれるため）。シグナルは直接接続のままにする。
- 新しいスロット `showMonitorError( const QString & )`: ステータスバーに 5 秒表示し、標準エラーにも出す（`--hide-status-bar` でも分かるように）。
- `connectMonitorTo()`（「Connect to ...」）: `QInputDialog` を `ConnectDialog` に替える。初期値は `M_last_connected_host` と `Options` の方式・ポート・パス。OK なら `Options` の setter に値を入れてから接続する（実行中だけ保持。決定事項 2）。
- `startServer()`: 接続方式が TCP / WebSocket なら、`M_server_args` に `server::tcp_port=PORT` / `server::ws_port=PORT` を加える（決定事項 3）。引数の書式（`=` でつなぐ 1 語でよいか）は rcssserver で確かめる。起動 1 秒後の `connectMonitor()` で TCP の接続が拒否される場合の扱いは、確認の後に決める（§10）。

### 5.5 Options（`src/model/options.{h,cpp}`）

| 設定 | コマンドライン | 既定値 | アクセサ |
|---|---|---|---|
| 接続方式 | `--transport udp\|tcp\|ws` | `udp` | `transport()` / `setTransport()` |
| WebSocket のパス | `--ws-path PATH` | `/monitor` | `wsPath()` / `setWSPath()` |
| ポート番号 | `--port PORT`（既存） | 6000 | `port()` / `setPort()`（setter を追加） |

- 「Monitor Client Options」に加える。`--help` の説明に、`--port` は方式ごとに `server::port` / `server::tcp_port` / `server::ws_port` を指定すること、`--transport` の tcp / ws は client version 3 以上か -1 が必要なこと、WebSocket 非対応ビルドでは ws が使えないこと（`config.h` の `HAVE_QT_WEBSOCKETS` で切り替え）を書く。
- `--client-version` の説明に「-1 は JSON」を加える。

### 5.6 ビルドシステム

**CMake**
- ルートの `CMakeLists.txt`: `option(ENABLE_WEBSOCKET ... ON)`。ON なら `find_package(Qt${QT_VERSION_MAJOR} ${SOCCERWINDOW2_QT_MIN_VERSION} QUIET COMPONENTS WebSockets)`、見つかれば `HAVE_QT_WEBSOCKETS`、見つからなければ WARNING。`configure_file()` より前に置き、確認の表示に結果を出す。
- `cmake-config.h.in`: `#cmakedefine HAVE_QT_WEBSOCKETS`。
- `src/qt/CMakeLists.txt`: 新しいソースを `soccerwindow2_gui` に加え、`HAVE_QT_WEBSOCKETS` のときだけ `ws_monitor_transport.cpp` と `Qt::WebSockets` を加える。

**Autotools**
- `m4/sw2_check_qt.m4`: `SW2_CHECK_QT` に任意モジュールの 4 番目の引数を加え、qmake のプロジェクトに `qtHaveModule(websockets): QT += websockets` を書く。
- `configure.ac`: `AC_ARG_ENABLE([websocket])`（既定 yes、`--disable-websocket` で無効）。有効なら任意モジュールに websockets を渡し、`QT_CXXFLAGS` で `QWebSocket` ヘッダを確かめて `HAVE_QT_WEBSOCKETS` を定義、`AM_CONDITIONAL([HAVE_QT_WEBSOCKETS])`。
- `src/qt/Makefile.am`: 新しいソース・ヘッダ・moc を一覧に加え、WebSocket の実装と moc は条件付きで加える（条件付きのソースも `make dist` には入る）。

**CI（`.github/workflows/build.yml`）**
- CMake のジョブ: Qt WebSockets の開発パッケージを入れる（Qt5: `libqt5websockets5-dev`、Qt6: 22.04 は `libqt6websockets6-dev`、24.04 は `qt6-websockets-dev`）。`config.h` に `HAVE_QT_WEBSOCKETS` があることを確かめる。
- Autotools のジョブ: 24.04 は WebSockets あり（検出できることを確かめる）、22.04 は WebSockets なし（WebSocket のない形がコンパイルできることを確かめる）。

## 6. 変更するファイル

新規: `src/qt/monitor_transport.*`、`udp_monitor_transport.*`、`tcp_monitor_transport.*`、`ws_monitor_transport.*`、`message_splitter.*`、`connect_dialog.*`、`test/test_message_splitter.cpp`、`test/test_monitor_transport.cpp`、`test/test_monitor_client.cpp`、`test/test_monitor_client_live.cpp`

変更:

| ファイル | 内容 |
|---|---|
| `src/qt/monitor_client.{h,cpp}` | トランスポートの利用、受信処理の統合、接続の流れ、切断の扱い |
| `src/qt/main_window.{h,cpp}` | 接続の非同期化、`deleteLater()`、エラー表示、`ConnectDialog`、サーバ起動時の引数 |
| `src/model/options.{h,cpp}` | `--transport`、`--ws-path`、setter |
| `CMakeLists.txt`、`cmake-config.h.in`、`src/qt/CMakeLists.txt`、`test/CMakeLists.txt` | CMake |
| `configure.ac`、`m4/sw2_check_qt.m4`、`src/qt/Makefile.am`、`Makefile.am`（`EXTRA_DIST` にテストのソース） | Autotools |
| `.github/workflows/build.yml` | CI |
| `test/test_gui_smoke.cpp`、`test/smoke_help.sh` | 既存テストへの追加 |
| `README.md`、`docs/build.md`、`RELEASE_NOTES.md` | 文書 |

## 7. 実装手順

各段階で動く状態を保ち、段階ごとにユーザの確認を受けてコミットする（種類ごとにコミットを分ける）。プッシュは全段階の後（または区切りのよいところ）で行う。

### 段階 1: UDP のまま抽象化する
- `monitor_transport` と `udp_monitor_transport` を加え、`MonitorClient` をトランスポート経由に書き換える（受信ループの統合、`start()`、`connected()` を受けての dispinit、`disconnectRequested()` / `errorMessage()`、受信バッファの不具合と終了時のログの修正）。
- `MainWindow` の接続処理を対応させる（`deleteLater()`、エラー表示）。
- CMake と Autotools に新しいファイルを加える。
- 手元の結合テスト `monitor_client_live`（§8.3）を UDP の分だけ作る（段階 5 の予定を前倒しした。段階 2 / 3 で TCP / WebSocket を足す）。
- 完了条件: 既存のテストが通る。rcssserver（develop）に UDP で接続し、JSON / v3 / v4 / v5 の表示、コマンド送信、`--wait-seconds` での切断と `--auto-quit-mode` が従来どおり動く。

### 段階 2: TCP と接続方式のオプション
- `message_splitter` と `tcp_monitor_transport` を加える。
- `--transport` / `--ws-path` と setter を加える（この段階では ws はエラー）。v1 / v2 の検査を入れる。
- `startServer()` に TCP 用の引数の追加を入れる（WebSocket 用の `server::ws_port` も同じ関数で加えるので、この段階で入れた）。
- テスト: `message_splitter`、`monitor_transport`（TCP の部分）、`monitor_client`、`smoke_help` への追加、`monitor_client_live` への TCP の追加。
- 完了条件: テストが通り、TCP で §8.4 の確認項目が通る。

### 段階 3: WebSocket
- `ws_monitor_transport` と、CMake / Autotools の Qt WebSockets の検出を加える。`startServer()` の WebSocket 用の引数（段階 2 で入れた）で接続できることを確かめる。
- テスト: `monitor_transport`（WebSocket の部分、非対応ビルドでの理由の返却）、`monitor_client_live` への WebSocket の追加。CI のパッケージと確認を更新する。
- 完了条件: WebSockets あり / なし（`-DENABLE_WEBSOCKET=OFF`、`--disable-websocket`）の両方で Qt5 / Qt6 のビルドとテストが通る。WebSocket で §8.4 の確認項目が通る。非対応ビルドで `--transport ws` を指定するとエラーが表示される。

### 段階 4: 「Connect to ...」ダイアログ
- `connect_dialog` を加え、`QInputDialog` を置き換える。GUI スモークテストの `openModalDialog` に `connect_monitor_to` を加える。
- 完了条件: ダイアログで方式を選んで接続でき、キャンセルで何も起きない。WebSocket 非対応ビルドの表示が分かりやすい。

### 段階 5: 文書
- README（使い方、サーバ側の設定例）、`docs/build.md`（パッケージ、`ENABLE_WEBSOCKET`、`--disable-websocket`、手元の結合テストの動かし方）、`RELEASE_NOTES.md`（日付付き）を更新する。
- 完了条件: README の手順どおりに TCP / WebSocket で接続できる。

## 8. テスト計画

### 8.1 単体テスト（CMake、Qt Test、CI で実行）
- `message_splitter`（rcssmonitor から移植）: 分割されて届く、1 回に複数、`\0` の連続（空は捨てる）、上限超過。
- `monitor_transport`（rcssmonitor から移植）: 手元の `QTcpServer` / `QWebSocketServer` につなぎ、送信の区切り、受信の分割、切断の通知が 1 回だけ、`close()` 時に最後のメッセージが届く、接続失敗の理由、WebSocket のパス `/monitor` と Origin なし、非対応ビルドでの理由。名前でつないだシグナルの警告もテストの失敗にする。
- `monitor_client`（soccerwindow2 で新規）: 偽のサーバ（`QTcpServer`）と `MainData` を使い、接続後に `(dispinit version 5)\0` が届く、show のメッセージが `MainData` に渡り `received()` が出る、最初の `(error no_more_monitor)` で `errorMessage()` と `disconnectRequested()`、サーバが閉じると `disconnectRequested()` が 1 回、TCP で v1 / v2 は `start()` が理由を返す、キックオフで `(dispstart)\0`、`disconnect()` で `(dispbye)` が届く。
- テストは一時的な HOME で動かす（`~/.soccerwindow2` を読み書きしない）。

### 8.2 既存テストへの追加
- `gui_smoke`: 「Connect to ...」でダイアログが開き、`QObject::connect` の警告が出ない。
- `smoke_help`: `--help` に `--transport` と `--ws-path` が出る。

### 8.3 手元の結合テスト（ユーザが実行できるもの、CI では skip）
- `monitor_client_live`: 環境変数 `RCSSSERVER` に rcssserver（develop）の実行ファイルを指定すると、一時的な `RCSS_CONF_DIR` とふだんと違うポート（UDP 16000、TCP 16100、WebSocket 16180）でサーバを起動し、方式と版の組み合わせ（UDP は -1 / 5 / 4 / 3 / 2 / 1、TCP / WebSocket は -1 / 5 / 4 / 3）のそれぞれで、接続、フレームの受信、コマンド（`change_mode`）でのプレイモードの変化、dispbye を確かめる。2 つ目のモニタが `(error no_more_monitor)` で断られることも確かめる。`RCSSSERVER` がなければ終了コード 77 で skip（`SKIP_RETURN_CODE`）。
  - サーバは `server::max_monitors=1` で起動する。dispbye が届かないと、次の接続が断られて失敗する（dispbye の確認を兼ねる）。
  - サーバは `server::coach_w_referee=true` で起動する。サーバはコーチモードのときだけモニタからのトレーナーコマンド（`change_mode` など）を受け付ける（rcssserver の `Monitor::parseCommand()`）。
  - キックオフ前はサーバの時刻が進まず、`ViewHolder` は before_kick_off のフレームを最後の 1 つしか残さないので、フレームの受信は `received()` の回数で確かめる。
  - 実行例: `RCSSSERVER=/path/to/rcssserver ctest --test-dir build-qt6 -R monitor_client_live --output-on-failure`
- rcssserver の develop は scratchpad でビルドする（手元の `~/work/src/rcsoccersim/rcssserver/src/rcssserver` は develop のマージ前のビルドなので使わない）。

### 8.4 画面での確認項目（ユーザが行う）

| # | 項目 | UDP | TCP | WebSocket |
|---|---|---|---|---|
| 1 | JSON / v3 / v4 / v5 で試合が表示される | ○ | ○ | ○ |
| 2 | v1 / v2 を指定したとき | 従来どおり | 接続前にエラー | 接続前にエラー |
| 3 | キックオフ、ドロップボール、フリーキック、カード、選手のドラッグ移動、トレーナーダイアログのコマンドがサーバに届く | ○ | ○ | ○ |
| 4 | サーバ未起動のときエラーが表示される | 待ち秒数の後 | すぐに | すぐに |
| 5 | 試合中にサーバを止めると切断として扱われる | 待ち秒数の後 | すぐに | すぐに |
| 6 | `--auto-quit-mode` で試合終了後に終了する | ○ | ○ | ○ |
| 7 | 切断操作の後、サーバのログでモニタの離脱が確認できる | ○ | ○ | ○ |
| 8 | `server::max_monitors` を超えるとエラーが表示される | ○ | ○ | ○ |
| 9 | 不正なトレーナーコマンドへの `(error ...)` では切断されない | ○ | ○ | ○ |
| 10 | ランチャーダイアログ・サーバの再起動から起動したサーバに接続できる（引数の自動追加） | ○ | ○ | ○ |
| 11 | Kill server が従来どおり動く | ○ | ○ | ○ |
| 12 | デバッグサーバ（`-d`）とタイムシフト再生に影響がない | ○ | ○ | ○ |
| 13 | `--ws-path /player` でも接続でき、存在しないパスではエラー | - | - | ○ |
| 14 | `server::ws_allowed_origins` を設定したサーバにも接続できる | - | - | ○ |
| 15 | 「Connect to ...」ダイアログで方式を選んで接続できる | ○ | ○ | ○ |
| 16 | ログファイルの再生に影響がない | ○ | - | - |

画面の確認で soccerwindow2 を起動するときも、一時的な HOME を使う（`--help` だけでも `~/.soccerwindow2` が書き換わるため）。

送信内容は nc でも確かめられる: `nc -l 6000 | xxd` と `soccerwindow2 -c --transport tcp --port 6000` で、`(dispinit version 5)` の後に `00` が続く。

## 9. 対象外

- wss://（TLS）: サーバが対応していない。
- デバッグサーバ（`DebugServer`、エージェントからの UDP）: rcssserver との接続とは別のプロトコルなので変えない。
- 自動再接続モード: soccerwindow2 にはもともとない。
- URL 形式のオプション（`--server tcp://host:6000` など）。
- 接続設定の `~/.soccerwindow2` への保存（決定事項 2）。
- 圧縮: TCP / WebSocket ではサーバ側でも使えない。

## 10. 未決事項

| 項目 | 案 |
|---|---|
| （なし） | |

段階 2 で解決したもの（2026-11-21）:

- **サーバ起動の 1 秒後の TCP 接続:** 手元では、`startServer()` で起動したサーバに 1 秒後の接続が間に合った（§11 の段階 2）。再試行は入れない。WebSocket も段階 3 で間に合うことを確かめた。遅いマシンで拒否される場合は、接続の失敗としてステータスバーに理由が出るので、手で接続し直せる。
- **`server::tcp_port=PORT` の引数の書式:** `=` でつないだ 1 語で、rcssserver に受け付けられた。

## 11. 記録

区切りごとに、日付、決めたこと、作ったもの、見つけた不具合とその原因、試験の結果を書き足す。

### 2026-11-21: 計画の作成
- rcssmonitor の PR #68 と計画書、rcssserver develop（8f1097c）の `serverparam.cpp`、soccerwindow2 の `MonitorClient` / `MainWindow` / `Options` / ビルドシステム / CI を調べて計画を作った。
- ユーザと決定事項 1〜3 を決めた。
- 手元には Qt WebSockets の開発パッケージ（Qt 5.15.13、Qt 6.4.2）が入っている。

### 2026-11-21: 段階 1（UDP のまま抽象化する）

**作ったもの**
- `src/qt/monitor_transport.{h,cpp}`、`src/qt/udp_monitor_transport.{h,cpp}`: rcssmonitor の a6df084 の版から移した（インクルードガードの名前と著作権表示だけを変えた）。`create_monitor_transport()` は、rcssmonitor の段階 1（af2e1af）と同じく UDP だけを作り、TCP / WebSocket は "Unsupported transport type." を返す。
- `MonitorClient`: コンストラクタはトランスポートを受け取るだけにし、`start()` でバージョンの範囲（-1、1〜5）と、トランスポートとの組み合わせを検査してから接続を始める。dispinit はトランスポートの `connected()` を受けて送る。受信ループを 1 つにまとめ、最初のメッセージが `(error ...)` なら接続失敗とする。`timeout()` を `disconnectRequested()` に改名し、`errorMessage()` を加えた。タイムアウトとトランスポートの切断は `requestDisconnect()` にまとめ、`--auto-quit-mode` なら `QCoreApplication::exit()` を呼ぶ（従来と同じく、その後に `disconnectRequested()` も出して dispbye を送る）。シグナルとスロットは関数でつなぐ（名前の間違いをビルドで見つけるため）。
- `MainWindow`: `connectMonitorTo()` はトランスポートを作り、シグナルをつないでから `start()` する。失敗の理由は `showConnectionFailure()` で標準エラーとステータスバーに出し、`startServer()` が設定した待ちカーソルを戻す。接続中に呼ばれたら先に切断する。`disconnectMonitor()` は `deleteLater()` にした。`showMonitorError()` スロットで、`MonitorClient` からのエラーをステータスバーに出す。この段階では方式は UDP に固定。
- 直した既存の不具合: UDP の受信で NUL 終端していなかった（`pendingDatagramSize()` で確保する UDP の通信路に移して直った）。自動終了のときのログの秒数が 1000 倍だった（`No message from the server for N seconds.` に変えた）。
- `test/test_monitor_client_live.cpp`（テスト `monitor_client_live`）: §8.3 の結合テストの UDP の分。テストを足すための関数 `soccerwindow2_add_qt_test()` を `test/CMakeLists.txt` に作った。ソースは `Makefile.am` の `EXTRA_DIST` にも加えた。

**試験**
- CMake（Qt 6.4.2）、CMake（Qt 5.15.13、`-Werror=deprecated-declarations`）で、ビルドとテストが通る（`monitor_client_live` は `RCSSSERVER` がなければ skip）。
- Autotools（Qt 5.15.13）で、ビルド、`make check`、`make distcheck` が通る。作業ツリーは in-tree で configure 済みなので、scratchpad に写してビルドした。
- rcssserver の develop（8f1097c）を scratchpad でビルドし、`monitor_client_live` を実行した。UDP で版 -1 / 5 / 4 / 3 / 2 / 1 のすべてが通った（サーバのログで、接続と dispbye による離脱を版ごとに確かめた）。
- 実行ファイル（一時的な HOME、オフスクリーン）で確かめた: サーバがないとき `--wait-seconds 2 --auto-quit-mode` で 2.1 秒後に終了する。`--auto-quit-mode` なしでは理由を出して切断し、終了しない。接続中にサーバを止めると、2 秒後に切断して終了する。

**分かったこと**
- 手元の PATH の先頭に anaconda の Qt 5.15.2（`~/anaconda3/envs/catboost`）があり、`./cmake-helper.sh build qt5` はこれを使う。システムの Qt5 で確かめるには `CMAKE_ARGS="-DQt5_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt5"` を付ける（Autotools は `QMAKE=/usr/lib/qt5/bin/qmake`）。段階 3 の Qt WebSockets の検出で注意する。
- 新しいサーバの `server_param` には librcsc の知らないパラメータ（`tcp_port`、`ws_port` など）があり、接続のたびに librcsc が `Unsupported parameter.` を標準エラーに出す。表示に害はない。librcsc 側の課題とする（この計画では扱わない）。

### 2026-11-21: 段階 2（TCP と接続方式のオプション）

**作ったもの**
- `src/qt/message_splitter.{h,cpp}`、`src/qt/tcp_monitor_transport.{h,cpp}`: rcssmonitor の a6df084 の版から移した（インクルードガードの名前と著作権表示だけを変えた）。`create_monitor_transport()` は TCP も作り、WebSocket には "The WebSocket transport is not available in this build." を返す。
- `Options`: `--transport`（`setTransport()` で大文字小文字を区別せずに検査し、`websocket` は `ws` にする。不正な値は `parseCmdLine()` がエラーにして終了）と `--ws-path`、`setPort()`、`setWSPath()` を加えた。`--port` と `--client-version` の説明を直した。`--help` の `--transport` の説明は、`HAVE_QT_WEBSOCKETS` がないと「ws is not available in this build」と出す。
- `MainWindow`: 方式とパスを `Options` から取る。`startServer()` は、方式が TCP なら `server::tcp_port=PORT`、WebSocket なら `server::ws_port=PORT` を引数に加える（`add_transport_port_arg()`。引数に同じパラメータがあれば加えない）。WebSocket の分も同じ関数なので、段階 3 を待たずに入れた。
- テスト（`soccerwindow2_add_qt_test()` で登録）:
  - `message_splitter`、`monitor_transport`: rcssmonitor から移した。`monitor_transport` は WebSocket なしのビルドなので、TCP の 3 項目と、WebSocket の通信路を作らずに理由を返すこと（`noWebSocket`）を確かめる。
  - `monitor_client`（新規）: 偽の TCP サーバ（`QTcpServer`）と UDP サーバ（`QUdpSocket`）で、§8.1 の項目を確かめる。show のメッセージは rcssserver の v5 の出力と同じ形をテストの中で作る。`--wait-seconds` のタイムアウトは、`Options::parseCmdLine()` で 1 秒にして確かめ、5 秒に戻す。UDP では、サーバが別のポートから返信すると、コマンドと dispbye がそのポートに送られることを確かめる。
  - `smoke_help`: `--help` に `--transport` と `--ws-path` が出ることを確かめる。
  - `monitor_client_live`: TCP（版 -1 / 5 / 4 / 3）と、TCP での no_more_monitor を加えた。

**直したこと**
- `monitor_client` のテストの `QUdpSocket::bind( QHostAddress::LocalHost, 0 )` が、Qt5 ではあいまいな呼び出しになってビルドできなかった（Qt6 では通る）。`QHostAddress( QHostAddress::LocalHost )` にした。

**試験**
- CMake（Qt 6.4.2）、CMake（Qt 5.15.13、`-Werror=deprecated-declarations`）で、ビルドとテスト 8 個が通る（`monitor_client_live` は skip）。
- Autotools（Qt 5.15.13）で、ビルド、`make check`、`make distcheck` が通る。配布物に新しいソースとテストが入っている。
- `monitor_client_live`: Qt6 と Qt5 で、UDP（-1 / 5 / 4 / 3 / 2 / 1）、TCP（-1 / 5 / 4 / 3）、UDP と TCP の no_more_monitor の 14 項目が通る。
- 実行ファイル（一時的な HOME、オフスクリーン）で確かめた:
  - `--transport foo` はエラーで終了する（終了コード 1）。`--transport WebSocket` は受け付け、接続の時に「WebSocket は使えない」と表示する。`--transport TCP --client-version 2` は、接続の前に理由を表示する。
  - TCP で接続中にサーバを止めると、止めた直後に切断して自動終了する（接続から 3 秒でサーバを止め、3.0 秒で終了した。UDP では `--wait-seconds` の後）。理由は `Disconnected from the server: The remote host closed the connection`。
- サーバの起動: `MainWindow::startServer()` を呼ぶ確認用のプログラムを scratchpad で作り（リポジトリには入れない）、rcssserver を PID を記録するラッパー経由で起動した。TCP では引数に `server::tcp_port=16100` が加わり、1 秒後の接続でモニタとして接続できた。UDP では引数は加わらない。ランチャーと同じ `restartServer( command, args )` で、引数に `server::tcp_port=16100` があるときは重複して加えない。確認用のプログラムは、終了時に `~MainWindow()` が `killall rcssserver` を実行しないように、`Options::setKillServer( false )` を呼んでから終了した（サーバの PID が分からないとき、soccerwindow2 は `killall -SIGINT rcssserver` で利用者のすべての rcssserver を止める。従来からの動作で、この計画では変えない）。

### 2026-11-21: 段階 3（WebSocket）

**作ったもの**
- `src/qt/ws_monitor_transport.{h,cpp}`: rcssmonitor の a6df084 の版から移した。`monitor_transport.cpp` も rcssmonitor の最終版と同じ形になり、移した通信路のファイルはすべて、名前と著作権表示を除いて rcssmonitor と同じ。
- CMake: `ENABLE_WEBSOCKET`（既定 ON）。`Qt${QT_VERSION_MAJOR}` の WebSockets を探し、見つかれば `HAVE_QT_WEBSOCKETS` を `config.h` に書き、`ws_monitor_transport.cpp` と `Qt::WebSockets` を `soccerwindow2_gui` に加える。見つからなければ WARNING を出して WebSocket なしでビルドする。configure の表示に `WebSocket connection (Qt WebSockets): TRUE/FALSE` を出す。
- Autotools: `SW2_CHECK_QT` に「あれば使うモジュール」の 4 番目の引数を加えた（qmake のプロジェクトに `qtHaveModule(websockets): QT += websockets` を書く）。`configure.ac` に `--disable-websocket` を加え、`QT_CXXFLAGS` で `QWebSocket` ヘッダがコンパイルできれば `HAVE_QT_WEBSOCKETS` を定義し、`src/qt/Makefile.am` で WebSocket の通信路を条件付きでビルドする。
- CI: CMake のジョブと Ubuntu 24.04 の Autotools のジョブに Qt WebSockets の開発パッケージを入れる（Qt5: `libqt5websockets5-dev`、Qt6: 22.04 は `libqt6websockets6-dev`、24.04 は `qt6-websockets-dev`）。Ubuntu 22.04 の Autotools のジョブには入れず、WebSocket なしの形もビルドする。ビルドの後に `test/check_websocket.sh` で、`config.h` の `HAVE_QT_WEBSOCKETS` が期待どおりかを確かめる。
- `monitor_client_live`: WebSocket（版 -1 / 5 / 4 / 3、パス `/player`）、WebSocket での no_more_monitor、存在しないパスが接続の失敗になること（`wsWrongPath`）、`server::ws_allowed_origins` を設定したサーバで、別の Origin のクライアントは断られること（`wsOtherOriginRefused`。Origin を送らない通信路が受け付けられることは `connectAndCommand` で確かめる）を加えた。テスト用のサーバは `server::ws_allowed_origins="http://localhost:8000"` で起動する（rcssserver のコマンドラインでは、文字列を引用符で囲む必要がある）。

**見つけた不具合と直したこと**
- CMake のビルドで、GUI のソースが古い `config.h` を読んでいた（以前からの不具合）。`soccerwindow2_gui` はソースのトップディレクトリをインクルードパスに入れていて、`build` より先に探される。手元の作業ツリーには、in-tree の Autotools が 2026-07-11 に作った `config.h` があり、`HAVE_QT_WEBSOCKETS` がないので WebSocket の通信路が作られなかった（`monitor_transport` のテストで見つかった）。ソースのトップにあるヘッダを読むソースはないので、このインクルードを外した。CI のようにきれいなチェックアウトでは起きない。
- `--disable-websocket` で configure の結果が壊れた（この段階で入れた不具合）。`AC_CHECK_HEADER` をシェルの `if` の中に書いたため、それが前提とする検査（`AC_INCLUDES_DEFAULT` のヘッダの検査）も `if` の中に展開され、無効のときに飛ばされた。後の型の検査が誤った結果になり、`config.h` に `pid_t` などの定義が入ってビルドできなかった。`AS_IF` で書き直した（前提の検査が条件の外に展開される）。
- 結合テストで、WebSocket ではフレームの受信を確かめる前に条件を満たしていた。WebSocket は 1 メッセージごとに `received()` が出るので、パラメータのメッセージ（約 20 件）だけで「5 回」に達していた。フレームが届く（`monitorViewCont()` が空でない）まで待つ形にした。

**試験**
- CMake（Qt 6.4.2）、CMake（Qt 5.15.13、`-Werror=deprecated-declarations`）で、ビルドとテスト 8 個が通る。`monitor_client_live` は 22 項目（UDP 6、TCP 4、WebSocket 5、no_more_monitor 3、WebSocket の 2 項目、と初期化・後始末）が両方で通る。Qt5 では WebSocket のエラーを `error()` でつなぐ経路を通る。
- CMake（Qt6、`-DENABLE_WEBSOCKET=OFF`）で、ビルドとテストが通る。`monitor_transport` は `noWebSocket` を確かめる。`--transport ws` で接続すると「このビルドでは使えない」と表示する。
- Autotools: Qt5、Qt5 で `--disable-websocket`、Qt6 の 3 通りで、ビルドと `make check` が通る。WebSockets ありのビルドだけが `libQt5WebSockets` / `libQt6WebSockets` にリンクし、`--help` の `--transport` の説明も切り替わる。Qt5 で `make distcheck` が通り、配布物に `ws_monitor_transport.*` と `check_websocket.sh` が入っている。
- 実行ファイル（一時的な HOME、オフスクリーン）: WebSocket で接続中にサーバを止めると、止めた直後に切断して自動終了する。確認用のプログラムで `startServer()` を呼ぶと、引数に `server::ws_port=16180` が加わり、1 秒後の接続で接続できた。

**分かったこと**
- Claude Code の VSCode 拡張機能は、Claude を起動するときにワークスペースの Python 環境を有効化する（`claudeCode.usePythonEnvironment`、既定で有効）。ユーザの `python.defaultInterpreterPath` は anaconda の環境なので、Claude の PATH の先頭にその `bin` が入り、Qt5 として anaconda の Qt 5.15.2 が見つかっていた（段階 1 の「分かったこと」の原因）。ユーザの判断で、soccerwindow2 のワークスペースの設定（`.vscode/settings.json`、Git の対象外）で無効にした。これで、指定なしでシステムの Qt 5.15.13 が使われる。
- 段階 3 の途中で、ユーザが librcsc を `~/local` から `~/.local` に入れ直した。CMake のキャッシュの `Librcsc_INCLUDE_DIR` / `Librcsc_LIBRARY` を消して（`cmake -U`）configure し直し、RPATH が `~/.local/lib` になったことを確かめた。
- 確認の手順の誤り（scratchpad の準備が失敗したまま、続きのコマンドが作業ツリーのトップで動いた）で、作業ツリーの in-tree の Autotools の生成物のうち、`configure`、`aclocal.m4`、`config.status`、`config.log`、`autom4te.cache/` が作り直された（2026-11-21 23:53、いずれも Git の対象外）。`Makefile`、`Makefile.in`、`config.h` は 2026-07-11 のまま。この in-tree のビルドは `config/test-driver` がなく、もともと今の `Makefile.am` では `make` できない状態だった。使うときは `./reconfig.sh` で作り直す。以後の Autotools の確認は、各手順を `set -e` のサブシェルにまとめ、scratchpad の外で動かないようにした。

