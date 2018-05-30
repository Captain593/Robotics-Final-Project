//Contains all movement functions for kicker and goalie
int conesLastSeen = 0;
int ballLastSeen = -2;
int countdown = 10;
int prevx = -1;
int prevSpeed = -1;
int decayTime = 0;

//Resets the globals
void reset_movement(){
        conesLastSeen = 0;
        ballLastSeen = 1;
        countdown = 10;
        prevx = -1;
        prevSpeed = -1;
        decayTime = 0;
}

void turnTo(int degrees);
int x_to_degrees(int x);
int currentDegrees = 0;
int y_to_speed(int y);
void move_and_turn(int speed, int degrees);
int get_current_degrees();
void kick();
void center_and_grab_ball(int ballType);
void default_servo_position();
enum {BALL, GOALIE, GOAL, KICKER};

void center_goalie(){
        center(GOALIE);
}

/*
   Used in conjunction with find_gap() to center the kicker to the center of the gap between
   the goalie and the cone.
 */
int foolGoalie(){
        int gap = find_gap();
        int range = 10;
        int middle = round(get_camera_width() * 0.5);
        //If the gap shrunk alot, make sure we didnt miss a cone, go back to where we were last time
        if(gap == -2 && 0) {
                int** goalPos = get_goal_position();
                if(in_goal_range() < 2) {
                        if(goalPos[0][0] > middle) {
                                motor(0,30);
                                motor(1,-30);
                        }
                        else{
                                motor(0,-30);
                                motor(1,30);
                        }
                        msleep(20);
                        return 0;
                }

        }
        if(gap < 0) {
                //Tell the kicker that we lost the goalie and at least 1 cone, so we need to recenter
                if(get_goal_position() < 2)
                        return -1;
                return 0;
        }



        printf("Gap: %d Middle: %d\n",gap,middle);
        if(gap < (middle + range) && gap > (middle - range)) {
                return 1;
        }
        else if(gap < middle) {
                motor(0,-30);
                motor(1,30);
        }
        else{
                motor(0,30);
                motor(1,-30);
        }
        msleep(100);
        return 0;
}


/*
   Taken and modified from the computer vision lab. Used to track the ball and keep it in the center of the robot.
   It modulates the speed of the wheels instead of using discrete movements, giving a smoother appearance when tracking.
   It's set up to track any color but for this project it only tracks the color of the ball
 */
void track_color(int color){
        int x = -1;
        int y = -1;
        int width = 0;
        int height = 0;

        if(get_object_count(color,0) > 0) {
                x = get_object_center_x(color,0);
                y = get_object_center_y(color,0);
                width = get_object_bbox_width(color,0);
                height = get_object_bbox_height(color,0);
        }

        int leftInt = 1;
        int rightInt = 1;
        int speed = 50;
        int bias = 40;

        //If the object is too small, or we have 0 detected objects then we know we've lost the ball
        if(get_object_count(color) == 0 || width < 7 || height < 7) {
                if(prevx < 0) {
                        motor(0, 50);
                        motor(1, -50);
                        return;
                }
                //If the ball was on either edge last time we checked, start turning in that direction until we find the ball again
                else if(((prevx * 1.0) / (get_camera_width() * 1.0)) > 0.7) {
                        motor(0, 50);
                        motor(1, -50);
                        return;
                }
                else if(((prevx * 1.0) / (get_camera_width() * 1.0)) > 0 && ((prevx * 1.0) / (get_camera_width() * 1.0)) < 0.3) {
                        motor(0, -50);
                        motor(1, 50);
                        return;
                }
                //Maintain previous course and speed if the ball wasn't on an edge (meaning it went too far away for the camera to detect
                else{
                        float left = 1.0;
                        float right = 1.0;

                        left = (160.0 - prevx) / 160.0;
                        right = prevx / 160.0;

                        leftInt = round(prevSpeed * left);
                        rightInt = round(prevSpeed * right);
                        printf("Left: %d Right:%d\n",prevx,rightInt);
                        motor(0, rightInt * 2);
                        motor(1, leftInt * 2);
                }
        }
        else{
                //Set the previous x for use with the lost ball condition
                prevx = x;
                //Speed at which the robot should travel towards the object
                speed = 0;
                //Set speed inversly proportional to the y to camera height ratio
                speed = 100 - round(100 * ((y * 1.0) / (get_camera_height() * 1.0)));
                //Left/right split depending on where the ball is
                float left = 1.0;
                float right = 1.0;

                //Left is proportinal to the ratio of x to camera width
                left = x / (1.0 * get_camera_width());
                //Right is inversly proportional to the ratio of x to camera width
                right = (1.0 * get_camera_width() - x) / (1.0 * get_camera_width());

                //Calculate the speed distribution to both wheels
                leftInt = round(speed * left);
                rightInt = round(speed * right);
        }
        //Store the previous speed for the lost ball condition
        prevSpeed = speed;


        //Motors move based on the calculated distribution of speed plus the bias
        motor(0, (leftInt * 2) + bias);
        motor(1, (rightInt * 2) + bias);
}

