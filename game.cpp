
#include "Framework.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

//initializations
int _width = 800; //these go into PreInit function and also get overwritten if proper command-line is used
int _height = 600;

struct Object {
	float x; //xcoord
	float y; //ycoord
	float w; //width
	float h; //height
	float xspeed;
	float yspeed;
};

struct Block : public Object {
	int number = 0;
	bool bActive = true;
	bool expl = false;
};

//functions
void drawBlocks(const vector<Block>&, Sprite*, Sprite*);
void blockHit(int&, int&, Block&, vector<Block>&, unordered_map<int, vector<int>>);
void checkCmdLnInput(int, char* [], int&, int&);
void calcNewBallXYSpeed(float& xspeed, float& yspeed, float rel, const float& ball_speed); //this function calculates x and y speeds for ball, based on constant ball flight speed

/* Test Framework realization */
class MyFramework : public Framework {
private:

	//these are used for size calculations
	float ww = _width; //Window Width
	float wh = _height;//WIndow Height

	int start = 0;
	int score = 0;
	int combo = 0;
	unsigned int t = 0;
	int delta = 0;

	float ball_speed; // 550 pixels on 800 wide screen per 1000 ms - 0.55				550/800 = 0.6875 relative to ww
	float paddle_speed; // 600 pixels on 800 wide screen per 1000 ms - 0.6				600/800 = 0.75 relative wh
	bool slowed = false;
	bool wall_buff = false;
	unsigned int slow_dur = 3000;
	unsigned int temp_slow_debuff = 5000; //stopwatch variable, random number > slow_dur on start
	unsigned int temp_slow = 0;	//timespamp variables, take TickCount to create new timing for ability to start falling from top
	unsigned int temp_wall = 0;	//timestamp variable

	Sprite* block, * orngblock, * ball, * paddle, * cursor, * slowed_paddle, * wall, * red_ability, * green_ability;

	unordered_map<int, vector<int>> orngblocks; //orange blocks numbers and vectors with numbers of their adjacent blocks
	vector<Block> blocks;

	Object pad;
	Object bal;
	Object curs;
	Object bloc;
	Object wal;
	Object abil_slow;
	Object abil_wall;

public:

	virtual void PreInit(int& width, int& height, bool& fullscreen)
	{
		width = _width;
		height = _height;
		fullscreen = false;
	}

