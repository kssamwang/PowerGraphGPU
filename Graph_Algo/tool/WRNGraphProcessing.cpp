//
// Created by Thoh Testarossa on 2019/10/24.
//

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <map>

#define RETAININGRATE 0.01
#define MAXEDGEWEIGHT 1000

// Analysis part

// Returns the set of edges which go beyond partition boundary
std::vector<std::string> partitionAnalysis(const std::string &fileName, int partitionNum)
{
    auto result = std::vector<std::string>();

    std::ifstream fin = std::ifstream(fileName);

    if(!fin.is_open())
        std::cout << "File not found!" << std::endl;
    else
    {
        int srcVCount, dstVCount, eCount;
        fin >> srcVCount >> dstVCount >> eCount;

        std::cout << "Total vertex count: " << srcVCount << std::endl;
        std::cout << "Total edge count: " << eCount << std::endl;

        std::cout << "Partition boundary: ";
        for(int i = 0; i <= partitionNum; i++)
            std::cout << ((srcVCount + 1) * i) / partitionNum << " ";
        std::cout << std::endl;

        int *partitionCountSet = new int [partitionNum];
        for(int i = 0; i < partitionNum; i++)
            partitionCountSet[i] = 0;

        for(int i = 0; i < eCount; i++)
        {
            int src, dst;
            fin >> src >> dst;
            int srcP = (src * partitionNum) / (srcVCount + 1), dstP = (dst * partitionNum) / (dstVCount + 1);
            if(srcP == dstP) partitionCountSet[srcP]++;
            else
                result.emplace_back(std::string("Edge(") + std::to_string(src) + std::string(", ") + std::to_string(dst) + std::string("), ") + std::to_string(srcP) + std::string(" <-> ") + std::to_string(dstP));
        }
    }

    fin.close();

    return result;
}

// Generation part

// Output the testGraph generated from input graph
void generateGraph(const std::string &fileName, int partitionNum)
{
    std::random_device r_direct;
    std::uniform_int_distribution<int> uniform_d_dist(0, 1);
    std::default_random_engine e_d(r_direct());

    std::random_device r_retain;
    std::uniform_real_distribution<double> uniform_r_dist(0, 1);
    std::default_random_engine e_r(r_retain());

    std::random_device r_weight;
    std::uniform_int_distribution<int> uniform_w_dist(0, MAXEDGEWEIGHT);
    std::default_random_engine e_w(r_weight());

    std::ifstream fin = std::ifstream(fileName);

    if(!fin.is_open())
        std::cout << "File not found!" << std::endl;
    else
    {
        int srcVCount, dstVCount, eCount;
        fin >> srcVCount >> dstVCount >> eCount;

        int o_vCount = srcVCount, o_eCount = 0;

        std::string outputFileName_testGraph = std::string("testGraph_") + fileName + std::string(".txt");
        std::ofstream fout_testGraph = std::ofstream(outputFileName_testGraph);

        //std::string outputFileName_analysis = std::string("analysis_") + fileName + std::string(".txt");
        //std::ofstream fout_analysis = std::ofstream(outputFileName_analysis);

        auto outputString_testGraph = std::string();
        //auto outputString_analysis = std::string();

        for(int i = 0; i < eCount; i++)
        {
            int o_src, o_dst, o_weight;

            fin >> o_src >> o_dst;

            //Random weight generation
            o_weight = uniform_w_dist(e_w);

            //Random direction generation
            if(uniform_d_dist(e_d) == 1)
                std::swap(o_src, o_dst);

            //Check partition status
            int srcP = (o_src * partitionNum) / (srcVCount + 1), dstP = (o_dst * partitionNum) / (dstVCount + 1);
            if(srcP == dstP || uniform_r_dist(e_r) <= RETAININGRATE)
            {
                outputString_testGraph += std::to_string(o_src) + std::string(" ") + std::to_string(o_dst) + std::string(" ") + std::to_string(o_weight) + std::string("\n");
                //outputString_analysis += std::to_string(o_src) + std::string(" ") + std::to_string(o_dst) + std::string("\n");
                o_eCount++;
            }
            else;
        }

        fout_testGraph << o_vCount << " " << o_eCount << std::endl;
        fout_testGraph << outputString_testGraph;

        std::cout << "File " << outputFileName_testGraph << " generated with " << o_vCount << " vertices and " << o_eCount << " edges" << std::endl;

        //fout_analysis << o_vCount << " " << o_vCount << " " << o_eCount << std::endl;
        //fout_analysis << outputString_analysis;

        //std::cout << "File " << outputFileName_analysis << " generated with " << o_vCount << " vertices and " << o_eCount << " edges" << std::endl;

        fin.close();
        fout_testGraph.close();
        //fout_analysis.close();
    }
}

int main(int argc, char *argv[])
{
    if(argc == 3 && (std::string(argv[1]) == "analysis" || std::string(argv[1]) == "generate"))
    {
        if(std::string(argv[1]) == "analysis")
        {
            std::cout << "Pls input partition number: ";
            int partitionNum;
            std::cin >> partitionNum;
            auto result = partitionAnalysis(std::string(argv[2]), partitionNum);

            std::cout << "Edges go beyond the partition: " << result.size() << std::endl;
            for(int i = 0; i < result.size(); i++)
                std::cout << result.at(i) << std::endl;
        }

        if(std::string(argv[1]) == "generate")
        {
            std::cout << "Pls input partition number: ";
            int partitionNum;
            std::cin >> partitionNum;

            generateGraph(std::string(argv[2]), partitionNum);
        }
    }
    else
        std::cout << "Usage: ./tool_WRNGraphProcessing analysis|generate fileName" << std::endl;

    return 0;
}