/*
   Depreciated function that was used to track the ball, was too jerky
 */
void follow_ball(){
        int left = 50;
        int right = 50;
        int* coordinates = get_position(BALL);
        int x = coordinates[0];
        int y = coordinates[1];
        int speed = 100 - round(y / (get_camera_height() * 1.0));
        if(x == -1) {
                left = ballLastSeen * 60;
                right = ballLastSeen * -60;
        }
        else{
                if(x < (get_camera_width() / 2.0))
                        ballLastSeen = -1;
                else
                        ballLastSeen = 1;

                left = round(50 * (x / (get_camera_width()/2.0))) - 50;
                right = 100 - round(50 * (x / (get_camera_width() / 2.0))) + 50;
                printf("Left: %d Right: %d\n", left, right);
        }


        motor(0,left);
        motor(1,right);
        msleep(50);

}

/*
   Used to rewind the motors back to 0
   Initially was used to go back to the goal.
   The motors were cleared when a goal was detected, then when this function was called it would
   go back to where the goal was last seen.
   Since it only retraces distance, and not the speed at a given time, this would often get the robot stuck on an object,
   so this method was not used
 */
void back_to_goal(){
        int speed = -50;
        int motorLeft = 1;
        int motorRight = 1;
        double multiple = (get_motor_position_counter(0) * 1.0) / (get_motor_position_counter(1) * 1.0);
        if(get_motor_position_counter(0) < 0)
                motorLeft *= -1;
        if(get_motor_position_counter(1) < 0)
                motorRight *= -1;
        motor(0, motorLeft * round(50 * multiple));
        motor(1, motorRight * round(50 / multiple));

        while(get_motor_position_counter(0) > 0 || get_motor_position_counter(1) > 0) {
                if(get_motor_position_counter(0) <= 0)
                        motor(0,0);
                if(get_motor_position_counter(1) <= 0)
                        motor(1,0);
                if(in_goal_range() == 2) {
                        motor(0,0);
                        motor(1,0);
                        return;
                }
                msleep(20);
        }
}


/*
   Used to line the kicker up with the center of the two cones.
   This was used in place of a goalie when testing before the goalie was created
   Once the goalie was created, this function was used to initialy line up the KICKER
   with the goal, then it let other functions determine where to actually shoot once both cones and the goalie were detected
 */
