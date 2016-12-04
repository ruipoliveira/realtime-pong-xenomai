
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "SDL/SDL.h"

#include <sys/types.h>
#include <fcntl.h>

#include <sys/mman.h> // For mlockall
#include <native/task.h>
#include <native/timer.h>
#include <native/pipe.h>

#include  <rtdk.h> // Provides rt_print functions

#define PIPE_MINOR 0

#define TASK_MODE 0   // No flags
#define TASK_STKSZ 0  // Default stack size

#define TASK_CINEMATICA_PRIO 50  // RT priority [0..99]
#define TASK_CINEMATICA_PERIOD_NS 1000000000 // Task period, in ns

#define TASK_B_PRIO 90  // priority
#define TASK_B_PERIOD_NS  45000000 // Task period, in ns

#define TASK_LOAD_NS      10000000 // Task execution time, in ns (same to all tasks)

RT_TASK task_cinematica_desc; // Task decriptor
RT_TASK task_b_desc; // Task decriptor

#define PIPE_SIZE 255
RT_PIPE mypipe;


typedef struct ball_s {

	int x, y; /* position on the screen */
	int w,h; // ball width and height
	int dx, dy; /* movement vector */
} ball_t;

typedef struct paddle {

	int x,y;
	int w,h;
} paddle_t;

//asset instances
static ball_t ball;
static paddle_t paddle[2];
int score[] = {0,0};

//surfaces
static SDL_Surface *screen;
static SDL_Surface *numbermap;
static SDL_Surface *title;
static SDL_Surface *end;


void catch_signal(int sig) {}

void wait_for_ctrl_c(void) {
	signal(SIGTERM, catch_signal); //catch_signal is called if SIGTERM received
	signal(SIGINT, catch_signal);  //catch_signal is called if SIGINT received

	// Wait for CTRL+C or sigterm
	pause();

	// Will terminate
	rt_printf("Terminating ...\n");
}

/*
* Simulates the computational load of tasks
*/ 
void simulate_load(RTIME load_ns) {
	RTIME ti, tf;

	ti=rt_timer_read(); // Get initial time
	tf=ti+load_ns;      // Compute end time
	while(rt_timer_read() < tf); // Busy wait

	return;
}





//inisilise starting position and sizes of game elemements
static void init_ball() {
	
	ball.x = screen->w / 2;
	ball.y = screen->h / 2;
	ball.w = 10;
	ball.h = 10;
	ball.dy = 1;
	ball.dx = 1;
	
	paddle[0].x = 20;
	paddle[0].y = screen->h / 2 - 50 ;
	paddle[0].w = 10;
	paddle[0].h = 50;

	paddle[1].x = screen->w - 20 - 10;
	paddle[1].y = screen->h / 2 - 50;
	paddle[1].w = 10;
	paddle[1].h = 50;
}

int check_score() {
	
	int i;

	//loop through player scores
	for(i = 0; i < 2; i++) {
	
		//check if score is @ the score win limit
		if (score[i] == 10 ) {
		
			//reset scores
			score[0] = 0;
			score[1] = 0;
			
			//return 1 if player 1 score @ limit
			if (i == 0) {

				return 1;	

			//return 2 if player 2 score is @ limit
			} else {
				
				return 2;
			}
		}
	}
	
	//return 0 if no one has reached a score of 10 yet
	return 0;
}

//if return value is 1 collision occured. if return is 0, no collision.
int check_collision(ball_t a, paddle_t b) {

	int left_a, left_b;
	int right_a, right_b;
	int top_a, top_b;
	int bottom_a, bottom_b;

	left_a = a.x;
	right_a = a.x + a.w;
	top_a = a.y;
	bottom_a = a.y + a.h;

	left_b = b.x;
	right_b = b.x + b.w;
	top_b = b.y;
	bottom_b = b.y + b.h;
	

	if (left_a > right_b) {
		return 0;
	}

	if (right_a < left_b) {
		return 0;
	}

	if (top_a > bottom_b) {
		return 0;
	}

	if (bottom_a < top_b) {
		return 0;
	}

	return 1;
}

