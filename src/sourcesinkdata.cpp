#include "sourcesinkdata.h"

SourceSinkData::SourceSinkData()
{

}

SourceSinkData::SourceSinkData(const SourceSinkData& mp)
{
    sources = mp.sources;
    targets = mp.targets;
}
SourceSinkData& SourceSinkData::operator=(const SourceSinkData &mp)
{
    sources = mp.sources;
    targets = mp.targets;
    return *this;
}