void line_up_goal(){
        int** coordinates;
        int* goalie = get_goalie_position();
        int left = 0;
        int right = 0;
        int middle = round(get_camera_width() * 0.5);

        if(decayTime > 0)
                decayTime--;

        coordinates = get_goal_position();
        //Too close to cones, back up
        if(cone_too_big() == 1) {
                printf("Cone too big! backing up\n");
                motor(0,-100);
                motor(1,-100);
                msleep(200);
                motor(0,0);
                motor(1,0);
                return;
        }

        if(coordinates[0][0] == -1 && coordinates[1][0] == -1) {
                printf("No Cones found, random search\n");
                //We have no goals, move in a random direction to find goals
                left = -80;
                right  = 80;
                decayTime = 100;
                conesLastSeen = 0;
        }
        //If there is only one cone, try to find the other cone
        else if(coordinates[1][0] == -1) {
                printf("One Cone found\n");
                if(goalie[0] > -1) {
                        if(goalie[0] > coordinates[0][0]) {
                                left = 30;
                                right = -30;
                        }
                        else{
                                left = -30;
                                right = 30;
                        }
                }
                else{
                        left = -30;
                        right = 30;
                }
                conesLastSeen = 1;
        }
        else if(coordinates[1][0] != -1 || coordinates[0][0] != -1) {
                printf("Two cones found\n");
                conesLastSeen = 2;
                left = 0;
                right = 0;
                //int middlex = (coordinates[1][0] - coordinates[0][0]);
                int middlex = round((coordinates[1][0] + coordinates[0][0]) * 0.5);
                int middle = round(get_camera_width() * 0.5);
                int distFromCone1 = middle - coordinates[1][0];
                int distFromCone2 = coordinates[0][0] - middle;
                printf("Middle: %d Middlex: %d", middle, middlex);
                //if((coordinates[0][0] + (middlex/4.0)) < middle && (coordinates[1][0]- (middlex/4.0)) > middle){
                if(middlex < (middle + 40) && middlex > (middle - 40)) {
                        //if(distFromCone1 < (distFromCone2 + 20) & distFromCone1 > (distFromCone2 - 20)){
                        printf("Centered\n");
                        // motor(0,100);
                        //motor(1,100);
                        //msleep(500);
                        return;
                }
                if((middlex * 2) < middle) {
                        motor(0,-30);
                        motor(1,30);
                        msleep(100);
                }
                else {
                        motor(0,30);
                        motor(1,-30);
                        msleep(100);
                }
                return;
        }

        printf("Left: %d Right: %d\n",left,right);
        motor(0,left);
        motor(1,right);
        msleep(10);
}

/*
   Old function used to statically center the goal.
   It was a modified version of center() and it didn't work well so the previous
   function was created as a replacement
 */
void center_goal(){
        int offset = 0;
        motor(0,0);
        motor(1,0);
        //Start by centering the ball on the x axis
        int** coordinates;
        int x = 0;
        int left = -30;
        int right = 30;
        while(1) {
                //Update our x position
                coordinates = get_goal_position();
                int x1 = coordinates[0][0];
                int x2 = coordinates[1][0];
                int middlex = x2 - x1;

                int middle = round(get_camera_width() / 2.0);
                printf("Middle: %d, Middlex: %d (%d - %d)\n", middle, middlex, x2, x1);
                //If the ball aprox in the center, exit the loop
                if((x1 + (middlex/4.0)) < (middle) && (x2 - (middlex/4.0)) > (middle) && (in_goal_range() == 2)) {
                        printf("Centered\n");
                        break;
                }

                int dontChange = 0;
                int goals = 0;
                while((goals = in_goal_range()) < 2) {
                        motor(0,0);
                        motor(1,0);

                        printf("Goals: %d\n", goals);

                        if(goals == 0) {
                                dontChange = 0;
                                //Try backing up and reverse spinning directions
                                motor(0,-40);
                                motor(1,-40);
                                msleep(500);
                                left *= -1;
                                right *= -1;
                                while(in_goal_range() == 0) {
                                        printf("Going %d, %d\n", left, right);
                                        motor(0,left);
                                        motor(1,right);
                                        msleep(10);
                                }
                                motor(0,0);
                                motor(1,0);
                                continue;
                        }
                        //Spin around until we find both goals

                        if(get_object_center_y(GOAL,0) < round(get_camera_width() / 2.0) && dontChange == 0) {
                                left = -30;
                                right = 30;
                                dontChange = 1;
                        }
                        else if(dontChange == 0) {
                                left = 30;
                                right = -30;
                                dontChange = 1;
                        }



                        motor(0,left);
                        motor(1,right);
                        msleep(30);
                }
                motor(0,0);
                motor(1,0);

                //If we are on the left side of the robot, turn left to center the ball until it is centered
                if(middlex > 0 && middlex < middle) {
                        left = 30;
                        right = -30;
                }
                //If we are on the right side of the robot, turn right to center the ball until it is centered
                else if (middlex > 0) {
                        left = -30;
                        right = 30;
                }

                motor(0,left);
                motor(1,right);
                msleep(50);
        }
        motor(0,0);
        motor(1,0);
        return;
}

void center_ball(){
        center(BALL);
}

//Moves the robot back to a degree based on where it is now
void turnTo(int degrees){
        if(degrees - currentDegrees < 0) {
                motor(0,30);
                motor(1,-30);
        }
        else{
                motor(0,-30);
                motor(1,30);
        }

        //move_and_turn(0, degrees - currentDegrees);
}

