#!/bin/bash
PASSWORD=$(zenity --password)
cd /tmp
wget https://antenna-dvb-t2.ru/dl_all/IMSProg.Dat
echo $PASSWORD | sudo -S cp IMSProg.Dat /opt/IMSProg
rm -rf /tmp/IMSProg.Dat
