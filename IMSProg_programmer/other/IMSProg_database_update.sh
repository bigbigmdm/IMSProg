#!/bin/bash
PASSWORD=$(zenity --password)
cd /tmp
wget https://antenna-dvb-t2.ru/dl_all/IMSProg.Dat
cd /etc
mkdir -p imsprog
echo $PASSWORD | sudo -S cp /tmp/IMSProg.Dat /etc/imsprog/
rm -rf /tmp/IMSProg.Dat
