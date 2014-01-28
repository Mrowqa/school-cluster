#!/bin/bash

if [ "$(id -u)" != "0" ]; then
	echo This script needs root privilages!
	exit 1
fi

echo Installing enviroment...
echo Copying workspace...

HOME=/home/klaster
if ! mount|grep $HOME/openmpi_workspace > /dev/null; then
	echo Workspace not available! Please mount it.
	exit 1
fi
cp -R $HOME/openmpi_workspace $HOME/.ompi_ws

# Adding LD_LIBRARY_PATH entry
echo Adding LD_LIBRARY_PATH entry...
LD_ENTRY=/opt/openmpi-1.6.5/lib
LD_CONF=/etc/ld.so.conf
if [ ! -e $LD_CONF -o "$(cat $LD_CONF|grep $LD_ENTRY)" == "" ]; then
	echo $LD_ENTRY >> $LD_CONF
	echo Done!
else
	echo Already set!
fi

## Adding PATH entry
#echo Adding PATH entry...
#BIN_ENTRY=/opt/openmpi-1.6.5/bin
#BIN_CONF=/etc/profile
#if [ ! -e $BIN_CONF -o "$(cat $BIN_CONF|grep $BIN_ENTRY)" == "" ]; then
#	# EXPORT_PATTERN="export PATH"
#	# REPLACE_WITH=$(echo "PATH+=:$BIN_ENTRY?$EXPORT_PATTERN"|tr ? '\n')
#	# cat $BIN_CONF | sed s/"$EXPORT_PATTERN"/"$REPLACE_WITH"/g > $BIN_CONF
#	echo "export PATH+=:$BIN_ENTRY" >> $BIN_CONF
#	echo Done!
#else
#	echo Already set!
#fi

# Add cron entry to start script every reboot
echo Adding cron entry on reboot...
CRON_JOB='@reboot root '$HOME'/.ompi_ws/startup_script/startup.sh'
if [ ! -e /etc/crontab -o "$(cat /etc/crontab|grep "$CRON_JOB")" == "" ]; then
	echo $CRON_JOB >> /etc/crontab
	echo Done!
else
	echo Already set!
fi

echo Enviroment installed!
