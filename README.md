# playVideoOnCmd
在控制台播放视频的方法。

## 如何运行
从 [Releases](https://github.com/tomokaitoh/playVideoOnCmd/releases) 下载 `videoPlay.7z`,将压缩包里的文件解压到同一个目录下，运行 `videoPlay.exe`.

## 手动编译
注意编译时需要链接Windows的库 `winmm.dll`.
```bash
gcc videoPlay.c -o videoPlay.exe -lwinmm
```
