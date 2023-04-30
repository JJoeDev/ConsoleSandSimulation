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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
static const char* CLEAR="cls";
#else
static const char* CLEAR="clear";
#endif

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
void sandUpdate(Map& map, int& x, int& y){
  if (map.grid[x][y + 1] == 0 && y > 2){
    map.grid[x][y] = 0;
    map.grid[x][y + 1] = 2;
  }
}

int main(void){
  std::unique_ptr<Map> map = std::make_unique<Map>();

  bool running{true};

  u8 xPos{20};
  u8 yPos{15};

  u8 currentTile{0};

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

    for (int y = map->mapY - 2; y > 0; --y){
    //for (int y = 0; y < map->mapY; ++y){
      for (int x = 0; x < map->mapX; ++x){
        if (x % map->mapX == 0){
          printf("\n");
        }
        else{
          switch(map->grid[x][y]){
          case 0:
            printf(".");
            break;
          case 1:
            printf("\033[1;31m█\033[0m");
            break;
          case 2:
            printf("\033[1;33m▒\033[0m");
            sandUpdate(*map, x, y);
            break;
          }
        }
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
        yPos = (yPos < map->mapY - 2) ? yPos += 1 : map->mapY - 2;
        currentTile = map->grid[xPos][yPos];
        map->grid[xPos][yPos] = 1;
        break;
      case 's':
        map->grid[xPos][yPos] = currentTile;
        yPos = (yPos > 2) ? yPos -= 1 : 2;
        currentTile = map->grid[xPos][yPos];
        map->grid[xPos][yPos] = 1;
        break;
      case 'e':
        currentTile = 2;
        break;
      case 't':
        running = false;
        break;
      }
    }

    queueMutex.unlock();

    printf("\033[4;36mCOORDS: %i | %i\033[0m", xPos, yPos);

    std::this_thread::sleep_for(1000ms);
  }

  inputHandler.join();

  system(CLEAR);
  printf("\033[1;41m | APPLICATION TERMINATED | \033[0m\n");

  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag |= ICANON;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);

  return 0;
}
