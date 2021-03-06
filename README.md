# mnsh

リダイレクト、パイプ、外部コマンド、一部の内部コマンド、バックグラウンド実行を操作できる小規模なbashを参考にして作成したシェル。

# Requirement
- Ubuntu 18.04.4 LTS

# 機能

## リダイレクト
|表記       |動作                                                       |
|:----------|:----------------------------------------------------------|
|\< file    |標準入力をfileに変更する                                   |
|\> file    |fileの中身を空にし、標準出力をfileの先頭に変更する         |
|\>\> file  |標準出力をfileの末尾に変更する                             |
|2\> file   |fileの中身を空にし、標準エラー出力をfileの先頭に変更する   |
|2\>\> file |標準エラー出力をfileの末尾に変更する                       |

- またリダイレクトで変更する、標準入力、標準出力、標準エラー出力は、コマンドラインにつきそれぞれ1回のみ変更が可能となる。

## パイプ
|表記|動作|
|:---|:---|
|command1 \| command2|command1の標準出力をcommand2の標準入力に繋ぐ|

*パイプを複数つなぎ合わせた、多段パイプの使用も可能である。

## 外部コマンド

- 外部コマンドの実行が可能である。

## 内部コマンド
- exitコマンド
    - シェルを終了させる。
    - ^Cではシェルは終了しない。
- cdコマンド
    - 引数を1つ指定するとそのパスへカレントディレクトリが移動する。
    - 指定しなければホームディレクトリに移動する。
- bgコマンド
    - バックグラウンドで一時中断したジョブをバックグラウンドで実行する。
    - 引数にジョブ番号を指定する。しない場合は、動作しない。(ここがbashと違う点)
- fgコマンド
    - バックグラウンドにあるジョブをフォアグラウンドで実行させる
    - 引数にジョブ番号を指定する。しない場合は、動作しない。(ここがbashと違う点)
- jobsコマンド
    - ジョブの稼働状況を表示する。
    - 引数については実装していない。

## バックグラウンド実行
|表記|動作|
|:---|:---|
|command1 &|command1をバックグラウンドで実行|

## シグナルの処理
- ^Cでシェルは終了せず、実行中のコマンドは終了させることが出来る
- ^Zでシェルは一時停止せず、実行中のコマンドは一時停止する