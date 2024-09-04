#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

//for loading sprites from png
#include "load_save_png.hpp"
#include "data_path.hpp"

#include <chrono>
#include <unistd.h>
#include <algorithm>

//used to print values (for debugging)
#include <glm/gtx/string_cast.hpp>

#include <random>

/*static std::vector < uint8_t > xpos = {
	0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7
};
static std::vector < uint8_t > ypos = {
	7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0
};*/

static std::vector <uint8_t > indices = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

//seed code from https://cplusplus.com/reference/random/mersenne_twister_engine/mersenne_twister_engine/
static std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count());
static std::vector <uint8_t> correctqueue = {};
static std::vector <uint8_t> repeatqueue = {};
static auto lastTime = std::chrono::system_clock::now().time_since_epoch().count();
static uint8_t patternlength = 3;
static uint8_t rollsLeft = 0;
static uint8_t highlighted = 255;
static uint8_t highlightsLeft = 0;
static uint8_t highlightWhole = 0;

auto timeNow(){
	return std::chrono::system_clock::now().time_since_epoch().count();
}

bool submitAnswer() {
	auto j = correctqueue.begin();
	if (correctqueue.size() != repeatqueue.size()){
		return false;
	}
	for (auto i = repeatqueue.begin(); i != repeatqueue.end(); ++i){
		std::cout << std::to_string(*i) << " " << std::to_string(indices[*j]) << std::endl;
		if (*i != indices[*j]) {
			return false;
		}
		++j;
	}
	return true;
}

void rollGrid() {
	indices = {};
	static std::vector <uint8_t> pickfrom;
	pickfrom = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	};
	static uint8_t generated;
	while (indices.size() < 16) {
		generated = rng() % pickfrom.size();
		indices.emplace_back(pickfrom[generated]);
		pickfrom.erase(pickfrom.begin() + generated);
	}
	std::cout << "Queue: ";
	for (auto iq = indices.begin(); iq != indices.end(); ++iq){
		std::cout << std::to_string(*iq) << ", ";
	}
	std::cout << std::endl;
}

void rollQueue() {
	correctqueue = {};
	repeatqueue = {};
	static std::vector <uint8_t> pickfrom;
	pickfrom = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	};
	static uint8_t generated;
	while (correctqueue.size() < patternlength) {
		generated = rng() % pickfrom.size();
		correctqueue.emplace_back(generated);
		pickfrom.erase(pickfrom.begin() + generated);
	}
	std::cout << "Correct Queue: ";
	for (auto cq = correctqueue.begin(); cq != correctqueue.end(); ++cq){
		std::cout << std::to_string(*cq) << ", ";
	}
	std::cout << std::endl;
}

void animateRound() {
	lastTime = std::chrono::system_clock::now().time_since_epoch().count();
	rollsLeft = rng() % 6 + 6;
}

