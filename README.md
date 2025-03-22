# OS93
My second attempt OS from scratch.

Written with the help of Cursor, ChatGPT and GitHub Copilot.

![OS93 0.0.3 on QEMU](https://raw.githubusercontent.com/adminekspi/OS93/refs/heads/main/media/OS93_0_0_3.png)

# Building and Running
In Pardus 23 (Debian-based), those packages should be installed:

    build-essential gcc-i686-linux-gnu nasm qemu-system-x86 mtools syslinux

After installing these packages and cloning this repository, you can build and run with that command:

    sudo make run

After testing, you can clean build artifacts and images:

    sudo make clean

## Copyright and License
Copyright (c) 2025 Erdem Ersoy (adminekspi). Licensed with MIT license.

See COPYING for full license text.
