int resetBot = 0;
int ballDegrees = -1;
int foundGoal = 0;
int haveBall = 0;
int centeredGoal = 0;
int fooled = 0;

/*
   This function handles all the logic of the KICKER
 */
void kicker(){
        //If the bot is reset (meaning the bot kicked the ball), backup and turn until there are no more cones, then move forward a distance
        if(resetBot == 1) {
                if(in_goal_range() > 0)
                {
                        motor(0,-100);
                        motor(1,100);
                        msleep(50);
                }
                else{
                        motor(0,-100);
                        motor(1,100);
                        msleep(1200);
                        motor(0,100);
                        motor(1,100);
                        msleep(2000);
                        resetBot = 0;
                }
                return;
        }

//If we dont have the ball, track the ball color (track has a built in search function if no ball is found)
        if(haveBall == 0) {
                track_color(0);
                //If the ball is close enough, grab the ball
                if(have_ball() == 1) {
                        printf("Grabbing ball\n");
                        //center_ball();
                        grab_ball();
                        //pick_up_ball();
                        haveBall = 1;
                        clear_motor_position_counter(0);
                        clear_motor_position_counter(1);
                        printf("Grabbed Ball\n");
                }
        }

        //If we have the ball
        if(haveBall == 1) {
                //Go up to the goal and center if its not already
                if(centeredGoal == 0) {
                        printf("Lining up\n");
                        if(in_goal_range() < 2)
                                line_up_goal();
                        else
                                centeredGoal = 1;
                }
                //If the goalie hasnt been fooled, then try to fool the goalie
                //The fooled variable can be set back to 0 if it is determined
                //that the current trajectory wont work
                else if(fooled == 0) {
                        printf("Fooling Goalie\n");
                        fooled = foolGoalie();
                }
                //If we are facing the gap, move forward
                else{
                        printf("Fooled Goalie\n");
                        fooled = foolGoalie();
                        motor(0,100);
                        //Compensate for slightly faster motor
                        motor(1,97);
                        msleep(300);
                }


//If we should kick, then kick the ball and reset the bot
                if(should_kick() == 1) {
                        //kick ball
                        printf("Kick!\n");
                        kick();
                        ballDegrees = -1;
                        foundGoal = 0;
                        haveBall = 0;
                        centeredGoal = 0;
                        fooled = 0;
                        reset_movement();
                        reset_detect();
                        motor(0,-100);
                        motor(1,-100);
                        msleep(1000);
                        resetBot = 1;
                }
        }



}
