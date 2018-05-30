#include <kipr/botball.h>

int main()
{
    camera_open();
    while(1)
    {
    center_ball();
    }
    camera_close();
    return 0;
}
