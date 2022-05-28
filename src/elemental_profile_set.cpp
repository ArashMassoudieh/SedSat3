#include "elemental_profile_set.h"

Elemental_Profile_Set::Elemental_Profile_Set()
{

}

Elemental_Profile_Set::Elemental_Profile_Set(const Elemental_Profile_Set& mp)
{
    elemental_profiles = mp.elemental_profiles;
}

Elemental_Profile_Set& Elemental_Profile_Set::operator=(const Elemental_Profile_Set &mp)
{
    elemental_profiles = mp.elemental_profiles;
    return *this;
}
