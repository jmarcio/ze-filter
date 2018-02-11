#! /bin/bash

Services="ze-filter ze-greyd"
echo $0

BaseDir=$(dirname $0)

SystemCtl=$(which systemctl 2>/dev/null)

echo $SystemCtl

if [ -n "$SystemCtl" ]
then
  echo "* Has systemctl at $SystemCtl"
  DirTarget=
  for base in /etc /usr/lib /lib
  do
    [ -d $base/systemd/system ] && DirTarget=$base/systemd/system
  done
  echo "  Installing at $DirTarget"
  for svc in $Services
  do
    echo "    Service : $svc"
    echo "              $BaseDir/$svc.service"
    if [ -f $BaseDir/$svc.service ]
    then
      echo "    Installing $svc.service"
      cp -p $BaseDir/$svc.service $DirTarget
      chown root:root $DirTarget/$svc.service
    fi
  done

  $SystemCtl daemon-reload
  for svc in $Services
  do
    echo "    Restarting : $svc"
    $SystemCtl is-enabled $svc > /dev/null && $SystemCtl restart $svc
  done
fi