/* This routine moves each ball by its motion vector. */
static void move_ball() {
	
	/* Move the ball by its motion vector. */
	ball.x += ball.dx;
	ball.y += ball.dy;
	

	
	/* Turn the ball around if it hits the edge of the screen. */
	/*
	
	if (ball.x < 0) {
		
		score[1] += 1;
		init_ball();
	}

	if (ball.x > screen->w - 10) { 
		
		score[0] += 1;
		init_ball();
	}

	if (ball.y < 0 || ball.y > screen->h - 10) {
		
		ball.dy = -ball.dy;
	}

	//check for collision with the paddle
	int i;


	for (i = 0; i < 2; i++) {
		
		int c = check_collision(ball, paddle[i]); 

		//collision detected	
		if (c == 1) {
			
			//ball moving left
			if (ball.dx < 0) {
					
				ball.dx -= 1;

			//ball moving right
			} else {
					
				ball.dx += 1;
			}
			
			//change ball direction
			ball.dx = -ball.dx;
			
			//change ball angle based on where on the paddle it hit
			int hit_pos = (paddle[i].y + paddle[i].h) - ball.y;

			if (hit_pos >= 0 && hit_pos < 7) {
				ball.dy = 4;
			}

			if (hit_pos >= 7 && hit_pos < 14) {
				ball.dy = 3;
			}
			
			if (hit_pos >= 14 && hit_pos < 21) {
				ball.dy = 2;
			}

			if (hit_pos >= 21 && hit_pos < 28) {
				ball.dy = 1;
			}

			if (hit_pos >= 28 && hit_pos < 32) {
				ball.dy = 0;
			}

			if (hit_pos >= 32 && hit_pos < 39) {
				ball.dy = -1;
			}

			if (hit_pos >= 39 && hit_pos < 46) {
				ball.dy = -2;
			}

			if (hit_pos >= 46 && hit_pos < 53) {
				ball.dy = -3;
			}

			if (hit_pos >= 53 && hit_pos <= 60) {
				ball.dy = -4;
			}

			//ball moving right
			if (ball.dx > 0) {

				//teleport ball to avoid mutli collision glitch
				if (ball.x < 30) {
				
					ball.x = 30;
				}
				
			//ball moving left
			} else {
				
				//teleport ball to avoid mutli collision glitch
				if (ball.x > 600) {
				
					ball.x = 600;
				}
			}
		}
	}
	*/
}

static void move_paddle_ai() {

	int center = paddle[0].y + 25;
	int screen_center = screen->h / 2 - 25;
	int ball_speed = ball.dy;

	if (ball_speed < 0) {
	
		ball_speed = -ball_speed;
	}

	//ball moving right
	if (ball.dx > 0) {
		
		//return to center position
		if (center < screen_center) {
			
			paddle[0].y += ball_speed;
		
		} else {
		
			paddle[0].y -= ball_speed;	
		}

	//ball moving left
	} else {
	
		//ball moving down
		if (ball.dy > 0) { 
			
			if (ball.y > center) { 
				
				paddle[0].y += ball_speed;

			} else { 
			
				paddle[0].y -= ball_speed;
			}
		}
		
		//ball moving up
		if (ball.dy < 0) {
		
			if (ball.y < center) { 
				
				paddle[0].y -= ball_speed;
			
			} else {
			
				paddle[0].y += ball_speed;
			}
		}

		//ball moving stright across
		if (ball.dy == 0) {
			
			if (ball.y < center) { 
			
				paddle[0].y -= 5;

			} else {
			
				paddle[0].y += 5;
			}
		}	 		
	}
}

static void move_paddle(int d) {
	// if the down arrow is pressed move paddle down
	if (d == 0) {
		if(paddle[1].y >= screen->h - paddle[1].h) {
			paddle[1].y = screen->h - paddle[1].h;
		} else { 
			paddle[1].y += 5;
		}
	}
	
	// if the up arrow is pressed move paddle up
	if (d == 1) {
		if(paddle[1].y <= 0) {
			paddle[1].y = 0;
		} else {
			paddle[1].y -= 5;
		}
	}
}

static void draw_game_over(int p) { 

	SDL_Rect p1;
	SDL_Rect p2;
	SDL_Rect cpu;
	SDL_Rect dest;

	p1.x = 0;
	p1.y = 0;
	p1.w = end->w;
	p1.h = 75;

	p2.x = 0;
	p2.y = 75;
	p2.w = end->w;
	p2.h = 75;
	
	cpu.x = 0;
	cpu.y = 150;
	cpu.w = end->w;
	cpu.h = 75;

	dest.x = (screen->w / 2) - (end->w / 2);
	dest.y = (screen->h / 2) - (75 / 2);
	dest.w = end->w;
	dest.h = 75;
	
	switch (p) {
	
		case 1:			
			SDL_BlitSurface(end, &p1, screen, &dest);
			break;
		case 2:
			SDL_BlitSurface(end, &p2, screen, &dest);
			break;
		default:
			SDL_BlitSurface(end, &cpu, screen, &dest);
	}
}

