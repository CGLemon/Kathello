# Kathello
基於 AlphaZero 的 othello 引擎

# 需求
C++14
CUDA (可選)
cuDNN (可選)

# 特色
Kathello 支持動態貼目，和預測最終勝負結果。

# 在 Linux ( Ubuntu ) 上編譯

    $ cd Kathello-main
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make


# 其他編譯選項

CPU 線性代數庫 （加速 CPU 端神經網路運算，需要下載對應的加速庫）

    $ cmake .. -DBLAS_BACKEND=EIGEN
    $ cmake .. -DBLAS_BACKEND=OPENBLAS
    

GPU 加速 （加速 GPU 端神經網路運算，cuDNN可選）

    $ cmake .. -DGPU_BACKEND=CUDA
    $ cmake .. -DUSE_CUDNN=1


# 測試的權重
訓練的權重將放在雲端硬碟上，歡迎下載並測試 kathello
https://drive.google.com/drive/folders/1hHGqRVBMOF1KkcA80sFWNUciDjsoPbaf?usp=sharing


# 啟動 Kathello 
./Kathello -w <weights-file> -p <integral / playouts number> -t <integral / threads>

# 簡單的 GTP 的介面
Kathello 實做了一些常用的 gtp 指令
使用 play 和 genmove 的指令就可以在文字介面下和電互相對決

gtp 指令全集： https://www.gnu.org/software/gnugo/gnugo_19.html

# TODO
- [ ] 完整 GTP 的界面
- [ ] 實做 alpha-beta tree ，增強在終盤的強度
