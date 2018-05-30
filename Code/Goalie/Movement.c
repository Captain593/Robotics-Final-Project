//Contains all movement functions for goalie
int isright = -1;
int currentDegrees = 0;

enum{BALL, GOAL, GOALIE, KICKER};

//center onto the ball
void center_ball(){
    center(BALL);
}
void center(int type, int yVal){
    int offset = 20;
    //Start by centering the ball on the x axis
    int* coordinates;
    int x = 0;
    while(1){
         //Update our x position
        coordinates = get_position(type);
        x = coordinates[0];
		//if robot cannot see ball, don't move
		if(x == -1)
        {
          motor(2,0);
          motor(0,0);
            break;
        }
		//get middle of screen
    	int middle = round(get_camera_width() / 2.0);
		//while the ball is centered, stop moving
        while(x < (middle + offset + 20) && x > (middle + offset - 20)){
            motor(2,0);
        	motor(0,0);
            printf("Let's stop for now\n");
            coordinates = get_position(type);
        	x = coordinates[0];
            middle = round(get_camera_width() / 2.0);
        }
    	//If we are on the left side of the robot, turn left to center the ball until it is centered
    	if(x!= -1 && x < middle + offset){
            motor(2,100);
        	motor(0,100);
             printf("To the left!\n");
    	}
    	//If we are on the right side of the robot, turn right to center the ball until it is centered
    	else if (x!= -1 && x > middle - offset){
            motor(0,-100);
        	motor(2,-100);
             printf("To the right!\n");
    	}
		//if ball is close, kick it
        if(in_ball_range() == 1)
        {
         printf("Time to smack the ball away!\n");
         enable_servos();
         set_servo_position(1,1000);
            msleep(500);
         set_servo_position(1,2000);
            msleep(1200);
         set_servo_position(1,1000);
            msleep(1200);
         disable_servos();
        }
    }
    
    return;
}

//Previous iteration of the main loop. Was scrapped and NOT used due to overcomplexity. It is included to show that more time was spent in writing the code
void do_your_job()
{
    int offset = 20;
    int* coordinates;
    int x_ball = 0, x_goal = 0, i = 0, diff = 0;
    int middle = round(get_camera_width() / 2.0);
    coordinates = get_position(0);
    x_ball = coordinates[0];
    coordinates = get_position(1);
    x_goal = coordinates[0];
   printf("Middle: %d, x of ball: %d\n",middle, x_ball);
    /*if (x_ball == -1)
    {
        switch(turn)
        {
            case 0:
                motor(0,100);
                motor(2,-100);
                turn = 1;
                break;
            case 1:
                motor(0,-100);
                motor(2,100);
                turn = 2;
                break;
            case 2:
                motor(0,-100);
                motor(2,100);
                turn = 3;
                break;
            case 3:
                motor(0,100);
                motor(2,-100);
                turn = 0;
                break;
        }
        msleep(300);
    }*/
    /*if(x_ball != -1)
    {
        
    }*/
   //turn right if ball is seen
    if(x_ball > middle + offset && x_ball != -1)
    {
        isright = 1;
        //turn until robot sees cone
        coordinates = get_position(1);
        x_goal = coordinates[0];
		//while goal is not seen, move until it is seen
        while(x_goal == -1)
        {
            motor(0,40);
            motor(2,-40);
            coordinates = get_position(1);
        	x_goal = coordinates[0];
        }
		//stop moving
        motor(0,0);
        motor(2,0);
        printf("I have positioned myself infront of the cone");
    }
	//turn left if ball is seen
    else if(x_ball < middle - offset && x_ball != -1)
    {
        isright = 0;
        coordinates = get_position(1);
        x_goal = coordinates[0];
		//while goal is not seen, move until it is seen
        while(x_goal == -1)
        {
            motor(0,-40);
            motor(2,40);
            coordinates = get_position(1);
        	x_goal = coordinates[0];
        }
        motor(0,0);
        motor(2,0);
        printf("I have positioned myself infront of the cone(2)");
    }
    coordinates = get_position(1);
    x_goal = coordinates[0];
	//assumes goal is seen
    if(x_goal != -1)
    {
        coordinates = get_position(1);
        x_goal = coordinates[0];
		//back up if goal is lost or too close to goal
      if(in_goal_range() == 1 || x_goal == -1)
            {
                printf("Backing up!\n");
           		motor(0,-100);
           		motor(2,-100);
                msleep(200);
          		if(isright == 1)
                {
          		motor(0,-100);
           		motor(2, 100);
          		
                }
          		else if(isright == 0)
                { 
          		motor(0,100);
           		motor(2, -100);
                }
          		msleep(600);
                motor(0,0);
       		    motor(2,0);	
            }
	// if it sees goal, move towards goal
       else if(x_goal!= -1)
       {
           printf("I see the goal!\n");
           motor(0,100);
           motor(2,100);
           msleep(500);
           motor(0,0);
       	   motor(2,0);
       }
    }
    motor(0,0);
    motor(2,0);
    
}