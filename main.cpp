#include <iostream>
#include <chrono>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <unistd.h>
#include <termios.h>
#include "Types.h"

/*
  BLACK   30 40
  RED     31 41
  GREEN   32 42
  YELLOW  33 43
  BLUE    34 44
  MAGENTA 35 45
  CYAN    36 46
  WHITE   37 47

  RESET         0
  BOLD          1
  UNDERLINE     4
  INVERSE       7
  BOLD OFF      21
  UNDERLINE OFF 24
  INVERSE OFF   27
*/

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
static const char* CLEAR="cls";
#else
static const char* CLEAR="clear";
#endif

typedef enum PIXEL{
  _EMPTY  = 0x0,
  _PLAYER = 0x1,
  _SAND   = 0x2,
  _BRICK  = 0x3
} PIXEL;

typedef struct Map{
  u8 mapX{0}; // Max Size of 255
  u8 mapY{0};
  std::vector<std::vector<u16>> grid;
} Map;

void init(Map& map){
  map.mapX = 50;
  map.mapY = 40;
  map.grid.resize(map.mapX, std::vector<u16>(map.mapY, 0));
}

void inputThread(std::queue<char>& inputQueue, std::mutex& queueMutex, bool& running){
  bool run{running};

  while (run){
    char input;
    std::cin >> input;
    queueMutex.lock();
    inputQueue.push(input);
    queueMutex.unlock();

    if (input == 't'){
      run = false;
    }
  }
}

using namespace std::chrono_literals;

// PARTICLE UPDATES
void particleUpdate(Map& map, int& x, int& y){
  switch(map.grid[x][y]){
  default:
    break;
  case _SAND:
    if (y < map.mapY - 1){
      if(map.grid[x][y + 1] == _EMPTY){
        map.grid[x][y] = _EMPTY;
        map.grid[x][y + 1] = _SAND;
      }
      else if (map.grid[x + 1][y + 1] == _EMPTY){
        map.grid[x][y] = _EMPTY;
        map.grid[x + 1][y + 1] = _SAND;
      }
      else if (map.grid[x - 1][y + 1] == _EMPTY){
        map.grid[x][y] = _EMPTY;
        map.grid[x - 1][y + 1] = _SAND;
      }
    }
  }
}

void particleDraw(Map& map, int& x, int& y){
  switch (map.grid[x][y]){
  case _EMPTY:
    printf(".");
    break;
  case _PLAYER:
    printf("\033[1;37m█\033[0m");
    break;
  case _SAND:
    printf("\033[1;43m▒\033[0m");
    particleUpdate(map, x, y);
    break;
  case _BRICK:
    printf("\033[1;31m█\033[0m");
    break;
  }
}

int main(void){
  std::unique_ptr<Map> map = std::make_unique<Map>();

  bool running{true};

  u8 xPos{20};
  u8 yPos{15};

  u8 currentTile{0};
  u8 currentDraw{2};

  init(*map);

  map->grid[xPos][yPos] = 1;

  struct termios t;
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag &= ~ICANON;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);

  std::queue<char> inputQueue;
  std::mutex queueMutex;

  // input thread
  std::thread inputHandler(inputThread, std::ref(inputQueue), std::ref(queueMutex), std::ref(running));

  while(running){
    system(CLEAR);

    printf("\033[4;36mCOORDS %i | %i\033[0m\n", xPos, yPos);
    printf("\033[1;32mTILE %i\033[0m\n", currentTile);
    printf("\033[1;32mCURRENT PIXEL %i\033[0m\n", currentDraw);

    for (int y = 0; y < map->mapY; ++y){
      for (int x = 0; x < map->mapX; ++x){
        if (x % map->mapX == 0){
          printf("\n");
        }
        else{
          particleDraw(*map, x, y);
        }

        fflush(stdout);
        //std::this_thread::sleep_for(1ms);
      }
    }

    queueMutex.lock();
    while (!inputQueue.empty()){
      char input = inputQueue.front();
      inputQueue.pop();

      switch (input){
      default:
        break;
      case 'a':
        map->grid[xPos][yPos] = currentTile;
        xPos = (xPos > 1) ? xPos -= 1 : 1;
        currentTile = map->grid[xPos][yPos];
        map->grid[xPos][yPos] = 1;
        break;
      case 'd':
        map->grid[xPos][yPos] = currentTile;
        xPos = (xPos < map->mapX - 1) ? xPos += 1 : map->mapX - 1;
        currentTile = map->grid[xPos][yPos];
        map->grid[xPos][yPos] = 1;
        break;
      case 'w':
        map->grid[xPos][yPos] = currentTile;
        yPos = (yPos > 0) ? yPos -= 1 : 0;
        currentTile = map->grid[xPos][yPos];
        map->grid[xPos][yPos] = 1;
        break;
      case 's':
        map->grid[xPos][yPos] = currentTile;
        yPos = (yPos < map->mapY - 1) ? yPos += 1 : map->mapY - 1;
        currentTile = map->grid[xPos][yPos];
        map->grid[xPos][yPos] = 1;
        break;

      // PLACEMENT AND DESTRUCTION OF PARTICLE
      case 'e':
        currentTile = currentDraw;
        break;
      case 'r':
        for (int y = 0; y < map->mapY; ++y){
          for (int x = 0; x < map->mapX; ++x){
            map->grid[x][y] = 0;
          }
        }
      case 'q':
        currentTile = _EMPTY;
        break;

      // PARTICLE SELECTION
      case '1':
        currentDraw = _SAND;
        break;
      case '2':
        currentDraw = _BRICK;
        break;


      // TERMINATION OF APPLICATION
      case 't':
        running = false;
        break;
      }
    }

    queueMutex.unlock();

    std::this_thread::sleep_for(16ms);
  }

  inputHandler.join();

  system(CLEAR);
  printf("\033[1;41m | APPLICATION TERMINATED | \033[0m\n");

  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag |= ICANON;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);

  return 0;
}
