#!/bin/sh

# Installing some neccesary packages
sudo apt-get install dchroot
sudo apt-get install debootstrap

sudo mkdir /home/ujail

# Adding new user
useradd -g bbn -c "jailed user" -d /home/ujail -s /bin/bash ujail
passwd ujail

# Adding chroot configuration
sudo cat chroot.conf >> /etc/schroot/schroot.conf

# Installing small installation of ubuntu in jailed dir
sudo debootstrap --variant=buildd --arch i386 lucid /home/ujail http://ftp.acc.umu.se/ubuntu/

# Change to root shell to chroot
sudo chroot /home/ujail

