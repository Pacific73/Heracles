# Heracles

This repo reconstructs a simple Google Heracles system prototype. It implements basic features mentioned in the paper (isca 15). But it cannot be deployed on clusters in data centers directly for its lack of distributed communication mechanism and other advanced features.

## Structure of this system

The system consists of 4 parts.

1. **TopController**: pulls data from LC service, analyzes data and turns on/off the system.
2. **CoreMemoryController**: dynamically adjusts the resource allocation configuration including physical cores/last level cache/memory bandwidth.
3. **NetworkController**: dynamically adjusts the bandwidth allocation for LC and BE jobs.
4. **Tap**: stores shared data and variables, and is a simple BE task scheduler.

You can find other details in my another repo: [GraduationProject/reports](https://github.com/Pacific73/GraduationProject/tree/master/reports).

## Environment support

1. Last level cache allocation and memory bandwidth monitoring needs support of Intel RDT Technology, which requires Intel-cmt-cat library. This library has been integrated in this repo (location: /lib). You can also update it from its [git repo](https://github.com/intel/intel-cmt-cat). Just copy related directory in Intel-cmt-cat over the lib directory.

2. Before execute the program, you may need execute this command.

   ```shell
   sudo modprobe msr
   ```

3. Intel RDT Technology only supports some of the Intel Xeon CPUs. Please confirm your CPU type.

4. You need to execute this program in root previledge.

5. You need cgroups, bcc (compiler for eBPF) and other components. So you'd better execute following commands to install necessary tools.

   ```shell
   sudo apt-get install cgroup-bin cgroup-lite cgroup-tools cgroupfs-mount libcgroup1
   # install cgroups
   
   sudo apt-get -y install bison build-essential cmake flex git libedit-dev libllvm3.7 llvm-3.7-dev libclang-3.7-dev zlib1g-dev libelf-dev
   # install llvm
   
   git clone https://github.com/iovisor/bcc.git
   mkdir bcc/build; cd bcc/build
   cmake .. -DCMAKE_INSTALL_PREFIX=/usr
   make
   sudo make install
   # install bcc
   
   sudo apt-get install sqlite3
   # install sqlite3
   ```

## Some notes

The basic idea about this system (program) is that

1. You need to have a LC service program first, and it can continually output its latency and load percent statistics to files. 

   What kind of latency and load? Such as 95% latency, 99% latency... Current load percent is 68%... other things like that. How to calculate latency and load percent? That's your LC service's job. 

   Also, we need to manually set some maximum value, like max acceptable latency and max acceptable QPS (query per second) so that we can calculate "percent".

2. We have a simple sqlite3 database named as `tasks.db`. You can add your BE tasks into this database. When Heracles is running, it will execute these BE tasks according to their task state. More details in `bin/dbop.sh`.

3. This program will dynamically adjust resource allocation for LC and BE tasks. Overall implementation and methods is similar to Heracles