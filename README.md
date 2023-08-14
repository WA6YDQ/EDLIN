# EDLIN
Everyone else seems to have their own copy of edlin - here's mine.

When started without a filename, you will be in command mode. Press 'a'
to start appending to the buffer. Type '.' (a period) by itself on a line
to return to command mode.

When started with a filename, the named file will be loaded first. All commands
apply to that file.

When adding text in append mode (or insert mode) pressing '.' (period) by 
itself on a line will return to command mode.

In command mode, the following single letters apply:

q    Exit edlin (any modified file will NOT be saved. There are no warnings.)
w [filename]  Write the text buffer to supplied filename
l [filename]  Load the file filename to the buffer. The buffer is NOT cleared first.
n    New. Delete the current buffer, reset pointers.
p    Print the buffer (ie list). The buffer will be displayed in it's entirety.
a    Append text to the end of the buffer.
s [substring]   Search for a string in the buffer.
v    Show buffer stats (buffer size)
n-m d    Delete lines n-m   ie 10-20d  will delete lines 10 thru 20
n-m p    Print lines n-m    ie 15-40p  will display lines 15 thru 40 on the terminal
n i      Insert lines BEFORE line n  ie 15i  start inserting BEFORE line 15

NOTE: a comma can be used in place of the dash for the above 3 commands.
ie  10,20l

There are probably bugs here. Please let me know when you find some.
The buffer increases as needed using realloc. Running against valgrind
found no issues (so far).

Filenames for load/save can include directories.
To build, type make. If you want a static build (and static gcc libs exist
on your system) type make static

To install to /usr/local/bin, type make install

Enjoy!

- Kurt

There 
