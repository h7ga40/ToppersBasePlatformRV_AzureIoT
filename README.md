# TOPPERS BASE PLATFORM でAzure IoTに接続するサンプルプログラム

[TOPPERS BASE PLATFORM](https://www.toppers.jp/edu-baseplatform.html)を使い Azure IoTに接続するサンプルアプリ。

[Sipeed Maix Bit](https://dl.sipeed.com/MAIX/HDK/Sipeed-Maix-Bit)向けで、別途用意した[ESP-WROOM-32](https://www.espressif.com/)のATコマンドのTCPを利用しWifiで通信します。ESP32とはIO6とIO7のUARTで通信します。

## 開発環境

ビルドはGCCとGNU makeを使用します。
ここではWindowsでの開発環境について説明します。
GCCは[ここ](https://github.com/xpack-dev-tools/riscv-none-embed-gcc-xpack/releases/)で、
GNU makeは[MSYS2](https://www.msys2.org/)のものを使います。
IDEとして、[Visual Studio Code](https://code.visualstudio.com/)を使います。

MSYS2とVisual Studio Codeの環境構築は、下記を参照してください。

<https://qiita.com/takasehideki/items/fa0a1a6567a22f469515>

ARM GCCの環境構築ですので、RISC-V版の場合はRISC-V GCCに置き換えて読んでください。

Visual Studio Codeの設定は、GCCが「C:\xpack-riscv-none-embed-gcc-8.3.0-1.2」に展開してある設定になっています。

Sipeed Maix BitのCPU Kendryte K210向けのGCCも使用できますが、下記の違いを直す必要があります。
|-|xpack|Kendryte|備考|
|-|-|-|-|
|インストールしたフォルダ|C:\xpack-riscv-none-embed-gcc-8.3.0-1.2|C:\kendryte-toolchain|各自の環境に合わせる|
|コマンドプリフィックス|riscv-none-embed|riscv64-unknown-elf|-|
|marchオプション|-march=rv64imafdc|-march=rv64imafc|-|
|mabiオプション|-mabi=lp64d|-mabi=lp64f|-|

TOPPERS BASE PLATFORMのリリースに合わせてxpackを使用しています。

## ビルド

ルートフォルダにある「azure_iothub.code-workspace」を、Visual Studio Codeで開きます。
初回の起動では下記の手順が必要です。
「Ctrl+@」でターミナルを開きます。この時警告のポップアップが出るので、「許可」してください。
Visual Studio Codeを一度終了して、もう一度起動します。
「Ctrl+Shift+B」でビルドが始まります。
「app_iothub_client\Debug\app_iothub_client.bin」が出力ファイルで、これを書き込みます。

## 書き込み

[K-Flash](https://github.com/kendryte/kendryte-flash-windows/releases)を使って書き込みます。
ボーレートは115200が安定して書き込めます。

## Lisence

|フォルダ|名称|ライセンス|
|-|-|-|
|app_iothub_client/src|アプリ|ファイルヘッダー参照|
|app_iothub_client/kendryte|[Kendryte K210 standalone SDK](https://github.com/kendryte/kendryte-standalone-sdk)|Apache-2.0 License|
|asp_baseplatform|[TOPPERS BASE PLATFORM](https://www.toppers.jp/edu-baseplatform.html)|[TOPPERS License](https://www.toppers.jp/license.html)|
|azure_iot_sdk|[Azure IoT C SDKs and Libraries](https://github.com/Azure/azure-iot-sdk-c)|MIT License|
|wolfssl-4.4.0|[wolfSSL](https://www.wolfssl.jp/)|GPL License</br>or</br>[商用ライセンス](https://www.wolfssl.jp/license/)|
|zlib-1.2.11|[zlib](http://zlib.net/)|[zlib license](http://zlib.net/zlib_license.html)|
