#include <kipr/botball.h>
//Detects the ball
enum {BALL, GOALIE, GOAL, KICKER};
int prevGap = 0;
int countKick = 0;
int closeCount = 0;
int closeCountBall = 0;
int count = 0;
int* get_position(int color);
int** get_goal_position();
int in_goal_range();

//Uses get_position() along with some extra processing to get the position of the goalie
int* get_goalie_position(){
        int* pos = get_position(GOALIE);
        //If the goalie is too small then its noise and we throw it out
        if(pos[0] < 15) {
                pos[0] = -1;
                pos[1] = -1;
        }
        return pos;
}

//Resets all of the globals
void reset_detect(){
        prevGap = 0;
        closeCount = 0;
        closeCountBall = 0;
        count = 0;
        countKick = 0;
}

/*
   Used to find the biggest gap between the goalie and the cones.
   When both cones and the goalie are detected, this function
   determines the biggest gap and returns the coordinates of the
   center of that gap. This was used to help the kicker move towards
   the spot where it had the best chance of making a goal.
 */
int find_gap(){
        int* goaliePos = get_goalie_position();
        //If there is no goalie detected, return the gap between two cones
        if(goaliePos[0] < 0) {
                //If we still see the two cones, return the gap between them
                if(in_goal_range() >= 2) {
                        int** goalPos = get_goal_position();
                        return middle(goalPos[0][0], goalPos[1][0]);
                }
                //If we dont, return -1
                return -1;
        }
        double mult = 0.4;
        int goaliePosX = goaliePos[0];
        int coneCount = in_goal_range();
        int** cones = get_goal_position();
        int x1 = cones[0][0];
        int x2 = cones[1][0];

        if(coneCount == 0) {
                return -1;
        }
        if(coneCount == 1) {
                int gap = abs(goaliePosX - x1);
                //if(gap < (prevGap * mult))
                // return -2;
                prevGap = gap;
                return middle(goaliePosX,x1);
        }
        else{
                int gap1 = abs(goaliePosX - x1);
                int gap2 = abs(goaliePosX - x2);
                if(gap1 > gap2) {
                        //if(gap1 < (prevGap * mult))
                        //return -2;
                        prevGap = gap1;
                        return middle(goaliePosX,x1);
                }

                else{
                        //if(gap2 < (prevGap * mult))
                        //return -2;
                        prevGap = gap2;
                        return middle(goaliePosX,x2);
                }

        }
}

int middle(x1,x2){
        return round((x1 + x2) * 0.5);
}

int cone_too_big(){
        int objects = get_object_count(GOAL);
        if(objects >= 1) {
                int width1 = get_object_bbox_width(GOAL,0);
                int height1 = get_object_bbox_height(GOAL,0);
                //Too close to the goal posts, back up
                if(width1 > 30)
                        return 1;
        }
        return 0;
}

/*
   This function returns 0, 1, or 2 depending on how many cones it saw.
   This function was very important in determining where the goal was.
   The function used the width and y position of the objects to determine
   whether the object was a cone or just a different object or noise.
 */
int in_goal_range(){
        int heightMin = 2;
        int widthMin = 3;
        int objects = get_object_count(GOAL);
        //We see one goal post, look for the other
        int width1 = 0;
        int height1 = 0;
        int y1 = 0;
        int width2 = 0;
        int height2 = 0;
        int y2 = 0;
        int count = 0;
        int x1 = 0;
        int x2 = 0;

        //If theres only 1 object, return that if its a verified goal post
        if(objects == 1) {
                width1 = get_object_bbox_width(GOAL,0);
                height1 = get_object_bbox_height(GOAL,0);
                y1 = get_object_center_y(GOAL,0);

                if(width1 > widthMin && height1 > heightMin && y1 > (get_camera_height() * 0.40))
                        return 1;
        }

        else if(objects >= 2) {
                width1 = get_object_bbox_width(GOAL,0);
                height1 = get_object_bbox_height(GOAL,0);
                y1 = get_object_center_y(GOAL,0);
                width2 = get_object_bbox_width(GOAL,1);
                height2 = get_object_bbox_height(GOAL,1);
                y2 = get_object_center_y(GOAL,1);
                x1 = get_object_center_x(GOAL,0);
                x2 = get_object_center_x(GOAL,1);

                //The two blobs are actually just one cone
                if(x1 > x2 + 5 && x1 < x2 - 5)
                        return 1;

                if(width1 > widthMin && height1 > heightMin && y1 > (get_camera_height() * 0.40))
                        count++;
                if(width2 > widthMin && height2 > heightMin && y2 > (get_camera_height() * 0.40))
                        count++;
                return count;
        }

        return 0;
}
/*
   It made sure that the ball’s y position was close to the bottom and that the ball was centered.
   This function would trigger the servo to bring the cage down around the ball to capture it.
 */