	virtual bool Init() {

		t = getTickCount();

		//assigning game objects values
		pad.w = 0.16 * ww; //default 485, scaled to 16% of window width in game
		pad.h = 0.046 * wh; //default 128, scaled to 4.6% of window height in game
		pad.x = ww / 2 - 50;
		pad.y = 0.92 * wh;
		pad.xspeed = 0;
		paddle_speed = 0.75 * ww / 1000;

		bal.w = 0.04 * ww; //4% of window width
		bal.h = 0.0533 * wh;
		bal.x = (ww - bal.w) / 2;
		bal.y = pad.y - bal.h;
		bal.xspeed = 0;
		bal.yspeed = 0;
		ball_speed = 0.6875 * ww / 1000;

		curs.w = 0.0375 * ww;
		curs.h = 0.05 * wh;

		bloc.w = 0.1 * ww;
		bloc.h = 0.045 * wh;

		abil_wall.h = 0.045 * ww;
		abil_wall.w = 0.06 * wh;
		abil_wall.yspeed = 0.2 * ww / 1000;
		abil_wall.y = wh + abil_wall.h;
		int random_pos = rand() % 16;
		abil_wall.x = 0.05 * ww + abil_wall.w * random_pos;
		int random_time = rand() % 5;
		temp_wall = 5000 + 2500 * random_time + t;

		abil_slow.h = 0.045 * ww;
		abil_slow.w = 0.06 * wh;
		abil_slow.yspeed = 0.15 * ww / 1000;
		abil_slow.y = wh + abil_slow.h;
		random_pos = rand() % 16;
		abil_slow.x = 0.05 * ww + abil_slow.w * random_pos;
		random_time = rand() % 5;
		temp_slow = 5000 + 2500 * random_time + t;

		wal.x = 0;
		wal.y = 0.97 * wh;
		wal.w = ww;
		wal.h = 0.037 * wh;

		start = 0;
		score = 0;
		combo = 0;
		slowed = false;
		wall_buff = false;
		blocks.clear();

		//creating blocks
		orngblocks = { {5, {1, 2, 6, 9, 10} },
					   {12, {7, 8, 11, 15, 16} },
					   {18, {13, 14, 15, 17, 19, 21, 22, 23} },
					   {25, {21, 22, 26, 29, 30}} }; //orange blocks numbers and vectors with numbers of their adjacent blocks
		int counter = 0;//to give blocks numbers
		for (int i = 1; i < 9; ++i) {
			for (int j = 3; j < 7; ++j) {
				++counter;
				Block blocck;
				blocck.x = i * bloc.w;
				blocck.y = j * bloc.h;
				blocck.number = counter;
				if (orngblocks.count(counter)) {
					blocck.expl = true;
				}
				blocks.push_back(blocck);
			}
		}

		//assigning sprites
		const char* block_path = "01-Breakout-Tiles.png";
		const char* ball_path = "63-Breakout-Tiles.png";
		const char* paddle_path = "50-Breakout-Tiles.png";
		const char* slowed_paddle_path = "41-Breakout-Tiles.png";
		const char* wall_path = "wall.png";
		const char* cursor_path = "59-Breakout-Tiles.png";
		const char* orngblock_path = "09-Breakout-Tiles.png";
		const char* green_ability_path = "22-Breakout-Tiles.png";
		const char* red_ability_path = "24-Breakout-Tiles.png";

		block = createSprite(block_path);
		setSpriteSize(block, bloc.w, bloc.h);
		orngblock = createSprite(orngblock_path);
		setSpriteSize(orngblock, bloc.w, bloc.h);
		paddle = createSprite(paddle_path);
		setSpriteSize(paddle, pad.w, pad.h);
		ball = createSprite(ball_path);
		setSpriteSize(ball, bal.w, bal.h);
		cursor = createSprite(cursor_path);
		setSpriteSize(cursor, curs.w, curs.h);
		showCursor(false);
		slowed_paddle = createSprite(slowed_paddle_path);
		setSpriteSize(slowed_paddle, pad.w, pad.h);
		wall = createSprite(wall_path);
		setSpriteSize(wall, wal.w, wal.h);
		green_ability = createSprite(green_ability_path);
		setSpriteSize(green_ability, abil_wall.w, abil_wall.h);
		red_ability = createSprite(red_ability_path);
		setSpriteSize(red_ability, abil_slow.w, abil_slow.h);

		cout << "New game! Click mouse to launch the ball!\n";
		return true;
	}

	virtual void Close() {

	}

