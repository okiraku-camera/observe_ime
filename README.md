# observe_ime: The companion application for hoboNicola adapters and keyboards on Windows.
observe_ime Copyright 2018-2023 Takeshi Higasa, okiraku-camera.tokyo.

Releaseビルドしたバイナリ（observe_ime.exe）は、[ブログのダウンロードページ](https://okiraku-camera.tokyo/blog/?page_id=12997)または[こちら](https://github.com/okiraku-camera/observe_ime/releases)からダウンロードできます。ビルドは Visual Studio 2017 Community 版で実施しています。

## observe_imeについて
observe_ime は、Windows の IME 状態（入力文字種）を検出し、主に仮想キーコードを使ったキーストロークによって hoboNicolaアダプターやキーボードに通知するための Win32 アプリケーションです。
これにより、hoboNicolaを使った日本語入力がしやすくなります。機能は以下のとおりです。

* アクティブなウィンドウの IME 状態を取得します。
* IME が有効で、入力文字種が「ひらがな」「カタカナ」「ｶﾀｶﾅ」のとき、キーボードの SendInput を使って指定キーをロック（LED 点灯）します。
* それ以外のときは指定キーをアンロックします。通知に使うロックキーとして、ScrLock または NumLock を選択できます。
* タスクトレイにインジケーターを表示し、右クリックメニューから動作の有効／無効や終了を選択できます。
* ScrLock/NumLock LED の点灯／消灯は、ScrLock/NumLock キーの仮想キーコード出力によって実現するため、実行中のアプリケーション（Excel や TeraTerm など）によっては副作用が生じる可能性があります。
* NumLock を使う場合、テンキー付きキーボードではテンキー入力に大きく影響しますが、テンキーレスではほぼ影響しません。
* アクティブなアプリケーションによっては、IME状態が正しく取得できないことがあります。Web 画面でのパスワード入力時などは、キーボード操作によりIMEを明示的にオフにする必要が生じることがあります。

なお、hoboNicola アダプターやキーボードでの日本語入力は、IME の文字種切り替えキーをカスタマイズし、一定の制約を受け入れることで、observe_ime がない環境でも十分に快適に行えます。

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