#include <iostream>
#include <SDL.h>
#include <cmath>
#include <string>
#include <vector>
#include <SDL_image.h>

const int MAP_WIDTH = 24;
const int MAP_HEIGHT = 24;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 1024;

const int TEX_WIDTH = 64;
const int TEX_HEIGHT = 64;

int worldMap[MAP_WIDTH][MAP_HEIGHT] =
{
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,7,7,7,7,7},
  {4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
  {4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
  {4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
  {4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
  {4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1},
  {6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,4,4,4,4},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
  {4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,4,4,4,4,4}
};

bool init();
void close();

bool loadMedia(std::vector<SDL_Texture*>&Textures);


SDL_Texture* loadImage(std::string path, SDL_Renderer* renderer);
SDL_Surface* loadSurface(std::string path);
SDL_Texture* loadStreamingTexture();

//Vector of static textures
std::vector<SDL_Texture*>Textures;

//Blank streaming textures to allow for pixel manipulation. 
//Individual pixels will be loaded from respective floor and ceiling surfaces to these, and then these will be rendered on the screen.
SDL_Texture* floorTexture = nullptr;
SDL_Texture* ceilTexture = nullptr;

//floor and ceiling surfaces that allow direct access to pixels.
SDL_Surface* floorSurface=nullptr;
SDL_Surface* ceilSurface=nullptr;

SDL_Surface* windowSurface = nullptr;

SDL_Window* mainWindow = nullptr;
SDL_Renderer* renderer = nullptr;

int main(int argc, char* argv[])
{
	//Starting position of the player
	double xPos = 17;
	double yPos = 6;
	//Initial direction the player is looking at
	double xDir = -1;
	double yDir = 0;

	//Initial camera plane for 66 degrees field of vision
	double xPlane = 0;
	double yPlane = 0.66;

	//Initialize SDL systems
	if (!init())
	{
		std::cout << "Unable to initialize!" << std::endl;
	}
	else
	{
		if (!loadMedia(Textures))
		{
			std::cout << "Could not load media!!" << std::endl;
		}
		else
		{


			bool quit = false;

			const double moveSpeed = 3*0.017;
			const double rotSpeed = 2 * 0.017;

			SDL_Event e;

			windowSurface = SDL_GetWindowSurface(mainWindow);
			floorSurface = loadSurface("assets/greystone.png");
			ceilSurface = loadSurface("assets/bluestone.png");
			floorTexture = loadStreamingTexture();
			ceilTexture = loadStreamingTexture();

			while (!quit)
			{
				double oldXDir;
				double oldPlaneX;

				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
				}
				
				SDL_SetRenderDrawColor(renderer, 100, 100, 100, 100);
				SDL_RenderClear(renderer);

				//We will cast rays at every vertical line on the screen, starting from the very left of the screen (i=0). When we hit a wall we will compute the distance to the wall from the player position. 
				//That will determine how high (in pixels) the wall will be drawn on the screen. 
				//After that, we will fill the rest of the screen with ceiling and floor pixels.

				for (int i = 0; i < SCREEN_WIDTH; i++)
				{
					//so that x=0 will get cameraX -1, and x=screen_width will get cameraX = +1
					double cameraX = 2 * i / double(SCREEN_WIDTH) - 1; 

					//X and Y direction vectors of the ray that has beeen cast.
					double rayXDir = xDir + xPlane * cameraX;
					double rayYDir = yDir + yPlane * cameraX;

					//where in the map are we??
					int xMap = int(xPos);
					int yMap = int(yPos);

					double firstXDist; //the distances to the first x and y intersections, then will be incremented by one step until a wall has been hit
					double firstYDist; //

					double nextXDist = std::abs(1 / rayXDir); //the distance from one intersection (x and y) to the other. this will be added to the first distance to calculate the total distance until a wall has been hit.
					double nextYDist = std::abs(1 / rayYDir); //

					double WallDist; //perpendicular distance to the wall

					int xIncrement;//x and y map position the ray is currently in.
					int yIncrement;

					bool hitWall = false;
					bool eastWestWall = false;

					//determine direction of the ray and initial distances in x and y directions
					if (rayXDir < 0)
					{
						xIncrement = -1;
						//calculate distance to the nearest x-side of a box from triangle similarity
						firstXDist = (xPos - xMap)*nextXDist;
					}
					else
					{
						xIncrement = 1;
						firstXDist = (xMap + 1 - xPos)*nextXDist;
						
					}
					if (rayYDir < 0)
					{
						yIncrement = -1;
						firstYDist = (yPos - yMap)*nextYDist;
					}
					else
					{
						yIncrement = 1;
						firstYDist = (yMap + 1 - yPos)*nextYDist;
					}
					//once we know the x and y directions of the ray that has been cast, we increment (or decrement) the x and y coordinates on the map by 
					while (hitWall == false)
					{ 
						if (firstXDist < firstYDist) //firstXDist and firstYDist changes meaning here, they now mean the total distance traversed in X and Y directions, respectively.
						{							 //we now move from one x intersection to the other, checking if we have hit a wall each time.
							firstXDist += nextXDist; //if we hit a wall after we increment in the x direction, it means the wall is in north south orientation. 					
							xMap += xIncrement;		 //Only east-west and north south walls are possible with this kind of ray casting.
							eastWestWall = false;
						}
						else
						{
							firstYDist += nextYDist;
							yMap += yIncrement;
							eastWestWall = true;
						}
						if (worldMap[xMap][yMap] > 0) hitWall = true;

					}

					if (eastWestWall == false) WallDist = (xMap - xPos + (1 - xIncrement) / 2) / rayXDir; //Calculate perpendicular distance to the camera plane to avoid fisheye effect.
					else WallDist = (yMap - yPos + (1 - yIncrement) / 2) / rayYDir;

					//calculate how large the wall is going to be drawn on the screen, and center it on the screen
					int lineHeight = (int)(SCREEN_HEIGHT / WallDist);

					int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
					if (drawStart < 0)drawStart = 0;
					int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
					if (drawEnd >= SCREEN_HEIGHT)drawEnd = SCREEN_HEIGHT - 1;

					int texNum = worldMap[xMap][yMap] - 1;

					//calculate the map x location of the texture that was hit.
					double wallX;
					if (eastWestWall == false) wallX = yPos + WallDist * rayYDir;
					else wallX = xPos + WallDist * rayXDir;
					wallX -= floor(wallX);

					//transform the location to texture coordinates.
					int texX = int(wallX * double(TEX_WIDTH));
					if (eastWestWall== false && rayXDir > 0) texX = TEX_WIDTH - texX - 1;
					if (eastWestWall == true && rayYDir < 0) texX = TEX_WIDTH - texX - 1;
					
					int texY;//source rectangle vertical draw start
					int sRectH; //source rectangle draw height

					if (lineHeight <= SCREEN_HEIGHT)
					{
						texY = 0;
						sRectH = TEX_HEIGHT;
					}
					else 
					{
						sRectH = floor(SCREEN_HEIGHT*TEX_HEIGHT / lineHeight);
						texY = floor((TEX_HEIGHT - (SCREEN_HEIGHT*TEX_HEIGHT / lineHeight)) / 2);
					}

					SDL_Rect sourceRect{ texX, texY, 1, sRectH };
					SDL_Rect destRect{ i, drawStart, 1, drawEnd-drawStart };

					if (texNum>=0)
					SDL_RenderCopy(renderer, Textures[texNum], &sourceRect, &destRect);


					//FLOOR CASTING


					double floorXWall, floorYWall; //x, y position of the floor texel at the bottom of the wall

					//4 different wall directions possible
					if (eastWestWall == 0 && rayXDir > 0)
					{
						floorXWall = xMap;
						floorYWall = yMap + wallX;
					}
					else if (eastWestWall == 0 && rayXDir < 0)
					{
						floorXWall = xMap + 1.0;
						floorYWall = yMap + wallX;
					}
					else if (eastWestWall == 1 && rayYDir > 0)
					{
						floorXWall = xMap + wallX;
						floorYWall = yMap;
					}
					else
					{
						floorXWall = xMap + wallX;
						floorYWall = yMap + 1.0;
					}

					double distWall, distPlayer, currentDist,nextDist;

					distWall = WallDist;

					distPlayer = 0.0;

					//These two arrays will contain the floor and ceiling pixels to be drawn at the current horizontal screen coordinate. 
					//I tried using a vector but it was slower than an array
					Uint32* floorVec = new Uint32[drawStart];
					Uint32* ceilVec = new Uint32[drawStart];
					//accessing the pixels of the floor and ceiling surfaces.
					Uint32* pixsflr = (Uint32*)floorSurface->pixels;
					Uint32* pixsceil = (Uint32*)ceilSurface->pixels;

					if (drawEnd < 0) drawEnd = SCREEN_HEIGHT; //becomes < 0 when the integer overflows
					
					//draw the floor from drawEnd to the bottom of the screen

					for (int y = drawEnd + 1; y < SCREEN_HEIGHT; y+=32)			//we are going to "calculate" every 32nd pixel, and use linear interpolation to approximate the ones in between.
					{															// this is done to avoid doing a division for every single pixel.
						currentDist = SCREEN_HEIGHT / (2.0 * y - SCREEN_HEIGHT); //It could be also worth using a look up table instead of the first division

						double weight = (currentDist - distPlayer) / (distWall - distPlayer);

						double currentFloorX = weight * floorXWall + (1.0 - weight) * xPos; //X and Y locations of the first texel
						double currentFloorY = weight * floorYWall + (1.0 - weight) * yPos; //

						nextDist = SCREEN_HEIGHT / (2.0 * (y+32) - SCREEN_HEIGHT);			//the next "true" texel. used for linear interpolating the pixels in between.

						weight = (nextDist - distPlayer) / (distWall - distPlayer);

						double nextFloorX = weight * floorXWall + (1.0 - weight) * xPos;
						double nextFloorY = weight * floorYWall + (1.0 - weight) * yPos;

						double addX, addY;

						addX = (nextFloorX - currentFloorX)*0.03125;
						addY = (nextFloorY - currentFloorY)*0.03125;

						int floorTexX, floorTexY;

						int c = 32;
						int j = y;

						while (c--&&j<SCREEN_HEIGHT)
						{
							floorTexX = int(currentFloorX * TEX_WIDTH) % TEX_WIDTH; //This could be a bit slow as well. 
							floorTexY = int(currentFloorY * TEX_HEIGHT) % TEX_HEIGHT;

							Uint32 pixflr = (Uint32)pixsflr[64 * floorTexY + floorTexX]; //copy the current floor and ceiling pixels to the respective pixel arrays
							Uint32 pixceil = (Uint32)pixsceil[64 * floorTexY + floorTexX];//

							floorVec[j - drawEnd - 1] = pixflr;
							ceilVec[SCREEN_HEIGHT - j] = pixceil;

							currentFloorX += addX;
							currentFloorY += addY;

							j++;
						}

					}

					//Once we have the floor and ceiling pixel arrays to be rendered, we lock the blank floor and ceiling textures to manipulate their pixels. 
					//After that, we copy&paste the pixel array contents to the blank textures with memcpy. 
					//Once we are done manipulating the texture, we unlock it and copy the contents to the renderer. Note that the "texture" is just a vertical strip of pixels to be drawn
					int pitch = 1 * sizeof(Uint32);
					void* mPixels=nullptr;
				
					size_t floorSize = (SCREEN_HEIGHT - lineHeight) / 2;
					SDL_Rect rect{ 0,0,1,floorSize };

					SDL_LockTexture(floorTexture, &rect, &mPixels, &pitch);
					
					memcpy(mPixels, floorVec, drawStart*pitch);
					
					SDL_UnlockTexture(floorTexture);
					mPixels = nullptr;
					
					SDL_Rect dstflr{ i, drawStart + lineHeight,1,floorSize };
					SDL_RenderCopy(renderer,floorTexture , &rect, &dstflr);
					delete [] floorVec;

					SDL_LockTexture(ceilTexture, &rect, &mPixels, &pitch);
					memcpy(mPixels, ceilVec, drawStart*pitch);

					SDL_UnlockTexture(ceilTexture);

					SDL_Rect dstclg = { i, 0, 1, floorSize };
					SDL_RenderCopy(renderer, ceilTexture, &rect, &dstclg);
					delete[]ceilVec;
				}

				SDL_RenderPresent(renderer);

				//check keyboard keystates for movement. forward and backward movement checks if we are sufficiently away from any walls.
				const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

				if (currentKeyStates[SDL_SCANCODE_UP])
				{
					if (worldMap[int(xPos + xDir * moveSpeed*10)][int(yPos)] == false)
						xPos += xDir * moveSpeed;

					if (worldMap[int(xPos)][int(yPos + yDir * moveSpeed*10)] == false)
						yPos += yDir * moveSpeed;

				}
				if (currentKeyStates[SDL_SCANCODE_DOWN])
				{
					if (worldMap[int(xPos - xDir * moveSpeed*10)][int(yPos)] == false)
						xPos -= xDir * moveSpeed;

					if (worldMap[int(xPos)][int(yPos - yDir * moveSpeed*10)] == false)
						yPos -= yDir * moveSpeed;

				}
				//rotate the player and camera (xPlane and yPlane) with RotSpeed
				if (currentKeyStates[SDL_SCANCODE_LEFT])
				{
					oldXDir = xDir;
					xDir = xDir * cos(rotSpeed) - yDir * sin(rotSpeed);
					yDir = oldXDir * sin(rotSpeed) + yDir * cos(rotSpeed);
					oldPlaneX = xPlane;
					xPlane = xPlane * cos(rotSpeed) - yPlane * sin(rotSpeed);
					yPlane = oldPlaneX * sin(rotSpeed) + yPlane * cos(rotSpeed);
				}
				if (currentKeyStates[SDL_SCANCODE_RIGHT])
				{
					oldXDir = xDir;
					xDir = xDir * cos(-rotSpeed) - yDir * sin(-rotSpeed);
					yDir = oldXDir * sin(-rotSpeed) + yDir * cos(-rotSpeed);
					oldPlaneX = xPlane;
					xPlane = xPlane * cos(-rotSpeed) - yPlane * sin(-rotSpeed);
					yPlane = oldPlaneX * sin(-rotSpeed) + yPlane * cos(-rotSpeed);
				}



			}
		}
		
	}
	close();
	return 0;
}

bool init()
{	//Initialize SDL systems, create window and renderer.
	bool success = true;
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "Failed to initialize SDL! SDL Error: " << SDL_GetError() << std::endl;
		success = false;
	}
	else
	{
		mainWindow = SDL_CreateWindow("RayCaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		
		if (mainWindow == nullptr)
		{
			std::cout << "Failed to create SDL_Window SDL Error: " << SDL_GetError() << std::endl;
			success = false;
		}
		else 
		{
			renderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == nullptr)
			{
				std::cout << "Failed to create SDL_Renderer SDL Error: " << SDL_GetError() << std::endl;
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);

			}
		}
	}
	return success;
}


