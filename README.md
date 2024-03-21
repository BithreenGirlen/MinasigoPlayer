# MinasigoPlayer
某寝室再生機。

## 動作要件
- Windows 8 以降のWindows OS
- MSVC 2015-2022 (x64)
- 游明朝

## 再生方法
フォルダ選択ダイアログから次のようなフォルダを開くと再生を開始します。
<pre>
120020811
  ├ 101.jpg
  ├ 102.jpg
  ├ ...
  ├ 120020811.txt
  ├ adu200208_01_01.mp3
  ├ ...
  └ adu200208_01_52.mp3
</pre>

## マウス機能

| 入力  | 機能  |
| --- | --- |
| マウスホイール | 拡大・縮小。 |
| 左ボタン + マウスホイール | 画像送り・戻し。 |
| 右ボタン + マウスホイール | 文章送り・戻し。 |
| 左ボタンドラッグ | 表示位置移動。モニタ解像度以上に拡大した場合のみ動作。 |
| 中ボタンクリック | 尺度・表示位置初期化。 |
| 右ボタン + 中ボタンクリック | 窓枠消去・表示。消去時にはモニタ原点位置に移動。 |
| 右ボタン + 左ボタンクリック | 窓移動。 窓枠消去時のみ動作。|


## キー機能

| 入力  | 機能  |
| --- | --- |
| Esc | 終了。 |
| T   | 文章表示・非表示切り替え。 |
| Up | 前のフォルダに移動。 |
| Down | 次のフォルダに移動。 |

例えば次の階層構造にて`120020811`を再生していた場合、キー入力で`120020712`や`120020812`に移動します。
<pre>
Resource
├ ...
├ 120020712
│   ├ 201.jpg
│   └ ...
├ 120020811
│   ├ 101.jpg
│   └ ...
├ 120020812
│   ├ 201.jpg
│   └ ...
└ ...
</pre>

## メニュー機能
| 分類 | 項目 | 機能 |
----|---- |---- 
Folder| Open| フォルダ選択ダイアログ表示。
Audio| Loop| 音声ループ有効・無効切り替え。
 -| Setting| 音量・再生速度設定画面表示。

## 補足説明
- 文章表示
  - 通し番号を振ってあります。
  - 音声なしの場合は2秒後に、音声有りの場合は再生終了を待って次の文章に移行します。
