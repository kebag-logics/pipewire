# PipeWire-avb Getting started

## Prerequisites

During installation, it is assumed that a reliable Internet connection is 
available.

### Hardware

* Intel Network Interface:
  * i210 (TESTED)
  * i226 (NOT TESTED)
* x86 platform with as many powerful cores as possible, tested with *SMT* and
 *hyperthreading*
* More than 8GB RAM
* More than 60GB SSD disk space

### Operating system

* Arch Linux version 2025-05-01 (TESTED)
* Linux Ubuntu 24.04 (NOT TESTED)

## Automatic Installation

The current doumentation is describing how to make PipeWire run on ArchLinux.

### Semi-Automatically

 The semi-automation **WILL NOT** take care of:

* Disk configuration
* Keyboard configuration

However, both can easily be done after executing the command below in the
interactive interface.

```bash
 $ bash <( curl -L -s https://bit.ly/42NrpvR )
```

Once booted the system will have the following credidential:

 * Login: pw
 * Password: pipewire

It is recommended to change the password.

## Manual installation

In case the computer already has Arch Linux installed or for 
control and peace of mind, the following packages are necessary 
for installation.

### Make sure that PipeWire is the Audio Server
But make sure to have PipeWire as the Audio Server by running

```pw-top```

It should result in something like this
```
S   ID  QUANT   RATE    WAIT    BUSY   W/Q   B/Q  ERR FORMAT           NAME                           
S   30      0      0    ---     ---   ---   ---     0                  Dummy-Driver
S   31      0      0    ---     ---   ---   ---     0                  Freewheel-Driver
S   35      0      0    ---     ---   ---   ---     0                  auto_null
```

### Create a USB bootable stick

