#include <kipr/botball.h>

enum {KICKER, GOALIE};

int main()
{
        //Define the robots position
        int position = KICKER;
        camera_open();
        if(position == KICKER) {
                default_servo_position();
        }
        while(1) {
                //Update the camera frame and dont continue until the frame is new
                while(camera_update() != 1) {
                        continue;
                }

                if(position == KICKER) {
                        kicker();
                        //  debug_kicker();
                }
                else if(position == GOALIE) {
                        goalie();
                }
        }
        camera_close();
        return 0;
}
