c4c
===

c4c is a Connect 4 game written in C, using ncurses.

TODO
----

 - Make the game (actually) playable
 - Add netplay

NOTES
-----

 - c4c should be used with a UTF-8 aware version of ncurses,
  and a terminal and font that can render unicode characters.

 - To compile it, simply run `make -j`.

 - If the variable `ASCII` is set when compiling, the game
  will use ASCII characters instead of unicode ones.