	virtual bool Tick() {
		delta = getTickCount() - t;
		t = getTickCount();//timestamp for current Tick();

		//if ((getTickCount() - temp_slow_debuff) < slow_dur && getTickCount() > 3000) { //this approach didn't allow to properly receive slow debuff when game just started getTickCount()<slow_dur
		//	slowed = true;
		//}
		//else {
		//	slowed = false;
		//}

		if (temp_slow_debuff < slow_dur) {
			temp_slow_debuff += delta;
			slowed = true;
		}
		else {
			slowed = false;
		}


		//abilities movement
		abil_wall.y += (delta * abil_wall.yspeed);
		if (abil_wall.y > wh) {
			if (t > temp_wall) {
				int random_time = rand() % 10000;
				temp_wall = (wh + abil_wall.h) / abil_wall.yspeed + random_time + t;
				int random_pos = ww - abil_wall.w;
				abil_wall.x = rand() % random_pos;

				abil_wall.y = -abil_wall.h;
			}
		}
		abil_slow.y += (delta * abil_slow.yspeed);
		if (abil_slow.y > wh) {
			if (t > temp_slow) {
				int random_time = rand() % 10000;
				temp_slow = (wh + abil_slow.h) / abil_slow.yspeed + random_time + t;
				int random_pos = ww - abil_slow.w;
				abil_slow.x = rand() % random_pos;

				abil_slow.y = -abil_slow.h;
			}
		}

		//abilities tests for paddle
		if (!wall_buff) {
			if (((abil_wall.y + abil_wall.h) > pad.y && abil_wall.y < pad.y) && ((abil_wall.x + abil_wall.w) > pad.x && abil_wall.x < (pad.x + pad.w))) {
				//we caught ability
				abil_wall.y = wh;
				wall_buff = true;
			}
		}
		if (((abil_slow.y + abil_slow.h) > pad.y && abil_slow.y < pad.y) && ((abil_slow.x + abil_slow.w) > pad.x && abil_slow.x < (pad.x + pad.w))) {
			//we caught ability
			abil_slow.y = wh;
			//temp_slow_debuff = t;
			temp_slow_debuff = 0;
		}
		
		//paddle movement
		if (!slowed) {
			pad.x += (delta * pad.xspeed);
		}
		else {
			pad.x += (delta * 0.5 * pad.xspeed);
		}
		if (pad.x < 0) {
			pad.x = 0;
		}
		if (pad.x > ww - pad.w) {
			pad.x = ww - pad.w;
		}

		//ball basic movement
		if (start == 0) {
			bal.x = pad.x + pad.w/2 - bal.w/2;
		}

		bal.y += (delta * bal.yspeed);
		if (bal.y < 0) {
			bal.y = 0;
			bal.yspeed = -bal.yspeed;
		}
		if (bal.y > wh) { //balls is out - restart
			cout << "You lost ball!\n";
			this->Init();
			return false;
		}
		bal.x += (delta * bal.xspeed);
		if (bal.x < 0) {
			bal.x = 0;
			bal.xspeed = -bal.xspeed;
		}
		if (bal.x > (ww-bal.w)) {
			bal.x = (ww-bal.w);
			bal.xspeed = -bal.xspeed;
		}

		//ball test for paddle
		if (((bal.y + bal.h) > pad.y  && (bal.y < (pad.y - bal.h/2))) && ((bal.x + bal.w) > pad.x && bal.x < (pad.x + pad.w)) && bal.yspeed >0) {
			int blocks_active = 0;
			for (const Block& blocck : blocks) {
				if (blocck.bActive) {
					++blocks_active;
				}
			}
			if (blocks_active==0) {
				cout << "You win, all blocks were destroyed!\n";
				this->Init();
				return false;
			}

			combo = 0;

			//checking relative position of ball to paddle centre
			float rel = 1.6 * ((bal.x + bal.w / 2) - (pad.x + pad.w / 2)) / (pad.w + bal.w);
			if (rel < 0) {
				rel *= rel;
				rel = -rel;
			}
			else {
				rel *= rel;
			}

			//and giving ball new speeds depending on which side of paddle it landed
			calcNewBallXYSpeed(bal.xspeed, bal.yspeed, rel, ball_speed);

			bal.yspeed = -bal.yspeed;
		}

		//ball test for saving wall
		if (wall_buff) {
			if ((bal.y + bal.h) > wal.y) {
				if (bal.yspeed > 0) {
					bal.yspeed = -bal.yspeed;
					wall_buff = false;
				}
			}
		}

		//ball test for blocks
		for (Block& item : blocks) {
			if (item.bActive) {
				float middle_y = bal.y + bal.h / 2;
				float middle_x = bal.x + bal.w / 2;
				if (bal.yspeed < 0) {
					if ((abs(middle_y - (item.y + bloc.h)) < bal.h / 2) && (middle_x > item.x && middle_x < (item.x + bloc.w))) {
						bal.yspeed = -bal.yspeed;
						blockHit(combo, score, item, blocks, orngblocks);
					}
				}
				else {
					if (abs(middle_y - item.y) < bal.h / 2 && (middle_x > item.x && middle_x < (item.x + bloc.w))) {
						bal.yspeed = -bal.yspeed;
						blockHit(combo, score, item, blocks, orngblocks);
					}
				}

				if (bal.xspeed > 0) {
					if ((abs(middle_x - item.x) < bal.w / 2) && (middle_y > item.y && middle_y < (item.y+bloc.h))) {
						bal.xspeed = -bal.xspeed;
						blockHit(combo, score, item, blocks, orngblocks);
					}
				}
				else {
					if ((abs(middle_x - (item.x+bloc.w)) < bal.w / 2) && (middle_y > item.y && middle_y < (item.y+bloc.h))) {
						bal.xspeed = -bal.xspeed;
						blockHit(combo, score, item, blocks, orngblocks);
					}
				}
			}
		}

		//drawing when all positions and states were checked 
		drawTestBackground();
		drawBlocks(blocks, block, orngblock);
		if (!slowed) {
			drawSprite(paddle, pad.x, pad.y);
		}
		else {
			drawSprite(slowed_paddle, pad.x, pad.y);
		}
		drawSprite(ball, bal.x, bal.y);
		if (start == 0) {//cursor sprite is only present before game is started
			drawSprite(cursor, curs.x, curs.y);	
		}
		if (wall_buff) {
			drawSprite(wall, wal.x, wal.y);
		}

		drawSprite(green_ability, abil_wall.x, abil_wall.y);
		drawSprite(red_ability, abil_slow.x, abil_slow.y);

		return false;
	}