void highlightQueue() {
	highlightsLeft = correctqueue.size() * 2 + 2;
	std::reverse(correctqueue.begin(), correctqueue.end());
}

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:

	{ //use tiles 0-16 as some weird dot pattern thing:
		// std::array< uint8_t, 8*8 > distance;
		/*for (uint32_t y = 0; y < 8; ++y) {
			for (uint32_t x = 0; x < 8; ++x) {
				float d = glm::length(glm::vec2((x + 0.5f) - 4.0f, (y + 0.5f) - 4.0f));
				d /= glm::length(glm::vec2(4.0f, 4.0f));
				distance[x+8*y] = uint8_t(std::max(0,std::min(255,int32_t( 255.0f * d ))));
			}
		}
		for (uint32_t index = 0; index < 16; ++index) {
			PPU466::Tile tile;
			uint8_t t = uint8_t((255 * index) / 16);
			for (uint32_t y = 0; y < 8; ++y) {
				uint8_t bit0 = 0;
				uint8_t bit1 = 0;
				for (uint32_t x = 0; x < 8; ++x) {
					uint8_t d = distance[x+8*y];
					if (d > t) {
						bit0 |= (1 << x);
					} else {
						bit1 |= (1 << x);
					}
				}
				tile.bit0[y] = bit0;
				tile.bit1[y] = bit1;
			}
			ppu.tile_table[index] = tile;
		}*/
		//Just want a static background for simplicity's sake
		PPU466::Tile tile;
			for (uint32_t y = 0; y < 8; ++y) {
				tile.bit0[y] = 0;
				tile.bit1[y] = 0;
			}
		ppu.tile_table[0] = tile;
		ppu.palette_table[0] = {glm::u8vec4(0xff, 0xff, 0xff, 0xff), glm::u8vec4(0xff, 0xff, 0xff, 0xff), glm::u8vec4(0xff, 0xff, 0xff, 0xff), glm::u8vec4(0xff, 0xff, 0xff, 0xff)};
		ppu.palette_table[1] = {glm::u8vec4(0xff, 0x00, 0x00, 0xff), glm::u8vec4(0x00, 0xff, 0x00, 0xff), glm::u8vec4(0x00, 0x00, 0xff, 0xff), glm::u8vec4(0xff, 0x00, 0x00, 0xff)};
		ppu.palette_table[6] = {glm::u8vec4(0xff, 0x00, 0x00, 0xff), glm::u8vec4(0xff, 0x00, 0x00, 0xff), glm::u8vec4(0xff, 0x00, 0x00, 0xff), glm::u8vec4(0xff, 0x00, 0x00, 0xff)};
		ppu.palette_table[7] = {glm::u8vec4(0x00, 0xff, 0x00, 0xff), glm::u8vec4(0x00, 0xff, 0x00, 0xff), glm::u8vec4(0x00, 0xff, 0x00, 0xff), glm::u8vec4(0xff, 0x00, 0x00, 0xff)};
	}
	// Not sure this works relative to exe.
	static std::string filename = "tiles.png";
	static std::string fullpath = data_path(filename);
	std::cout << filename << std::endl;
	glm::uvec2 size = {0,0};
	std::vector< glm::u8vec4 > data(64, glm::u8vec4(0x00, 0x00, 0x00, 0x00)); 
	load_png(fullpath, &size, &data, UpperLeftOrigin);
	//prints data vector in full
	/*for (uint32_t i = 0; i < data.size(); i++){
		std::cout << glm::to_string(data[i]) << std::endl;
	}*/

	//ASSET PIPELINE (PNG to Game)
	//Iterate over sprite rows
	for (uint8_t row = 0; row < 8; row++) {
		std::array< glm::u8vec4, 4 > colors = {glm::u8vec4(0x00, 0x00, 0x00, 0x00), glm::u8vec4(0x00, 0x00, 0x00, 0x00), glm::u8vec4(0x00, 0x00, 0x00, 0x00), glm::u8vec4(0x00, 0x00, 0x00, 0x00)};
		//Iterate over sprite cols
		for (uint8_t col = 0; col < 8; col++) {
			// palette colors (init as transparent since I am not using transparent values)
			std::array< uint8_t, 8 > bit0 = {0,0,0,0,0,0,0,0};
			std::array< uint8_t, 8 > bit1 = {0,0,0,0,0,0,0,0};
			
			//Iterate over height pixels
			/*if (row == 0){
				std::cout << "Row: " << std::to_string(row) << std::endl;
				std::cout << "Col: " << std::to_string(col) << std::endl;
			}*/
			for (uint8_t y = 0; y < 8; y++) {
				//Iterate over width pixels
				for (uint8_t x = 0; x < 8; x++) {
					uint32_t index = row*8*size[0] + y*size[0] + col*8 + x;
					
					//std::cout << std::to_string(index) << std::endl;
					glm::u8vec4 pixel = data[index];

					/*if (row == 0){
						std::cout << std::to_string(index) << std::endl;
						std::cout << glm::to_string(pixel) << std::endl;
					}*/
					//std::cout << glm::to_string(pixel) << std::endl;
					for (uint8_t cindex = 0; cindex < 4; cindex++){
						if (colors[cindex] == pixel){
							bit0[7-y] += ((cindex % 2) << x);
							bit1[7-y] += ((cindex / 2) << x);
							break;
						}
						if (colors[cindex] == glm::u8vec4(0x00, 0x00, 0x00, 0x00)){
							colors[cindex] = pixel;
							bit0[7-y] += ((cindex % 2) << x);
							bit1[7-y] += ((cindex / 2) << x);
							break;
						}
					}
				}
			}
			ppu.tile_table[row*8 + col + 64].bit0 = bit0;
			ppu.tile_table[row*8 + col + 64].bit1 = bit1;
			// This palette setting method only works because of the png I am loading
			// Letting it do this multiple times because it shouldn't affect load time much.
			ppu.palette_table[row/2+2] = colors;
			// std::cout << "setting palette to... " << glm::to_string(colors[0]) << glm::to_string(colors[1]) << glm::to_string(colors[2]) << glm::to_string(colors[3]) << std::endl;
		}
	}
	/*for (uint32_t j = 0; j < 4; j++) {
		std::cout << "palette is... ";
		for (uint32_t k = 0; k < 4; k++){
			std::cout << glm::to_string(ppu.palette_table[j][k]) << "; ";
		}
		std::cout << std::endl;
	}*/



	//use sprite 32 as a "player":
	/*ppu.tile_table[32].bit0 = {
		0b01111110,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b01111110,
	};
	ppu.tile_table[32].bit1 = {
		0b00000000,
		0b00000000,
		0b00011000,
		0b00100100,
		0b00000000,
		0b00100100,
		0b00000000,
		0b00000000,
	};*/

	//makes the outside of tiles 0-16 solid:
	/*ppu.palette_table[0] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//makes the center of tiles 0-16 solid:
	ppu.palette_table[1] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//used for the player:
	ppu.palette_table[7] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xff, 0xff, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//used for the misc other sprites:
	ppu.palette_table[6] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x88, 0x88, 0xff, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};*/

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	/*if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}*/
	if (evt.button.type == SDL_MOUSEBUTTONDOWN) {
		if (evt.button.button == SDL_BUTTON_LEFT) {
			left.pressed = true;
			left.downs += 1;
			left.x = evt.button.x/2;
			left.y = ((PPU466::ScreenHeight*2+8) - evt.button.y)/2;
			//std::cout << std::to_string(left.x) << ", " <<  std::to_string(left.y) << std::endl;
			return true;
		} else if (evt.button.button == SDL_BUTTON_MIDDLE) {
			middle.pressed = true;
			middle.downs += 1;
			middle.x = evt.button.x/2;
			middle.y = ((PPU466::ScreenHeight*2+8) - evt.button.y)/2;
			if (submitAnswer()){
				highlightWhole = 1;
			} else {
				highlightWhole = 2;
			}
			lastTime = timeNow();
		} else {
			assert (evt.button.button == SDL_BUTTON_RIGHT);
			right.pressed = true;
			right.downs += 1;
			right.x = evt.button.x/2;
			right.y = ((PPU466::ScreenHeight*2+8) - evt.button.y)/2;
		}
	} else if (evt.button.type == SDL_MOUSEBUTTONUP) {
		if (evt.button.button == SDL_BUTTON_LEFT) {
			left.pressed = false;
			left.x = evt.button.x/2;
			left.y = ((PPU466::ScreenHeight*2+8) - evt.button.y)/2;
			if(pos.pressed != 255) {
				repeatqueue.emplace_back(pos.pressed);
				std::cout << "Queue: ";
				for (auto rq = repeatqueue.begin(); rq != repeatqueue.end(); ++rq){
					std::cout << std::to_string(*rq) << ", ";
				}
				std::cout << std::endl;
			}
			return true;
		} else if (evt.button.button == SDL_BUTTON_MIDDLE) {
			middle.pressed = false;
			middle.x = evt.button.x/2;
			middle.y = ((PPU466::ScreenHeight*2+8) - evt.button.y)/2;
		} else {
			assert (evt.button.button == SDL_BUTTON_RIGHT);
			right.pressed = false;
			right.x = evt.button.x/2;
			right.y = ((PPU466::ScreenHeight*2+8) - evt.button.y)/2;
			if (repeatqueue.size() > 0) {
				repeatqueue.pop_back();
			}
		}
	} else if (evt.motion.type == SDL_MOUSEMOTION) {
		pos.x = evt.motion.x/2;
		pos.y = ((PPU466::ScreenHeight*2+8) - evt.motion.y)/2;
		const uint32_t adjustx = pos.x - 97;
		const uint32_t adjusty = pos.y - 89;
		if (adjustx < 72){
			if (adjusty < 72) {
				pos.pressed = adjustx/18 + ((3-(adjusty/18)) * 4);
				
			}else {
				// Using pressed to indicate which tile is selected.
				pos.pressed = 255;
			}
		} else {
			// Using pressed to indicate which tile is selected.
			pos.pressed = 255;
		}
		//std::cout << std::to_string(pos.pressed) << std::endl;
		//std::cout << std::to_string(pos.x) << ", " <<  std::to_string(pos.y) << std::endl;
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	// background_fade += elapsed / 10.0f;
	// background_fade -= std::floor(background_fade);

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	middle.downs = 0;
	if (rollsLeft > 0) {
		if (timeNow() - lastTime > 50000000){
			rollGrid();
			rollsLeft-=1;
			lastTime = timeNow();
		} 
	}
	if (highlightsLeft > 0) {
		if (timeNow() - lastTime > 500000000){
			if (highlightsLeft > 2 && highlightsLeft % 2 == 1){
				highlighted = correctqueue[(highlightsLeft-3) / 2];
				std::cout << "highlighting: " << std::to_string(correctqueue[(highlightsLeft-3) / 2]) << std::endl;
			} else {
				highlighted = 255;
			}
			highlightsLeft -= 1;
			lastTime = timeNow();
			if (highlightsLeft == 0) {
				std::reverse(correctqueue.begin(), correctqueue.end());
				animateRound();
			}
		}
	}
	if (highlightWhole == 1) {
		if (timeNow() - lastTime > 1000000000) {
			indices = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
			highlightWhole = 0;
			rollQueue();
			highlightQueue();
		}
	} else if (highlightWhole == 2)
		if (timeNow() - lastTime > 1000000000) {
			indices = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
			highlightWhole = 0;
			rollQueue();
			highlightQueue();
		}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	/*ppu.background_color = glm::u8vec4(
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 1.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 2.0f / 3.0f) ) ) ))),
		0xff
	);*/
	ppu.background_color = glm::u8vec4(0xff, 0xff, 0xff, 0xff);

	//tilemap gets recomputed every frame as some weird plasma thing:
	
	//Dev Note: Intentionally killing background, deemed it unnecessary for the game.
	//Base code left commented in case I change my mind.

	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			//Old plasma bg
			//ppu.background[x+PPU466::BackgroundWidth*y] = ((x+y)%16);
			//Blank bg
			ppu.background[x+PPU466::BackgroundWidth*y] = 0;
		}
	}

	//background scroll: None
	ppu.background_position.x = int32_t(0);
	ppu.background_position.y = int32_t(0);

	//player sprite:
	/*ppu.sprites[0].x = int8_t(player_at.x);
	ppu.sprites[0].y = int8_t(player_at.y);
	ppu.sprites[0].index = 32;
	ppu.sprites[0].attributes = 7;*/

	//some other misc sprites:
	for (uint32_t i = 0; i < 64; i++) {
		//ppu.sprites[i].x = int8_t(((float) (xpos[i])) * 8.0f + (float)(xpos[i]/2) * 2.0f + PPU466::ScreenWidth/2.0f - 35.0f);
		//ppu.sprites[i].y = int8_t(((float) (ypos[i])) * 8.0f + (float)(ypos[i]/2) * 2.0f + PPU466::ScreenHeight/2.0f - 35.0f);
		static uint32_t indexpos;
		static uint32_t xpos, ypos;
		// ix = (((i % 16) % 8) / 2)
		static uint32_t i_16;
		i_16 = (((i % 16) % 8) / 2) + i/16 * 4;
		indexpos = indices[i_16];
		//std::cout<< std::to_string(indexpos) << std::endl;
		xpos = (indexpos % 4) * 2 + i % 2;
		ypos = 7 - ((indexpos / 4) * 2 + (i/8) % 2);
		
		ppu.sprites[i].x = int8_t(((float) (xpos)) * 8.0f + (float)(xpos/2) * 2.0f + 95.0f);
		ppu.sprites[i].y = int8_t(((float) (ypos)) * 8.0f + (float)(ypos/2) * 2.0f + 87.0f);
		ppu.sprites[i].index = i+64;
		ppu.sprites[i].attributes = i/16+2;

		//std::cout << "hovering... " << std::to_string(base) << std::endl;
		
		//if ((i == base || i == base + 1 || i == base + 8 || i == base + 9) && pos.pressed != 255){
		if (indexpos == pos.pressed || indexpos == highlighted){
			ppu.palette_table[1] = {ppu.palette_table[i_16/4+2][2],ppu.palette_table[i_16/4+2][1],ppu.palette_table[i_16/4+2][0],ppu.palette_table[i_16/4+2][3]}; 
			//std::cout<< std::to_string(indexpos) << " " << std::to_string(i_16) << " " << std::to_string(pos.pressed)<< std::endl;
			ppu.sprites[i].attributes = 1;
		}
		if (highlightWhole == 2){
			ppu.sprites[i].attributes = 6;
		} else if (highlightWhole == 1){
			ppu.sprites[i].attributes = 7;
		}
	}
	

	//--- actually draw ---
	ppu.draw(drawable_size);
}
