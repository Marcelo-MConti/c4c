  - [ ] Fix signal handling hack for centered windows;
        causes a segmentation fault on recent versions of ncurses
        (perhaps even rework it to use callbacks to update
        the rest of the screen)

  - [ ] Stop using function pointers for player move functions
        (just weird)
 
  - [ ] Implement netplay with threads (decide on a message format and
        transport protocol for message passing --- either UDP or TCP)
