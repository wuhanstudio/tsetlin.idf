## Quick Start

```
$ pip install west
$ west init ~/zephyrproject

$ cd ~/zephyrproject
$ west update

$ west zephyr-export
$ west packages pip --install
```

Step 2: Install Zephyr-SDK

```
$ cd ~/zephyrproject/zephyr
$ west sdk install
```

Step 3: Build

```
$ west build -b pandora_stm32l475
$ west build -b black_f407zg_pro

$ west build -b mini_stm32h743

$ dfu-util -a 0 -s 0x08000000:leave -D build/zephyr/zephyr.bin   
```
