#include "elemental_profile.h"

Elemental_Profile::Elemental_Profile()
{

}

Elemental_Profile::Elemental_Profile(const Elemental_Profile& mp)
{
    profile = mp.profile;
}

Elemental_Profile& Elemental_Profile::operator=(const Elemental_Profile &mp)
{
    profile = mp.profile;
    return *this;
}

