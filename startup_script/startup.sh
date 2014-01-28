#!/bin/bash

is_mounted() { # args: $directory
	if [ -z "$1" ]; then
		echo Missing is_mouted\(\) argumentes!
		return 2
	fi
	mount|grep $1 > /dev/null
	return $?
}

mount_remote_dir() { # args: $server, $remote_dir $local_dir
	if [ -z "$1" -o -z "$2" -o -z "$3" ]; then
		echo Missing mount_remote_dir\(\) arguments!
		return 1
	fi
	
	server=$1
	remote_dir=$2
	local_dir=$3
	
	echo Mouting $server:$remote_dir at $local_dir ...
	
	if [ ! -d "$local_dir" ]; then
		mkdir $local_dir
	fi
	
	if is_mounted $local_dir; then
		echo Directory has been already mounted. It will be remounted.
		echo Unmouting...
		umount $local_dir
		if is_mounted $local_dir; then
			echo Cannot unmount!
			return 2
		fi
		echo Unmouting done!
	fi

	mount $server:$remote_dir $local_dir
	if ! is_mounted $local_dir; then
		echo Cannot mount remote dir!
		return 3
	fi
	
	echo Mouting $server:$remote_dir at $local_dir done!
	return 0
}

run() {
	if [ "$(id -u)" != "0" ]; then
		echo This script needs root privilages!
		return 1
	fi
	
	found=false
	server=lab04-02
	
	while [ "$found" == "false" ]; do
		echo Waiting for a server...
		ping -c 1 -t 1 $server > /dev/null
		if [ $? -eq 0 ]; then # check whether sever is available
			found=true
			echo Server found!
			break
		fi
		sleep 60s
	done
	
	ompi_dir=/opt/openmpi-1.6.5
	ompi_workspace=/home/klaster/openmpi_workspace
	
	echo Mounting remote directories...
	if \
	! mount_remote_dir $server /opt/openmpi-1.6.5 $ompi_dir || \
	! mount_remote_dir $server /home/klaster/openmpi_workspace $ompi_workspace
	then
		echo Mounting error. Script will exit now.
		return 2
	fi
	
	#. ./prepare_ompi_env.sh $ompi_dir
	echo Enviroment ready!
}

run
