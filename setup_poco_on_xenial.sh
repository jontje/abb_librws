#!/bin/bash

if [ $(lsb_release -sc) = "xenial" ]
then
  if [ ! -e /etc/apt/sources.list.d/poco-bionic.list ]
  then
    echo "deb http://archive.ubuntu.com/ubuntu/ bionic main restricted universe"           >> /etc/apt/sources.list.d/poco-bionic.list
    echo "deb http://security.ubuntu.com/ubuntu/ bionic-security main restricted universe" >> /etc/apt/sources.list.d/poco-bionic.list
  fi

  if [ ! -e /etc/apt/preferences.d/poco-dev.pref ]
  then
    echo "Package: *"            >> /etc/apt/preferences.d/poco-dev.pref
    echo "Pin: release n=xenial" >> /etc/apt/preferences.d/poco-dev.pref
    echo "Pin-Priority: -10"     >> /etc/apt/preferences.d/poco-dev.pref
    echo ""                      >> /etc/apt/preferences.d/poco-dev.pref
    echo "Package: libpoco-dev"  >> /etc/apt/preferences.d/poco-dev.pref
    echo "Pin: release n=bionic" >> /etc/apt/preferences.d/poco-dev.pref
    echo "Pin-Priority: 500"     >> /etc/apt/preferences.d/poco-dev.pref
  fi

  apt-get update
  apt-get install -y --no-install-recommends libpoco-dev
fi
