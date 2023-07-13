# observe_ime , The companion application of hoboNicola adapters and keyboards for Windows environments.
 observe_ime Copyright 2018-2023 Takeshi Higasa, okiraku-camera.tokyo.

 releaseビルドしたバイナリ(observe_ime.exe)は、[ブログのダウンロードページ](https://okiraku-camera.tokyo/blog/?page_id=12997)から、あるいは、~~[こちら](./release) から~~ダウンロードできます。

## observe_imeについて
observe_imeは、WindowsのIME状態の入力文字種を検出し、おもに仮想キーコードを使ったキーストロークによりhoboNicolaに通知するためのWIN32アプリケーションです。
これにより、hoboNicolaアダプターやキーボードを使った日本語の入力がしやすくなります。機能は以下のとおり。

* アクティブなWindowのIMEの状態を取得する。
* タスクトレイにインジケータを表示し、インジケータの右クリックでメニューを開き、動作の有効／無効、終了を選択できる。
* IMEが有効な場合、「ひらがな」、「カタカナ」、「ｶﾀｶﾅ」かどうかを検出する。
* IMEが有効で、ひらがな カタカナ ｶﾀｶﾅ のとき、キーボードのSendInputを使って指定のキーをロック(LEDが点灯)する。
* そうでないときアンロックする。対象のロックキーは、ScrLockまたはNumLockから選択できる。

* ScrLock/NumLock LEDの点灯/消灯は、ScrLock/NumLockキーの仮想キーコードの出力によって実現するので、実行中のアプリケーション(ExcelとかTeraTermとか)によって副作用があるだろう。
* NumLockの場合、テンキーのあるキーボードでのテンキー入力時には大きく影響するが、テンキーレスの場合にはほぼ関係ない。

なお、hoboNicolaアダプターやキーボードでの日本語入力は、IMEの文字種切り替えのためのキーをカスタマイズしてある種の制約を受け入れることで、observe_imeがない環境でも十分に快適に行うことができます。

## 改版履歴
* vserion 1.01
  * Windowsのスリープ復帰時に強制的にScrLock をオフにする。スリープ復帰時のサインインでパスワード入力がNICOLAモードとなってしまって正しいパスワードを入力できないため。
  * IME状態の問い合わせは、100msec間隔で行うように変更（従来は50msec）。
* version 1.02 (2021/7/20)
  * IME状態通知にNumLockキーを使えるようにした。
  * IME状態の問い合わせは、200msec間隔で行うように変更
* version 1.10 (2022/4/1)
  * hoboNicolaでTinyUSBが使える場合、Mass Storage Classを使ったRAMDISKを作成し、その中にHOBONICO.BINファイルを作るようにした。
  * このHOBONICO.BINファイルの操作によってIME状態通知を行う機能を追加。
  * HOBONICO.BINを検索する機能も追加。
* version 1.2.0 (2022/10/5)
  * IME状態を監視するインターバル時間をレジストリ設定で上書きできるようにした。
  * \HKEY_CURRENT_USER\SOFTWARE\okiraku-camera\observe_ime\settings 内に、DWORD値 observe_interval を追加し値を設定する。値は、十進数の100を指定すれば100msecとなる。レジストリ設定がない場合は200msec。
  * レジストリ値の読み取りはプログラム開始時にのみ行うので、変更したときはobserve_ime.exeを再起動すること。
  * タスクトレイのインジケーターアイコンの上にマウスカーソルを置いたとき、ツールチップの一部としてリソース内のファイルバージョンを表示するようにした。