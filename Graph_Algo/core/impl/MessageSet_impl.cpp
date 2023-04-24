//
// Created by Thoh Testarossa on 2019-05-25.
//

#include "../MessageSet.cpp"
#include "../../algo/PageRank/PageRank.h"
#include "../../algo/DDFS/DDFS.h"
#include "../../algo/LabelPropagation/LabelPropagation.h"

template class Message<double>;
template class Message<int>;
template class Message<LPA_MSG>;
template class Message<PRA_MSG>;
template class Message<DFSMSG>;

template class MessageSet<double>;
template class MessageSet<int>;
template class MessageSet<std::pair<int, int>>;
template class MessageSet<PRA_MSG>;
template class MessageSet<DFSMSG>;
template class MessageSet<LPA_MSG>;
