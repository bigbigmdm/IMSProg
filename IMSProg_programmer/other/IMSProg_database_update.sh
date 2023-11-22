#!/bin/bash
PASSWORD=$(zenity --password)
cd /tmp
wget https://antenna-dvb-t2.ru/dl_all/IMSProg.Dat
cd /etc
mkdir -p imsprog
file=/etc/imsprog/IMSProg.Dat
oldsize=$(wc -c <"$file")
let oldrec=oldsize/68-1
echo $PASSWORD | sudo -S cp /tmp/IMSProg.Dat /etc/imsprog/
rm -rf /tmp/IMSProg.Dat
newsize=$(wc -c <"$file")
let newrec=newsize/68-1
out_text=$(echo $oldrec "->" $newrec)
echo $oldrec "->" $newrec
zenity --warning \
--text="The number of chips in the database:\n\n$out_text" \
--title="Database update" \
--icon-name='applications-electronics'