static void draw_menu() {

	SDL_Rect src;
	SDL_Rect dest;

	src.x = 0;
	src.y = 0;
	src.w = title->w;
	src.h = title->h;

	dest.x = (screen->w / 2) - (title->w / 2);
	dest.y = (screen->h / 2) - (title->h / 2);
	dest.w = title->w;
	dest.h = title->h;
	
	SDL_BlitSurface(title, &src, screen, &dest);
}

static void draw_background() {
 
	SDL_Rect src;
	
	//draw bg with net
	src.x = 0;
	src.y = 0;
	src.w = screen->w;
	src.h = screen->h;

	//draw the backgorund
	int r = SDL_FillRect(screen,&src,0);
	
	if (r !=0){
		
		printf("fill rectangle faliled in func draw_background()");
	}
}

static void draw_net() {

	SDL_Rect net;
	
	net.x = screen->w / 2;
	net.y = 20;
	net.w = 5;
	net.h = 15;

	//draw the net
	int i,r;

	for(i = 0; i < 15; i++) {
		
		r = SDL_FillRect(screen,&net,255);
	
		if (r != 0) { 
		
			printf("fill rectangle faliled in func draw_background()");
		}

		net.y = net.y + 30;
	}

}

static void draw_ball() {
	
	SDL_Rect src;

	src.x = ball.x;
	src.y = ball.y;
	src.w = ball.w;
	src.h = ball.h;

	int r = SDL_FillRect(screen,&src,255);

	if (r !=0){
	
		printf("fill rectangle faliled in func drawball()");
	}
}

static void draw_paddle() {

	SDL_Rect src;
	int i;

	for (i = 0; i < 2; i++) {
	
		src.x = paddle[i].x;
		src.y = paddle[i].y;
		src.w = paddle[i].w;
		src.h = paddle[i].h;
		
		int r = SDL_FillRect(screen,&src,255);
		
		if (r !=0){
		
			printf("fill rectangle faliled in func draw_paddle()");
		}
	}
}

static void draw_player_0_score() {
	
	SDL_Rect src;
	SDL_Rect dest;

	src.x = 0;
	src.y = 0;
	src.w = 64;
	src.h = 64;

	dest.x = (screen->w / 2) - src.w - 12; //12 is just padding spacing
	dest.y = 0;
	dest.w = 64;
	dest.h = 64;

	if (score[0] > 0 && score[0] < 10) {
		
		src.x += src.w * score[0];
	}
	
	SDL_BlitSurface(numbermap, &src, screen, &dest);
}

static void draw_player_1_score() {
	
	SDL_Rect src;
	SDL_Rect dest;

	src.x = 0;
	src.y = 0;
	src.w = 64;
	src.h = 64;

	dest.x = (screen->w / 2) + 12;
	dest.y = 0;
	dest.w = 64;
	dest.h = 64;
	
	if (score[1] > 0 && score[1] < 10) {
		
		src.x += src.w * score[1];
	}

	SDL_BlitSurface(numbermap, &src, screen, &dest);
}



#define NTASKS 2

#define QUEUE_SIZE 255
#define MAX_MESSAGE_LENGTH 40

RT_TASK task_struct[NTASKS];

#define PIPE_SIZE 255



/*
* Task body implementation
*/
void task_cinematica_code(void *task_period_ns) {





/*	
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	int *task_period;

	RTIME to=0,ta=0;
	unsigned long overruns;
	int err;

	//Get task information 
	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	rt_printf("%s init\n", curtaskinfo.name);
	task_period=(int *)task_period_ns;
*/



	//Set task as periodic 
	//err=rt_task_set_periodic(NULL, TM_NOW, *task_period);
	

	/*
	for(;;) {
		printf("x: %d - dx: %d \n", ball.x, ball.dx);
		printf("y: %d - dy: %d \n", ball.y, ball.dy);


		err=rt_task_wait_period(&overruns);
		ta=rt_timer_read();
		if(err) {
			rt_printf("%s overrun!!!\n", curtaskinfo.name);
			break;
		}
		rt_printf("%s activation\n", ball.x);
		
		if(to!=0) 
			rt_printf("Measured period (ns)= %lu\n",ta-to);
		to=ta;

		// Task "load"
		simulate_load(TASK_LOAD_NS);
		
	}
	*/

	return;
	 

}



