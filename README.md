# automata
(https://github.com/vidal-adrien/automata)
Windowed application for Conway's game of life and any other life like automaton (2D square grid, binary state, adjacency rule).

The Game of Life, also known simply as Life, is a cellular automaton devised by the British mathematician John Horton Conway in 1970.
The "game" is a zero-player game, meaning that its evolution is determined by its initial state, requiring no further input.
One interacts with the Game of Life by creating an initial configuration and observing how it evolves.

This application can run all 2*(2**8) rule variations. Rules are coded Bb1b2.../Ss1s2... with:

    b(i) (birth)       -> a  number of neighbours at which a cell transitions from death to life.
    s(i) (survival)  -> a number of neighbours at which a live cell remains alive.

Features:

    Paint generations to the infinity.
    Simulate step by step or automatically.
    Set time interval between generations.
    Set cells color.
    Save and load patterns.
    Browse patterns from the application.
    Freely move around the grid.

The simulation happens on a finite universe. Thus some patterns will not work like on an infinite plane.

https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life

https://en.wikipedia.org/wiki/Life-like_cellular_automaton