void close()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(mainWindow);

	SDL_DestroyTexture(floorTexture);
	SDL_DestroyTexture(ceilTexture);

	for (auto tex:Textures) 
		SDL_DestroyTexture(tex);

	
	SDL_FreeSurface(floorSurface);
	SDL_FreeSurface(ceilSurface);

	mainWindow = nullptr;
	renderer = nullptr;

	SDL_Quit();
}


bool loadMedia(std::vector<SDL_Texture*>&textures)
{
	//load the wall textures.

	bool success = true;
	SDL_Texture* tempTexture=nullptr;

	tempTexture = loadImage("assets/bluestone.png", renderer);

	if (tempTexture==nullptr)
	{
		std::cout << "Could not load bluestone texture!!" << std::endl;
		success = false;
	}
	else textures.push_back(tempTexture);

	tempTexture = loadImage("assets/colorstone.png",renderer);

	if (tempTexture== nullptr)
	{
		std::cout << "Could not load colorstone texture!!" << std::endl;
		success = false;
	}
	else textures.push_back(tempTexture);

	tempTexture= loadImage("assets/eagle.png",renderer);

	if (tempTexture == nullptr)
	{
		std::cout << "Could not load eagle texture!!" << std::endl;
		success = false;
	}
	else textures.push_back(tempTexture);

	tempTexture = loadImage("assets/greystone.png",renderer);

	if (tempTexture ==nullptr)
	{
		std::cout << "Could not load greystone texture!!" << std::endl;
		success = false;
	}
	else textures.push_back(tempTexture);

	tempTexture = loadImage("assets/mossy.png",renderer);

	if (tempTexture ==nullptr)
	{
		std::cout << "Could not load second mossy texture!!" << std::endl;
		success = false;
	}
	else textures.push_back(tempTexture);

	tempTexture = loadImage("assets/purplestone.png",renderer);

	if (tempTexture == nullptr)
	{
		std::cout << "Could not load purplestone texture!!" << std::endl;
		success = false;
	}
	else textures.push_back(tempTexture);

	tempTexture = loadImage("assets/redbrick.png",renderer);

	if (tempTexture ==nullptr)
	{
		std::cout << "Could not load redbrick texture!!" << std::endl;
		success = false;
	}
	else textures.push_back(tempTexture);

	tempTexture = loadImage("assets/wood.png",renderer);

	if (tempTexture == nullptr)
	{
		std::cout << "Could not load wood texture!!" << std::endl;
		success = false;
	}
	textures.push_back(tempTexture);

	tempTexture = loadImage("assets/greystone.png", renderer);


	return success;
}