Steps:

 * Download the latest Archlinux version from [here](https://archlinux.org/download/)
 * Make a bootable USB stick with either Balena Etcher (works well on macOS: https://etcher.balena.io) or Rufus (works well on Windows: https://rufus.ie/en/)
 * Configure the BIOS so that it can boot from the USB stick.
 * Boot from USB
 * Use archinstall, and follow the steps, or follow
    [the official Installation Guide](https://wiki.archlinux.org/title/Installation_guide)

#### Configure and install the packages

1. Install the desktop

    ```sudo pacman -S plasma-desktop```

2. Install terminal

    ```sudo pacman -S gnome-terminal```

3. Install the development and PipeWire necessary tools

```bash
    sudo pacman -S base \
          base-devel \
          bash-completion \
          btrfs-progs \
          clang \
          cmake \
          dolphin \
          efibootmgr \
          ethtool \
          git \
          glib2-devel \
          gnome-console \
          gnu-netcat \
          htop \
          less \
          linux-firmware \
          ltrace \
          meson \
          networkmanager \
          numactl \
          openssh \
          openvpn \
          qpwgraph \
          sddm \
          sshfs \
          strace \
          tmux \
          tree \
          unzip \
          vim \
          wireshark-cli \
          zram-generator
```

4. Enable Network Manager

    ```sudo systemctl enable NetworkManager.service```

5. Start desktop

    ```sudo systemctl start sddm```

6. Make it persistent

    ```sudo systemctl enable sddm```

7. Start ssh server

    ```sudo systemctl start sshd```

8. Enable ssh server on boot

    ```sudo systemctl enable sshd```

9. Enable the window manager (sddm is assumed)

    ```sudo systemctl enable ssdm ```

Then reboot: ```sudo reboot```.


# Install LinuxPTP

LinuxPTP is a crucial element of synchronisation in the network. 

Install as follows:

```bash
 $ git clone http://git.code.sf.net/p/linuxptp/code ~/linuxptp
 $ cd ~/linuxptp/
 $ make
 $ sudo make install
```

Then modify the file located at ```~/linuxptp/configs/gPTP.conf```. The parameter to change is
the **priority1** to **247** as diplayed below:

```bash
#
# 802.1AS example configuration containing those attributes which
# differ from the defaults.  See the file, default.cfg, for the
# complete list of available options.
#
[global]
gmCapable               1
priority1               247
priority2               248
logAnnounceInterval     0
logSyncInterval         -3
syncReceiptTimeout      3
neighborPropDelayThresh 800
min_neighbor_prop_delay -20000000
assume_two_step         1
path_trace_enabled      1
follow_up_info          1
transportSpecific       0x1
ptp_dst_mac             01:80:C2:00:00:0E
network_transport       L2
delay_mechanism         P2P
```

Changing the **priority1** to your system proiority is better to make sure the timing
is as close a system current timing, otherwise an error will arise when executing the
ptp_start.sh script later on.


## Install Pipewire

Retrieve the source using git in your home Folder (~).:

```bash
$ git clone --single-branch --branch milan-avb-dev \
   https://github.com/kebag-logics/pipewire.git ~/pipewire
```

## Prepare the system to Run PipeWire

Connect a Milan capable device to the network interface where Milan is going
to receive/send from/to (i210/i226) network card.

Identify the i210/226 interface name with ```ip a```. Typically, the interface name is something like ```enp2s0``` but it can differ on your system.

Once the name is figured out, the following execute the following command and replace ```<interface-name>``` with the name you retrieved.:

```bash
$ export AVB_INTERFACE  <interface-name>
```

Now, execute the following:

```bash
# Follow the PipeWire folder
$ cd ~/pipewire

#configure
$ meson configure builddir -Dprefix=/usr -Davb-interface="$AVB_INTERFACE"

# Compile
$ meson compile -C builddir

# Install
$ cd builddir
$ sudo make install
$ cd ..

# Set capabilities
$ sudo setcap cap_net_raw,cap_net_admin,cap_dac_override+eip /usr/bin/pipewire

# Then prepare the i210 interface
$ sudo ./scripts-milan/prepare-traffic-shaper.sh $AVB_INTERFACE
$ sudo ./scripts-milan/setup-vlan $AVB_INTERFACE

```

Once done, the PTP instances need to be ran in another separate terminal as follows:

```bash
 $ export AVB_INTERFACE <interface-name>
 $ cd ~/pipewire
 $ ./scripts-milan/ptp-start.sh $AVB_INTERFACE
sending: SET GRANDMASTER_SETTINGS_NP
phc2sys[2050.269]: Waiting for ptp4l...
phc2sys[2051.269]: Waiting for ptp4l...
phc2sys[2052.269]: Waiting for ptp4l...
phc2sys[2053.269]: Waiting for ptp4l...
phc2sys[2054.270]: CLOCK_REALTIME phc offset 37030500695 s0 freq      -0 delay   2174
phc2sys[2055.270]: CLOCK_REALTIME phc offset 37030515520 s1 freq  +14823 delay   2154
phc2sys[2056.270]: CLOCK_REALTIME phc offset       -19 s2 freq  +14804 delay   2164
phc2sys[2057.270]: CLOCK_REALTIME phc offset       -32 s2 freq  +14785 delay   2164
phc2sys[2058.270]: CLOCK_REALTIME phc offset       -33 s2 freq  +14775 delay   2164
phc2sys[2059.270]: CLOCK_REALTIME phc offset        51 s2 freq  +14849 delay   2144
phc2sys[2060.270]: CLOCK_REALTIME phc offset        -7 s2 freq  +14806 delay   2144
phc2sys[2061.270]: CLOCK_REALTIME phc offset         0 s2 freq  +14811 delay   2124
```

## Run PipeWire

Once PipeWire is installed, the execution shall be done as below in a terminal:

```bash
 $ cd ~/pipewire

# Restart pipewire
$ systemctl --user restart pipewire.service

# Start with verbose logging
$ /usr/bin/pipewire-avb -v
```

## Configure PipeWire inputs and outputs

1. Install qpwgraph
    
    ```sudo pacman -S qpwgraph```

2. Run qpwgraph by typing ```qpwgraph``` into the console. A window with the available AVB Milan sources and sinks should show up. You can route audio from other applications to pipewire-avb-milan.
