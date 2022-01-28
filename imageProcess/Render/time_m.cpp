#include "time_m.h"
#include <sys/timeb.h>
namespace  Engine{
long long systemtime()
{
    timeb t;
    ftime(&t);
    return t.time*1000+t.millitm;
}
}
