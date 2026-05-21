# observe_ime: The companion application for hoboNicola adapters and keyboards on Windows.
observe_ime Copyright (C) Takeshi Higasa, okiraku-camera.tokyo.

Releaseビルドしたバイナリ（observe_ime.exe）は、[こちら](https://github.com/okiraku-camera/observe_ime/releases)からダウンロードできます。ビルドは Visual Studio 2022 Community 版で実施しています。

## observe_imeについて
observe_ime は、Windows の IME 状態（入力文字種）を検出し、hoboNicolaアダプターやキーボードに通知するための Win32 アプリケーションです。
これにより、hoboNicolaを使った日本語入力がしやすくなります。機能は以下のとおりです。

* アクティブなウィンドウの IME 状態を取得します。
* IMEが有効、かつ、入力文字種が日本語（ひらがな、カタカナ、ｶﾀｶﾅ）のとき、nicolaモードを有効にするよう通知します。
* それ以外のときは、無効にするように通知します。
* 通知の方法としては、以下の３種類を用意しています。
	* ロックキー(ScrLock または NumLock) を使う方法
	* Mass Storage Classを使う方法
	* USBHID APIを使う方法

* タスクトレイにインジケーターを表示し、右クリックメニューから動作の有効／無効、通知方法の選択、設定および終了を選択できます。
* ロックキーでの通知は、ScrLock/NumLock キーの仮想キーコード出力によって実現するため、実行中のアプリケーション（Excel や TeraTerm など）やテンキーの有無によっては副作用が生じる可能性があります。
 
* アクティブなアプリケーションによっては、IME状態が正しく取得できないことがあります。Web 画面でのパスワード入力時などは、キーボード操作によりIMEを明示的にオフにする必要が生じることがあります。

* USBHID による通知について
	* hoboNicolaデバイスがバージョン 1.8.0 以降である必要があります。
	* 通知対象のデバイスの ``USB情報(VID, PID)`` を設定する必要があります。usbtreeview などのツールやデバイスマネージャーで確認のこと。
	* 同じVID/PIDのデバイスが複数接続されている場合、プログラムが見つけた最初のデバイスにのみ通知します。

## 改版履歴
* version 1.00 (2018/9/12)
  + 初版作成
* version 1.01 (2018/12/8)
  * Windowsのスリープ復帰時に強制的にScrLock をオフにする。スリープ復帰時のサインインでパスワード入力がNICOLAモードとなってしまって正しいパスワードを入力できないため。
  * IME状態の問い合わせは、100msec間隔で行うように変更（従来は50msec）。
* version 1.02 (2021/7/20)
  * IME状態通知にNumLockキーを使えるようにした。
  * IME状態の問い合わせは、200msec間隔で行うように変更。
* version 1.10 (2022/4/1)
  * hoboNicolaでTinyUSBが使える場合、USBデバイス側にMass Storage Classを使ったRAMDISKを作成し、その中のHOBONICO.BINファイルが見えるようにした。
  * このHOBONICO.BINファイルの操作によってIME状態通知を行う機能を追加。
  * HOBONICO.BINを検索する機能も追加。
* version 1.2.0 (2022/10/5)
  * IME状態を監視するインターバル時間をレジストリ設定で上書きできるようにした。
  * \HKEY_CURRENT_USER\SOFTWARE\okiraku-camera\observe_ime\settings 内に DWORD 値 observe_interval を追加し、値を設定する。値は十進数で指定し、100なら100msecとなる。レジストリ設定がない場合は200msec。
  * レジストリ値の読み取りはプログラム開始時にのみ行うので、変更したときはobserve_ime.exeを再起動すること。
  * タスクトレイのインジケーターアイコンの上にマウスカーソルを置いたとき、ツールチップの一部としてリソース内のファイルバージョンを表示するようにした。
* version 1.2.1 (2023/7/28)
  * Windows Explorerが再起動したとき、トレイアイコンを再表示するようにした。
  * RegisterWindowMessage(TEXT("TaskbarCreated")); で得られた値が来たらトレイアイコンを再表示する。
* version 1.8 (2026/5/20)
  * USBHID APIを使ったIME状態通知機能を追加。
  * バージョン 1.8.0以降のhoboNicolaデバイスが必須。
  * ビルド環境を Microsoft Visual Studio Community 2022 (64 ビット) Version 17.14 に変更。