void init_xenomai() {
	/* Avoids memory swapping for this program */
	mlockall(MCL_CURRENT|MCL_FUTURE);

	/* Perform auto-init of rt_print buffers if the task doesn't do so */
	rt_print_auto_init(1);
}



void taskOne(void *arg)
{
    int retval;
    char msgBuf[MAX_MESSAGE_LENGTH];
    char message[] = "Message from taskOne";

    //task message blocks
    RT_TASK_MCB mymcb, talk_reply;

    // this is to debug which task started first
    rt_printf("Entered Task one\n");
	



	char str[15];
	sprintf(str, "%d", ball.x);



    mymcb.data = str;
    mymcb.size = sizeof(str);

    talk_reply.size = 0;
    talk_reply.data = NULL;

    retval = rt_task_send(&task_struct[1], &mymcb, &talk_reply, TM_NONBLOCK);
    if (retval == -EWOULDBLOCK ) {
       rt_printf("Would block error: %d\n", retval);
    } 
    else if(retval == -ETIMEDOUT){
	rt_printf("Timedout error: %d\n", retval);
    }
    else if(retval == -ENOBUFS){
	rt_printf("unblocked called error: %d\n", retval);
    }
    else if(retval == -EIDRM){
	rt_printf("sleep error: %d\n", retval);
    }
    else if(retval == -ESRCH){
    	rt_printf("Buffer error: %d\n", retval);
    }  
    else {
       rt_printf("taskOne sent message to mailbox\n");
    }



}

void taskTwo(void *arg)
{
    int retval;
    char msgBuf[MAX_MESSAGE_LENGTH];
    char message[] = "Message from taskTwo";
    RT_TASK_MCB mymcb, listen_reply;
  

    mymcb.data = (caddr_t)msgBuf;
    mymcb.size= sizeof(msgBuf);\

    // this is for debug
    rt_printf("Entered Task two\n");

    /* receive message */
    retval = rt_task_receive(&mymcb,TM_INFINITE); 	

    //error checks
    if (retval == -EWOULDBLOCK ) {
       rt_printf("Would block error: %d\n", retval);
    } 
    else if(retval == -ETIMEDOUT){
	rt_printf("Timedout error: %d\n", retval);
    }
    else if(retval == -ENOBUFS){
	rt_printf("unblocked called error: %d\n", retval);
    }
    else if(retval == -EIDRM){
	rt_printf("sleep error: %d\n", retval);
    }
    else if(retval == -ESRCH){
    	rt_printf("Buffer error: %d\n", retval);
    }    
    else if (retval < 0 ) {
          rt_printf("Receiving error\n");
    } else {
        rt_printf("taskTwo received message: %s\n",mymcb.data);
        rt_printf("with length %d\n",retval);
    }

    listen_reply.size = 0;
    listen_reply.data = NULL;
    rt_task_reply(retval, &listen_reply);

 	

}



void startup(){
	int err; 
	int task_cinematica_period_ns=TASK_CINEMATICA_PERIOD_NS; 

  	rt_pipe_create (&mypipe, "mypipe", 0, PIPE_SIZE);
	
	err=rt_task_create(&task_cinematica_desc, "Task cinematica", TASK_STKSZ, TASK_CINEMATICA_PRIO, TASK_MODE);
	
	if(err) {
		rt_printf("Error creating task cinematica (error code = %d)\n",err);
	} else 
		rt_printf("Task cinematica created successfully\n");

	rt_task_start(&task_cinematica_desc, &task_cinematica_code, (void *)&task_cinematica_period_ns);




	int i;
  char  str[10] ;

  void (*task_func[NTASKS]) (void *arg);
  task_func[0]=taskOne;
  task_func[1]=taskTwo;

  rt_pipe_create (&mypipe, "mypipe", 0, PIPE_SIZE);
	
  rt_timer_set_mode(0); // set timer to tick in nanoseconds and not in jiffies
  for(i=0; i < NTASKS; i++) {
    rt_printf("create task  : %d\n",i);
    sprintf(str,"task%d",i);
    rt_task_create(&task_struct[i], str, 0, 50, 0);
  }

  for(i=1; i > -1 ; i--) {
    rt_printf("start task  : %d\n",i);
    sprintf(str,"task%d",i);
    rt_task_start(&task_struct[i], task_func[i], &i);
  }	  






}



