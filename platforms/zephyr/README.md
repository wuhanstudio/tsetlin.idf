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
$ west build -b esp32_cam/esp32/procpu --sysbuild
```