int have_ball(){
        int width = get_object_bbox_width(BALL,0);
        int y = get_object_center_y(BALL,0);
        int x = get_object_center_x(BALL,0);
        int range = 40;
        int middle = round(get_camera_width() * 0.5);
        //If the ball takes up more than 75% of the camera width (height doesnt matter), then we have the ball
        //if(width > (get_camera_width() * 0.70))
        // return 1;
        //Fallback - If the height of the ball is at the bottom
        if(y > 107) {
                printf("x: %d middle: %d\n",x,round(get_camera_width() / 2.0));
                closeCountBall++;
                if(x < (middle + range) && x > (middle - range)) {
                        if(closeCountBall > 1) {
                                closeCountBall = 0;
                                return 1;
                        }

                }
        }
        return 0;
}

//Modified version of get_position that returns a 2-d array of the position of each cone
int** get_goal_position(){
        //Find both goal posts and return both points
        int** coordinates = malloc(2 * sizeof(int*));
        coordinates[0] = malloc(2 * sizeof(int));
        coordinates[1] = malloc(2 * sizeof(int));
        coordinates[0][0] = -1;
        coordinates[0][1] = -1;
        coordinates[1][0] = -1;
        coordinates[1][1] = -1;


        int x1 = -1;
        int y1 = -1;
        int width1 = 0;
        int x2 = -1;
        int y2 = -1;
        int width2 = 0;
        int goals = in_goal_range();
        if(goals == 1) {
                x1 = get_object_center_x(GOAL,0);
                y1 = get_object_center_y(GOAL,0);
                coordinates[0][0] = x1;
                coordinates[0][1] = y1;
        }
        if(goals == 2) {
                x1 = get_object_center_x(GOAL,0);
                y1 = get_object_center_y(GOAL,0);
                width1 = get_object_bbox_width(GOAL,1);
                x2 = get_object_center_x(GOAL,1);
                y2 = get_object_center_y(GOAL,1);
                width2 = get_object_bbox_width(GOAL,1);
                //If x1 is bigger, its on the right, so x2 is first
                //Add in half the width to make sure the range doesnt include any of the cone in it
                if(x1 > x2) {
                        coordinates[0][0] = x2 + round(width2 / 2.0);
                        coordinates[0][1] = y2;
                        coordinates[1][0] = x1 - round(width1 / 2.0);
                        coordinates[1][1] = y1;
                }
                else{
                        coordinates[0][0] = x1 + round(width1 / 2.0);
                        coordinates[0][1] = y1;
                        coordinates[1][0] = x2 - round(width2 / 2.0);
                        coordinates[1][1] = y2;
                }
        }
        return coordinates;
}

int* get_kicker_position(){
        return get_position(KICKER);
}

int* get_ball_position(){
        return get_position(BALL);
}

/*
   Used to determine if the kicker should kick the ball or wait until it was closer.
   There were many different iterations of this function, ranging from kicking if the cones were
   too big or kicking if it detected the cones were going to go out of frame on the camera, or if
   the goalie and one of the cones were going out of frame. In the end, we only got rid of the kicking
   due to both cones going out of frame, since we can just keep track of one goalie and one cone to
   determine where to shoot the ball.
 */
int should_kick(){
        int width = 18;

        int ** coordinates = get_goal_position();
        int* goaliePos = get_goalie_position();
        int x1 = coordinates[0][0];
        int x2 = coordinates[1][0];
        int y1 = coordinates[0][1];
        int y2 = coordinates[1][1];

        int width1 = get_object_bbox_width(2,0);
        int width2 = get_object_bbox_width(2,1);

        //Check if width is big enough to shoot
        //The camera can usually pick up the width more often than the height, which can be seperated
        if(width1 > width || width2 > width) {
                if(closeCount > 2) {
                        printf("Kicking due to cones being too close to camera\n");
                        closeCount = 0;
                        return 1;
                }
                closeCount++;
        }


        //If the cones are on the outer edges of the camera view, shoot the ball
        //This is an edge case that can be caused by the failure of the centering function
        if(x1 < (get_camera_width() / 16.0) && goaliePos[0] > (get_camera_width() * (15.0 / 16.0))) {
                printf("Kicking due to the cone and golie about to go out of camera\n");
                return 1;
        }
        if(goaliePos[0] < (get_camera_width() / 16.0) && x1 > (get_camera_width() * (15.0 / 16.0))) {
                printf("Kicking due to the cone and golie about to go out of camera\n");
                return 1;
        }

        return 0;
}
/*
   When given a color would return the coordinates if the function determined
   the object was real and not just noise. To determine this, the function used the
   width and height of the object as well as the y position to determine if it
   was an object or noise that should be discarded.
   This function is used to determine the position of the ball and goalie,
   and a modified version was used to return the positions of the two cones used as a goal.
 */
int* get_position(int color){
        //Loop until the camera gets a proper new frame
        int* coordinates = malloc(2 * sizeof(int));
        coordinates[0] = -1;
        coordinates[1] = -1;


        int x = -1;
        int y = -1;
        int width = 0;
        int height = 0;

        if(get_object_count(color) > 0) {
                x = get_object_center_x(color,0);
                y = get_object_center_y(color,0);
                width = get_object_bbox_width(color,0);
                height = get_object_bbox_height(color,0);
        }


        //If the object is too small, or we have 0 detected objects then report -1,-1
        if(get_object_count(color) <= 0 || width < 4 || height < 4) {
        }
        else{
                coordinates[0] = x;
                coordinates[1] = y;
        }
        return coordinates;
}
