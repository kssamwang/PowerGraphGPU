//
// Created by cave-g-f on 10/16/19.
//

#include "../algo/JumpIteration/JumpIteration.h"
#include "../core/GraphUtil.h"
#include "../srv/UtilServer.h"
#include "../srv/UNIX_shm.h"
#include "../srv/UNIX_msg.h"

#include <iostream>

int main(int argc, char *argv[])
{
    if(argc != 4 && argc != 5)
    {
        std::cout << "Usage:" << std::endl << "./UtilServerTest_BellmanFord vCount eCount numOfInitV [nodeNo]" << std::endl;
        return 1;
    }

    int vCount = atoi(argv[1]);
    int eCount = atoi(argv[2]);
    int numOfInitV = atoi(argv[3]);
    int nodeNo = (argc == 4) ? 0 : atoi(argv[4]);

    auto testUtilServer = UtilServer<JumpIteration<double, double>, double, double>(vCount, eCount, numOfInitV, nodeNo);
    if(!testUtilServer.isLegal)
    {
        std::cout << "mem allocation failed or parameters are illegal" << std::endl;
        return 2;
    }

    testUtilServer.run();
}