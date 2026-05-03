Name: imsprog
Version: 1.8.3
Release: 1%dist

Summary: I2C, SPI and MicroWire EEPROM/Flash chip programmer for CH341a/CH347t devices
Summary(ru_RU.UTF-8): I2C, SPI and MicroWire EEPROM/Flash программатор для CH341a/CH347t устройств
Summary(de_DE.UTF-8): I2C, SPI und MicroWire EEPROM/Flash Chip-Programmierer für CH341a/CH347t Geräte
Summary(es_ES.UTF-8): Programador de chips EEPROM/Flash I2C, SPI y MicroWire para dispositivos CH341a/CH347t
Summary(hu_HU.UTF-8): I2C, SPI és MicroWire EEPROM/Flash chipprogramozó CH341a/CH347t eszközökhöz
Summary(it_IT.UTF-8): Programmatore di chip EEPROM/Flash I2C, SPI e MicroWire per dispositivi CH341a/CH347t
Summary(pt_BR.UTF-8): Programador de chip EEPROM/Flash I2C, SPI e MicroWire para dispositivos CH341a/CH347t
Summary(uk_UA.UTF-8): I2C, SPI і MicroWire EEPROM/програматор мікросхем для пристроїв CH341a/CH347t
Summary(zh_CN.UTF-8): 用于 CH341a/CH347t 设备的 I2C、SPI 和 MicroWire EEPROM/闪存芯片编程器
SourceLicense: GPL-2.0-or-later AND GPL-3.0-or-later AND LGPL-2.1-only and BSD-1-Clause
License: GPL-3.0-or-later AND LGPL-2.1-only
ExcludeArch: s390x

Group: Applications/Engineering

Url: https://github.com/bigbigmdm/IMSProg
Source: https://github.com/bigbigmdm/IMSProg/archive/refs/tags/v%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: cmake(Qt5LinguistTools)
BuildRequires: pkgconfig(libusb-1.0)
BuildRequires: cmake
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

%description
IMSProg is a free software for CH341A/CH347T based chip programmers with 
graphical interface. With IMSProg you can read, verify, program, erase i2C, SPI, 
SPI NOR, MicroWire, Dataflash EEPROM chips using the CH341A/CH347T programmer 
device.