//Modulates the servo to put the cage around the ball
pick_up_ball(){
        enable_servos();
        set_servo_position(0, 700);
        msleep(300);
        disable_servos();
}

//Converts the x coordinates to degrees
int x_to_degrees(int x){
        int val = round((x * 1.0) / (get_camera_width() * 1.0) * 10.0) - 5;
        printf("Degrees: %d -> %d\n", x, val);
        return val;
}

int y_to_speed(int y){
        int val = (100 - round(100.0 * ((y * 1.0) / (get_camera_width() * 1.0))));
        printf("speed: %d -> %d\n", y, val);
        return val;
}

/*
   Old function from when I was monitoring the degrees of the KICKER
   This was going to be used to get the kicker to center back to the goal,
   but it ended up being too inconsistent and led to choppy movement, so it was
   scrapped and a general cone searching function was used instead
 */
void move_and_turn(int speed, int degrees){
        //This indicates we want to turn in place
        if(speed == 0) {
                if(degrees < 0) {
                        //Set left to be half the speed
                        motor(0,-50);
                        motor(1,50);
                }
                //Turns right (clockwise)
                else{
                        motor(0,50);
                        motor(1,-50);
                }
                msleep(18 * degrees);
                motor(0,0);
                motor(1,0);
                return;
        }
        //Determins how long we sleep to turn 1 degree when speed of one wheel is quartered
        //Works perfectly at 50 speed, so we can make a ratio where 50 speed = 50, 100 = 25, etc
        int sleepPerDegree = round(50.0 * (50.0 / speed));

        //Start by moving forward at speed
        motor(0,speed);
        motor(1,speed);
        currentDegrees += degrees;
        //This will prevent the robot from turning more than 360 degrees
        currentDegrees = currentDegrees % 360;

        if(degrees == 0) {
                return;
        }
        //Turns left (counterclockwise)
        int left = 0;
        int right = 0;
        if(degrees < 0) {
                //Set left to be half the speed
                left = round(speed / 4.0);
                right = speed;
        }
        //Turns right (clockwise)
        else{
                //Set right to be half the speed
                left = speed;
                right = round(speed / 4.0);
        }
        motor(0,left);
        motor(1,right);
        msleep(sleepPerDegree * abs(degrees));

        motor(0,speed);
        motor(1,speed);
}

int get_current_degrees(){
        return currentDegrees;
}

/*
   Modulates the servo to kick the ball
 */
void kick(){
        //Stop the motors
        motor(0,0);
        motor(1,0);
        //Enable the servo, and move it to kick the ball, then move it back and disable the servo
        int sleepTime = 500;
        motor(0,100);
        motor(1,100);
        enable_servos();
        set_servo_position(0, 1200);
        msleep(sleepTime);
        motor(0,0);
        motor(1,0);
        disable_servos();
        default_servo_position();
}

/*
   Puts the servo in its "resting" state
 */
void default_servo_position(){
        enable_servos();
        set_servo_position(0, 950);
        msleep(500);
        disable_servos();
}

/*
   Function that will center the bot on a partiular color.
   It does not move forward, it only rotates the bot until
   the object is in the center
 */
void center(int type, int yVal){
        int offset = 0;
        //Start by centering the ball on the x axis
        int* coordinates;
        int x = 0;
        //Update our x position
        coordinates = get_position(type);
        x = coordinates[0];
        int middle = round(get_camera_width() / 2.0);

        //If the ball aprox in the center, exit the loop
        if(x < (middle + 10 + offset) && x > (middle - 10 + offset)) {
                return;
        }
        //If we are on the left side of the robot, turn left to center the ball until it is centered
        if(x < middle + offset) {
                motor(1,30);
                motor(0,-30);
        }
        //If we are on the right side of the robot, turn right to center the ball until it is centered
        else{
                motor(0,30);
                motor(1,-30);
        }
        return;
}

/*
   Modulates the servos to bring down the cage to capture the ball
 */
void grab_ball(){
        motor(0,30);
        motor(1,30);
        enable_servos();
        set_servo_position(0, 570);
        msleep(1000);
        disable_servos();
        motor(0,0);
        motor(1,0);
}