int main() {

	init_xenomai(); 

	startup(); 
	

	SDL_Surface *temp;

	/* Initialize SDL’s video system and check for errors */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	/* Make sure SDL_Quit gets called when the program exits! */
	atexit(SDL_Quit);
	
	/* Attempt to set a 640x480 8 bit color video mode */
	screen = SDL_SetVideoMode(640, 480, 8,SDL_DOUBLEBUF);
	if (screen == NULL) {
		printf("Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	}

	//load the numbermap image strip of 10 number 64px * 64px
	temp = SDL_LoadBMP("numbermap.bmp");
	if (temp == NULL) {
		printf("Unable to load numbermap.bmp.\n");
		return 1;
	}

	/* Set the numbermaps colorkey. */
	Uint32 colorkey = SDL_MapRGB(temp->format, 255, 0, 255);
	SDL_SetColorKey(temp, SDL_SRCCOLORKEY, colorkey);
	
	//convert the numbermaps surface to the same type as the screen
	numbermap = SDL_DisplayFormat(temp);
	
	if (numbermap == NULL) {
		printf("Unable to convert bitmap.\n");
		return 1;
	}

	SDL_FreeSurface(temp);
	
	//load the numbermap image strip of 10 number 64px * 64px
	temp = SDL_LoadBMP("title.bmp");
	if (temp == NULL) {
		printf("Unable to load numbermap.bmp.\n");
		return 1;
	}

	/* Set the numbermaps colorkey. */
	SDL_SetColorKey(temp, SDL_SRCCOLORKEY, colorkey);
	
	//convert the numbermaps surface to the same type as the screen
	title = SDL_DisplayFormat(temp);
	if (numbermap == NULL) {
		printf("Unable to convert bitmap.\n");
		return 1;
	}

	SDL_FreeSurface(temp);

	//load the numbermap image strip of 10 number 64px * 64px
	temp = SDL_LoadBMP("gameover.bmp");
	if (temp == NULL) {
		printf("Unable to load gameover.bmp.\n");
		return 1;
	}

	//convert the end surface to the same type as the screen
	end = SDL_DisplayFormat(temp);
	if (end == NULL) {
		printf("Unable to convert bitmap.\n");
		return 1;
	}

	SDL_FreeSurface(temp);

	/* Initialize the ball position data. */
	init_ball();

	int quit = 0;
	int state = 0;
	Uint8 *keystate = 0;
	Uint32 next_game_tick = SDL_GetTicks();
	int sleep = 0;
	int r = 0;

	/* Animate */
	while (quit == 0) {
		
		/* Update SDL's internal input state information. */
		SDL_PumpEvents();

		/* Grab a snapshot of the keyboard. */
		keystate = SDL_GetKeyState(NULL);
		
		/* Respond to input. */
		if (keystate[SDLK_ESCAPE]) {
			quit = 1;
		}
		
		if (keystate[SDLK_DOWN]) {
			move_paddle(0);
		}

		if (keystate[SDLK_UP]) {
			move_paddle(1);
		}
		
		//draw the background
		draw_background();

		//display main menu
		if (state == 0 ) {
		
			if (keystate[SDLK_SPACE]) {
				state = 1;
			}
		
			//draw menu 
			draw_menu();
		
		//display gameover
		} else if (state == 2) {
		
			if (keystate[SDLK_SPACE]) {
				state = 0;
				//delay for a little bit so the space bar press dosnt get triggered twice
				//while the main menu is showing
            			SDL_Delay(500);
			}

			if (r == 1) {

				//if player 1 is AI if player 1 was human display the return value of r not 3
				draw_game_over(3);

			} else {
			
				//display gameover
				draw_game_over(r);
			}
				
		//display the game
		} else if (state == 1){
			
			//check score
			r = check_score();
			
			if (r == 1) {
				
				state = 2;	

			} else if (r == 2){
			
				state = 2;	
			}

			//paddle ai movement
			move_paddle_ai();

			/* Move the balls for the next frame. */
			//move_ball();
			
			//draw net
			draw_net();

			//draw paddles
			draw_paddle();
			
			/* Put the ball on the screen. */
			draw_ball();
	
			//draw the score
			draw_player_0_score();
	
			//draw the score
			draw_player_1_score();
		}

		/* Ask SDL to update the entire screen. */
		SDL_Flip(screen);

		next_game_tick += 1000 / 60;
		sleep = next_game_tick - SDL_GetTicks();
	
		if( sleep >= 0 ) {

            		SDL_Delay(sleep);
        	}
	}
	
	return 0;
}