IMSProg is a collection of tools:
IMSProg - the chip programmer (it's the main part).
IMSProg_editor - chip database editor.
IMSProg_database_update - script to update chip database using external
web-server.

%description -l ru_RU.UTF-8
IMSProg - это бесплатное программное обеспечение для программаторов микросхем
на базе CH341A/CH347T с графическим интерфейсом. С помощью IMSProg вы можете 
читать, проверять, программировать, стирать микросхемы i2C, SPI, SPI NOR, 
MicroWire EEPROM, DataFlash используя устройство-программатор CH341A/CH347T.

IMSProg - это набор инструментов:
IMSProg - программатор микросхем (это основная часть).
IMSProg_editor - редактор базы данных микросхем.
IMSProg_database_update - скрипт для обновления базы данных микросхем с помощью
внешнего веб-сервера.

%description -l de_DE.UTF-8
IMSProg ist eine kostenlose Software für CH341A/CH347T-basierte 
Chip-Programmiergeräte mit grafischer Oberfläche. Mit IMSProg können Sie i2C, 
SPI, SPI NOR, MicroWire, DataFlash EEPROM Chips mit dem CH341A/CH347T 
Programmiergerät lesen, verifizieren, programmieren, löschen.

IMSProg ist eine Sammlung von Werkzeugen:
IMSProg - der Chip-Programmierer (das ist der Hauptteil).
IMSProg_editor - Chipdatenbank-Editor.
IMSProg_database_update - Skript zur Aktualisierung der Chipdatenbank mit Hilfe
eines externen Web-Servers.

%description -l es_ES.UTF-8
IMSProg es un software gratuito para programadores de chips basados en 
CH341A/CH347T con interfaz gráfica. Con IMSProg puede leer, verificar, programar, 
borrar chips i2C, SPI, SPI NOR, MicroWire, DataFlash EEPROM utilizando el 
dispositivo programador CH341A/CH347T.

IMSProg es una colección de herramientas:
IMSProg - el programador de chips (es la parte principal).
IMSProg_editor - editor de bases de datos de chips.
IMSProg_database_update - script para actualizar la base de datos de chips
utilizando un servidor web externo.

%description -l hu_HU.UTF-8
Az IMSProg egy ingyenes szoftver CH341A/CH347T alapú chipprogramozók számára 
grafikus felülettel. Az IMSProg segítségével a CH341A/CH347T programozó 
eszközzel i2C, SPI, SPI NOR, MicroWire, DataFlash EEPROM chipeket olvashat, 
ellenőrizhet, programozhat, törölhet.

Az IMSProg egy eszközgyűjtemény:
IMSProg - a chip programozó (ez a fő rész).
IMSProg_editor - chip adatbázis szerkesztő.
IMSProg_database_update - szkript a chip adatbázis frissítéséhez külső
webszerver használatával.

%description -l it_IT.UTF-8
IMSProg è un software gratuito per programmatori di chip basati su CH341A/CH347T
con interfaccia grafica. Con IMSProg è possibile leggere, verificare, 
programmare, cancellare chip i2C, SPI, SPI NOR, MicroWire, DataFlash EEPROM, 
utilizzando il programmatore CH341A/CH347T.

IMSProg è una raccolta di strumenti:
IMSProg - il programmatore di chip (è la parte principale).
IMSProg_editor - editor del database dei chip.
IMSProg_database_update - script per aggiornare il database dei chip utilizzando
un server web esterno.

%description -l pt_BR.UTF-8
O IMSProg é um software gratuito para programadores de chips baseados no 
CH341A/CH347T com interface gráfica. Com o IMSProg, você pode ler, verificar, 
programar e apagar chips i2C, SPI, SPI NOR e MicroWire, DataFlash EEPROM usando 
o dispositivo programador CH341A/CH347T.

O IMSProg é uma coleção de ferramentas:
IMSProg - o programador de chips (é a parte principal).
IMSProg_editor - editor de banco de dados de chips.
IMSProg_database_update - script para atualizar o banco de dados de chips usando
um servidor da Web externo.

%description -l uk_UA.UTF-8
IMSProg - вільне програмне забезпечення для програматорів мікросхем на базі
CH341A/CH347T з графічним інтерфейсом. За допомогою IMSProg ви можете читати,
перевіряти, програмувати, стирати мікросхеми i2C, SPI, SPI NOR, MicroWire,
DataFlash EEPROM за допомогою програматора CH341A/CH347T.

IMSProg - це набір інструментів:
IMSProg - програматор мікросхеми (це основна частина).
IMSProg_editor - редактор бази даних мікросхем.
IMSProg_database_update - скрипт для оновлення бази даних чіпів за допомогою
зовнішнього веб-сервера.

%description -l zh_CN.UTF-8
MSProg 是一套 I2C、SPI、MicroWire EEPROM/Flash 芯片 CH341A/CH347T 编程器免费软件。
借助 IMSProg，您可以使用 CH341A/CH347T 编程器读取、编程、校验、擦除 I2C、DataFlash、
SPI、SPI NOR 和 MicroWire 芯片。

IMSProg 是一套工具集合
IMSProg - 芯片编程器（主要部分）
IMSProg_editor - 芯片数据库编辑器。
IMSProg_database_update - 使用外部Web服务器更新芯片数据库的脚本。

%prep
%autosetup -p1 -n IMSProg-%{version}

%build
# update translations
lrelease-qt5 IMSProg_editor/language/*.ts
lrelease-qt5 IMSProg_programmer/language/*.ts

pushd IMSProg_editor
%cmake -DCMAKE_INSTALL_SYSCONFDIR=%_sysconfdir
%cmake_build
popd

pushd IMSProg_programmer
%cmake -DCMAKE_INSTALL_SYSCONFDIR=%_sysconfdir
%cmake_build
popd

pushd IMSProg_database_update
%cmake -DCMAKE_INSTALL_SYSCONFDIR=%_sysconfdir
%cmake_build
popd

%install
pushd IMSProg_editor
%cmake_install
popd

pushd IMSProg_programmer
%cmake_install
popd

pushd IMSProg_database_update
%cmake_install
popd

# rename README
cp IMSProg_editor/README.md IMSProg_editor.md
cp IMSProg_programmer/README.md IMSProg_programmer.md

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/*.desktop
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/*.xml

%files
%doc README.md IMSProg_editor.md IMSProg_programmer.md ChangeLog
%_docdir/imsprog/
%_bindir/IMSProg
%_bindir/IMSProg_editor
%_bindir/IMSProg_database_update
%_datadir/imsprog
%_datadir/applications/IMSProg.desktop
%_datadir/applications/IMSProg_editor.desktop
%_datadir/applications/IMSProg_database_update.desktop
%_datadir/metainfo/*.xml
/usr/lib/udev/rules.d/*.rules
%_datadir/pixmaps/chipEdit64.png
%_datadir/pixmaps/IMSProg64.png
%_datadir/pixmaps/IMSProg_database_update.png
%_datadir/man/man1/*.1.*
%license LICENSE

%changelog
* Tue Apr 21 2026 Mikhail Medvedev 1.8.3-1
- Fixed: If no chip is selected, the programmer was not detected
- Fixed: MACOS library compatibility
- Fixed: MACOS invalid pathes
- Fixed: libusb detection
- Fixed: udev rules install
- Added: 2.5v VCC item
- SPI NOR FLASH writing speed increased
- Changed chip database update bash script to Qt app
- The Zenity package is no longer used

* Wed Mar 18 2026 Mikhail Medvedev 1.8.2-1
- Fix: error reading/writing data from I2C chips on the CH347T version 5.44
- Fix: bit error during NOR/NAND operations in the CH347V1.1 programmer
- Added: showing the programmer revision function
- Added: erase check function
- Added: filling code function

* Fri Mar 13 2026 Mikhail Medvedev 1.8.1-1
- Fix: increased interface speed
- Fix: increased SPI NOR Flash operations speed
- Fix: incorrect chip page sizes values removed
- Fix: unused files removed
- Added: support for the CH347T programmer

* Fri Feb 06 2026 Mikhail Medvedev 1.7.3-1
- Fix: Incorrect HexEdit size when starting the program
- Added: saving program settings to an ini file

* Tue Nov 25 2025 Mikhail Medvedev 1.7.2-1
- Fix: NAND Flash information form freeze
- Added new images for CH341A v1.7    
- Added programmer type menu
- Added support for the AT24CM02 chip

* Wed Nov 06 2025 Mikhail Medvedev 1.7.1-1
- Fix: pressing the Stop key did not stop auto operations
- Added support for SPI NAND flash
- Added traditional chinese translation
- New chips added to database

* Thu Jul 24 2025 Fedora Release Engineering <releng@fedoraproject.org> - 1.6.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_43_Mass_Rebuild

* Tue Jul 01 2025 Mikhail Medvedev 1.6.2-1
- Fix: program crashed if security registers was less than 256 bytes
- Fix: Information in the status bar disappears when hovering over a menu
- Added comparison of INTEL HEX and ASUS CAP with other files
- Added close and minimise buttons for subordinate forms
- New chips added to database
- Chip information form code optimized

* Mon May 19 2025 Mikhail Medvedev 1.6.1-1
- Add new feature: compare files
- Add new feature: fill test array
- Fix: when the page size is changed, the buffer is cleared
- Fix: the code is executed twice and the code slows down the display of the interface

* Tue Apr 29 2025 Mikhail Medvedev 1.5.3-1
- Last version of QHexEdit used
- Hex-functions optimised
- Added support GIANTEC, DOSILICON NOR FLASH chips
- Fix: incorrect uninstall script
- Fix: i2C 128K size is repeated twice
- Fix: error calculating file size

* Wed Mar 05 2025 Mikhail Medvedev 1.5.2-1
- Fix: Memory leaks
- Fix: Gigadevice status registers write error
- Added new SPI chip sizes
- Added Fidelix, Zetta, MXIC OTP algorithm
- Added support for new Boya, Winbond chips

* Wed Feb 19 2025 Mikhail Medvedev 1.5.1-1
- Fix: Incorrect install dir for udev while cross compiling
- Added form for working with security registers for SPI NOR flash memory chips

* Mon Jan 20 2025 Mikhail Medvedev 1.4.5-1
- Fix: Incorrect block/page size after first read procedure
- Fix: Incorrect reading/writing I2C low speed chips
- Added support for MXIC MX25V8035F
- Added I2C bus speed combobox to the main form

* Tue Sep 24 2024 Mikhail Medvedev 1.4.4-1
- Fix: incorrect reading of the DataFlash chips status register
- Fix: In the Save menu, the cancel button causes an error message
- Fix: Incorrect names of DataFlash chips
- Fedora repository added
- MacOS build support added
- Uninstall script added

* Sat Sep 07 2024 Mikhail Medvedev 1.4.3-3
- added license file to the spec file

* Tue Sep 03 2024 Mikhail Medvedev 1.4.3-2
- added translations to the spec file

* Wed Aug 21 2024 Mikhail Medvedev 1.4.3-1
- initial release