SDL_Texture* loadImage(std::string path, SDL_Renderer* renderer)
{
	//load the images on temporary surfaces, colorkey them (not used here really but would be useful with sprites.) create textures and destroy temp surfaces.
	SDL_Surface* tempSurface = IMG_Load(path.c_str());

	if (tempSurface == nullptr)
	{
		std::cout << "Unable to load image. SDL_Error: " << SDL_GetError() << std::endl;
	}
	else
	{
		SDL_SetColorKey(tempSurface, SDL_TRUE, SDL_MapRGB(tempSurface->format, 0, 0xFF, 0xFF));
	}
	SDL_Texture* tempTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
	SDL_FreeSurface(tempSurface);
	return tempTexture;
	
}

SDL_Surface* loadSurface(std::string path)
{
	// used to create floor and ceiling surfaces which allow us to access the pixels directly.
	SDL_RWops* rwop = SDL_RWFromFile(path.c_str(), "rb");
	SDL_Surface* loaded = IMG_LoadPNG_RW(rwop);
	SDL_Surface* conv = NULL;
	if (loaded != NULL)
	{
		conv = SDL_ConvertSurface(loaded, SDL_GetWindowSurface(mainWindow)->format, 0);
		SDL_FreeSurface(loaded);
	}
	return conv;
}

void free(SDL_Texture* texture)
{
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}


SDL_Texture* loadStreamingTexture()
{
	//create streaming textures that allow pixel manipulation. 
	SDL_Texture* newTexture = nullptr;
	SDL_Surface* windowSurface = SDL_GetWindowSurface(mainWindow);

	newTexture = SDL_CreateTexture(renderer, windowSurface->format->format,
		SDL_TEXTUREACCESS_STREAMING, 1, SCREEN_HEIGHT);
	if (newTexture == nullptr)
	{
		printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
	}
	return newTexture;
}

