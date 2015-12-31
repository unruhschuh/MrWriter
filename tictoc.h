/*
#####################################################################
Copyright (C) 2015 Thomas Leitz (thomas.leitz@web.de)
#####################################################################

LICENSE:

This file is part of MrWriter.

MrWriter is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License 2.0 as published
by the Free Software Foundation.

MrWriter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MrWriter.  If not, see <http://www.gnu.org/licenses/>.
#####################################################################
*/

#ifndef TICTOC_H
#define TICTOC_H

#include <stack>
#include <iostream>

// tic toc code from http://stackoverflow.com/a/13485583/1580081

std::stack<clock_t> tictoc_stack;

// changed it to output to stderr
void tic() {
    tictoc_stack.push(clock());
}

void toc() {
    std::cerr << "Time elapsed: "
        << ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC
        << std::endl;
        tictoc_stack.pop();
}


#endif // TICTOC_H

