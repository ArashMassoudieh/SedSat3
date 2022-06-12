#include "conductor.h"

Conductor::Conductor()
{

}

bool Conductor::Execute(const string &command, map<string,string> arguments)
{
    if (command == "GA")
    {
        if (GA!=nullptr) delete GA;
        GA = new CGA<SourceSinkData>(Data());
    }
    return true;
}
