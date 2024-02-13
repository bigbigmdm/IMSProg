# EZP2019-EZP2025_chip_data_editor
Qt based editor  chip database for EZP2019, EZP2019+, EZP2020, EZP2023, EZP2025, MinPro, XP866+, MinproI programmer devices.

![Screenshot editor](https://github.com/bigbigmdm/EZP2019-EZP2025_chip_data_editor/blob/main/img/ezp_editor.png)

Press the button ![read](img/open.png) for open the database file (Depending on the name of your programmer device EZP2019.Dat, EZP2020.Dat, EZP2023+.Dat, EZP2025.Dat, XP866+.Dat, MinproI.Dat).

For save the changes to binary file press the button save ![save](img/save.png).

For deleting the strings select one or more strings and press the delete button ![delete](img/del.png).

You can moving the selected string use the arrow buttons (![undo](img/undo.png) and ![redo](img/redo.png)).

You can copying the selected string use the plus button ![plus](img/plus.png).

The selected strings may be converted info .CSV format by pressing the button ![tocsv](img/tocsv.png). 

If you want convert all strings in to .CSV file use the `Export to CSV` ![tocsv](img/tocsv.png) in `File` menu.

If you want import .CSV database in to program use `Import from CSV` ![import](img/import.png) in `File` menu.

Any cell is editable.

## Building

```
mkdir build
cd build
cmake ..
make -j4
sudo make install
```