	virtual void onMouseMove(int x, int y, int xrelative, int yrelative) {
		curs.x = x + xrelative - curs.w/2;
		curs.y = y + yrelative - curs.h/2;
	}

	virtual void onMouseButtonClick(FRMouseButton button, bool isReleased) {
		if (start == 0) {
			switch (button) {
				case FRMouseButton::LEFT: {
					bal.xspeed = curs.x - bal.x;
					bal.yspeed = curs.y - bal.y;

					calcNewBallXYSpeed(bal.xspeed, bal.yspeed, 0, ball_speed);

					destroySprite(cursor);
					++start;
					break;
				}
			}
		}
	}

	virtual void onKeyPressed(FRKey k) {
		switch (k) {
			case FRKey::RIGHT: {
				pad.xspeed = paddle_speed;
				break;
			}
			case FRKey::LEFT: {
				pad.xspeed = -paddle_speed;
				break;
			}
		}
	}

	virtual void onKeyReleased(FRKey k) {
		switch (k) {
			case FRKey::RIGHT:
			case FRKey::LEFT:
			{
				pad.xspeed = 0;
				break;
			}
		}
	}
	
	virtual const char* GetTitle() override
	{
		return "Arkanoid";
	}
};

void drawBlocks(const vector<Block>& blocks, Sprite* block, Sprite* orngblock) {
	for (const Block& item : blocks) {
		if (item.bActive) {
			if (!item.expl) {
				drawSprite(block, item.x, item.y);
			}
			else {
				drawSprite(orngblock, item.x, item.y);
			}
		}
	}
}

void blockHit(int& cmb, int& scr, Block& blck, vector<Block>& blocks, unordered_map<int, vector<int>> orngblocks) {
	++cmb;
	if (cmb > 3) {
		scr += 300;
		cout << "Score: " << scr << "(x3 hit)" << "\n";
	}
	else {
		scr += 100;
		cout << "Score: " << scr << "\n";
	}
	blck.bActive = false;

	if (blck.expl) {
		cout << "Explosion!\n";
		for (Block& item : blocks) {
			for (int adjacent : orngblocks[blck.number]) {
				if (adjacent == item.number && item.bActive) {
					blockHit(cmb, scr, item, blocks, orngblocks);
				}
			}
		}
	}
}


void calcNewBallXYSpeed(float& xspeed, float& yspeed, float rel, const float& ball_speed) {
	float tempx = xspeed + rel * ball_speed;
	float tempy = yspeed;
	float coef=abs(tempx/tempy);

	tempy = sqrt(ball_speed * ball_speed / (1 + coef * coef));

	if (tempx < 0) {
		xspeed = -coef * tempy;
	}
	else {
		xspeed = coef * tempy;
	}
	yspeed = tempy;
}


void checkCmdLnInput(int argc, char* argv[], int& _width, int& _height) {
	if (argc == 1) {
		cout << "Using default 800x600\n";
	}
	if (argc == 3) {
		string window_cmd = "-window";
		if (argv[1] == window_cmd) {
			string window_res = argv[2];
			string resx, resy;
			resx = resy = "";
			int x_or_y = 0;
			for (char c : window_res) {
				if (c != 'x') {
					if (x_or_y == 0) {
						resx.push_back(c);
					}
					else {
						resy.push_back(c);
					}
				}
				else{
					++x_or_y;
				}
			}
			try {
				int res_x = stoi(resx);
				int res_y = stoi(resy);
				cout << "Width " << res_x << "\n" << "Height " << res_y << "\n";
				_width = res_x;
				_height = res_y;
			}
			catch (exception& e) {
				cout << "Wrong window resolution format!\nUsing default 800x600\n";
			}
		}
		else {
			cout << "Wrong window resolution format!\nUsing default 800x600\n";
		}
	}
	if (argc == 2 || argc > 3) {
		cout << "Unknown command line input!\n";
	}
}

int main(int argc, char *argv[])
{
	checkCmdLnInput(argc, argv, _width, _height);

	Framework* MyGame = new MyFramework;

	return run(MyGame);
}
