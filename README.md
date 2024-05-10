# Better mount
> **Better Mount** is a terminal-based program written in C that helps users mount a device in their system more easily.

![print_066](https://github.com/Marce1in/bmount/assets/98642728/6ce2c554-7f70-43aa-903d-e055b1b44f63)

# Why bmount?
Sometimes you don't want to mount all your devices on boot; you just want to mount what you are actually using.

Yes, you could use the `mount` command, but this usually involves passing a bunch of flags so you can edit or change files inside the device without root access, etc. Additionally, you have to find the right device, usually by using another command like `fdisk -l`. Doing this every time is a hassle, and that's why I developed this program.

# Installing
- Make sure you have `fdisk` and `make` installed.
- Clone this repo using `git clone https://github.com/Marce1in/bmount.git`.
- Navigate to the bmount directory: `cd bmount`.
- Run `sudo make clean install`.
- Now `sudo bmount` should work :)

# Usage
Run `sudo bmount`, select your device, then navigate to `/media/` using `cd /media/`, and your files should be there.

# How it works under the hood?
First, we run `fdisk -l` to get the whole output. We parse the most important information (Device, Size, and Type) and present it to the user in a nicely formatted way.

After the user selects the device, we run `mount -o nosuid,nodev,rw,noatime,exec,umask=0 *(the device path)* /media/`, and that's it.

# Metas
The base program works perfectly, but I still desire to implement:
- Allow users to choose their own mount path, like: `bmount ~/file-system`.
- Allow users to change the base internal mount flags, like: `bmount -f [-o ro,auto,nouser]`.
- No sudo mode; instead of running `mount` internally, we generate the command and return it to the user.
