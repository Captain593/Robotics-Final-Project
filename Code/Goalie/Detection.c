#include <kipr/botball.h>
//Detects the ball
    enum{BALL, GOALIE, GOAL, KICKER};

int* get_position(int color);

int in_ball_range(){
	while(camera_update() != 1)
    	    continue;
    int objects = get_object_count(0);
    //We see one goal post, look for the other
    int width1 = 0;
    int height1 = 0;
    int y1 = 0;
    int width2 = 0;
    int height2 = 0;
    int y2 = 0;
    int count = 0;
    
    //If theres only 1 object, return that if its a verified ball
    if(objects > 1){
    width1 = get_object_bbox_width(0,0);
    height1 = get_object_bbox_height(0,0);
    y1 = get_object_center_y(0,0);
    printf("width: %d height: %d \n", width1, height1);
	//sees if ball is close
    if(width1 > 40 && height1 > 40 && y1 > (get_camera_height() * 0.40))
        
    {
        printf("Hi ball!\n");
        return 1;
    
    }}
    
    return 0;
}

int* get_position(int color){
    //Loop until the camera gets a proper new frame
while(camera_update() != 1)
        continue;
    int* coordinates = malloc(2 * sizeof(int));
    coordinates[0] = -1;
    coordinates[1] = -1;
    
    
    int x = -1;
    int y = -1;
    int width = 0;
    int height = 0;
    //sees if object is seen
    if(get_object_count(color) > 0){
		x = get_object_center_x(color,0);
		y = get_object_center_y(color,0);
        width = get_object_bbox_width(color,0);
    	height = get_object_bbox_height(color,0);
    }
    
    
    //If the object is too small, or we have 0 detected objects, do nothing
    if(get_object_count(color) <= 0 || width < 5 || height < 5){

    }
	//else get the coordinates
    else{
        coordinates[0] = x;
        coordinates[1] = y;
    }
return coordinates;
}

