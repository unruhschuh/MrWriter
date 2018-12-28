#ifndef TICTOC_H
#define TICTOC_H

#include <stack>
#include <iostream>

// tic toc code from http://stackoverflow.com/a/13485583/1580081

std::stack<clock_t> tictoc_stack;

// changed it to output to stderr
void tic()
{
  tictoc_stack.push(clock());
}

void toc()
{
  std::cerr << "Time elapsed: " << ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC << std::endl;
  tictoc_stack.pop();
}

#endif // TICTOC_H
