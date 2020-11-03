# Kathello

基於 AlphaZero 的 Othello 引擎

# 特色

Kathello 支援動態貼目，和預測最終勝負結果。

# 硬體

1. 一顆普通的　CPU ，多核心會更好
2. 一塊 NVIDIA 的 GPU （沒有可，有會更好）
3. 顯示器、鍵盤

# 需求

1. 支援 C++14 的編譯器

2. cmake，版本高於 3.9

3. CUDA (可選)

4. cuDNN (可選，必須先安裝 CUDA)

5. Eigen (可選 ，請到 third_party 的目錄閱讀下載方法）

# 在 Linux ( Ubuntu ) / MacOS 上編譯

    $ sudo apt install cmake
    $ cd Kathello-main
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make -j


# 其他編譯選項

CPU 線性代數庫 （加速 CPU 端神經網路運算，需要下載對應的加速庫）

    $ cmake .. -DBLAS_BACKEND=EIGEN
    $ cmake .. -DBLAS_BACKEND=OPENBLAS
    

GPU 加速 （加速 GPU 端神經網路運算，cuDNN可選）

在編譯以前，請先確定你有 NVIDIA 的顯卡， 並到 NVIDIA 官網下載 CUDA 。

    $ cmake .. -DGPU_BACKEND=CUDA
    $ cmake .. -DUSE_CUDNN=1


# 測試的權重

訓練的權重將放在雲端硬碟上，歡迎下載並測試 Kathello

https://drive.google.com/drive/folders/1hHGqRVBMOF1KkcA80sFWNUciDjsoPbaf?usp=sharing


# 啟動 Kathello 

    ./Kathello -w <weights-file> -p <integral / playouts number> -t <integral / threads>

# 簡單的 GTP 的介面

Kathello 實做了一些常用的 gtp 指令
使用 play 和 genmove 的指令就可以在文字介面下和電腦相互對決

gtp 指令全集： https://www.gnu.org/software/gnugo/gnugo_19.html

# TODO

- [ ] 完善 GTP 介面
- [ ] 實做 alpha-beta tree ，增強在終盤的強度
- [ ] 自帶簡單的圖形介面
