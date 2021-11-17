#include <iostream>
#include <string>

#include "Crawler.h"

using namespace std;

int main(int ac , char *av[]) {
    if (ac == 1) {
        return -1;
    }

    Crawler crawler(av[1]);

    crawler.process();

    return 0;
}
