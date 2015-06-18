# "Adventure"
A school project for a simple "adventure" game like Rogue but nowhere near
as feature-rich.

##How the game works:

If you have ever played a text-based adventure game like Rogue, you are
probably familiar with the action: you are in a room and you are offered a
list of directions (read: other rooms) to enter, then you enter the room
to which you want to move, you get another list, etc. You continue to do
this until you find the "end room." Your moves are limited to only the
rooms that connect with the one in which you're standing. Pretty simple.

There are 10 rooms total so it's not easy to get lost here. The rooms are
all named after classical, baroque, and nocturne composers (I may have
left out a classification). When you have finally reached the "end room"
you will be presented with the path that you followed, which boils down to
an ordered list of rooms that you traversed on your way to the end. If you
burn through too many movements, you'll eventually lose. This can't go on
forever.

This program was an exercise in filesystem I/O. As such, the functionality
focuses more on that than entertainment value.

##How this program works:

Just compile the program and run the resulting executable file. 

When the program first starts, a random list of room files will be built
in a subdirectory and each file will contain a random list of adjacent
room names as well as the room's designation (start room, mid room, or end
room). The room files and the subdirectory that was created to house them
are not removed upon completion of the game, so you will need to do some
housekeeping when you're through playing. That was part of the assignment,
and we're sticking with purity here.

##Build:

Download the single source code file and compile it directly with GCC:

gcc -Wall -Werror -o adventure adventure.c

##Colophon:

This program was written with standards in mind but was only
developed and tested on Linux and Mac OS. Mostly Linux.

GCC is required. The program is 100% C, no C++ or other C
variants.

Normally I would create a header file for specifications and prototypes,
but again we're sticking with purity here. That was not permitted in the
assignment so there is no header here.

##Disclaimer:

This is code from a school project. It satisfies assignment requirements
but is nowhere near as "scrubbed" as released software should be.
Security is not addressed, only functionality and no input
validation. If you use this code for anything other than satisfying your
curiosity, please keep the following in mind:

- there is no warranty of any kind (you use the code as-is)
- there is no support offered, please do not ask
- there is no guarantee it'll work, although it's not complex so it should
  work
- please do not take credit for code you did not write, especially if you
  are a student. NO CHEATING.

Thanks!
