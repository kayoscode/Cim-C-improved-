
#ifndef INCLUDE_COLOR_H
#define INCLUDE_COLOR_H

#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KBLD "\x1B[1m"
#define KUNL "\x1B[4m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) KBLD x RST
#define UNDL(x) KUNL x RST

#define SET_COLOR_RED std::cout << KRED
#define SET_COLOR_GREEN std::cout << KGRN
#define SET_COLOR_YELLOW std::cout << KYEL
#define SET_COLOR_BLUE std::cout << KBLU
#define SET_COLOR_MAGENTA std::cout << KMAG
#define SET_COLOR_CYAN std::cout << KCYN
#define SET_COLOR_WHITE std::cout << KWHT

#define SET_COLOR_BOLD std::cout << KBLD
#define SET_COLOR_UNDERLINE std::cout << KUNL

#define COLOR_RESET std::cout << RST

#endif
