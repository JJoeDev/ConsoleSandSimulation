#include <iostream>
#include <chrono>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include "Types.h"

typedef struct Map{
  u8 mapX{0}; // Max Size of 255
  u8 mapY{0};
  u8 newLiner{0};
  std::vector<std::vector<u16>> grid;
} Map;

void init(Map& map){
  map.mapX = 40;
  map.mapY = 30;
  map.newLiner = map.mapX;
  map.grid.resize(map.mapX, std::vector<u16>(map.mapY, 0));
}

void inputThread(std::queue<char>& inputQueue, std::mutex& queueMutex){
  while (true){
    char input;
    std::cin >> input;
    queueMutex.lock();
    inputQueue.push(input);
    queueMutex.unlock();
  }
}

using namespace std::chrono_literals;

int main(void){
  std::unique_ptr<Map> map = std::make_unique<Map>();

  bool running{true};

  u8 xPos{20};
  u8 yPos{15};

  init(*map);

  map->grid[xPos][yPos] = 1;

  std::queue<char> inputQueue;
  std::mutex queueMutex;

  // input thread
  std::thread inputHandler(inputThread, std::ref(inputQueue), std::ref(queueMutex));

  while(running){
    system("clear");

    for (int y = 0; y < map->mapY; ++y){
      for (int x = 0; x < map->mapX; ++x){
        if (x % map->newLiner == 0){
          printf("\n");
        }
        else{
          switch(map->grid[x][y]){
          case 0:
            printf(" ");
            break;
          case 1:
              printf("\033[1;31m@\033[0m");
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
        map->grid[xPos][yPos] = 0;
        xPos--;
        map->grid[xPos][yPos] = 1;
        break;
      case 'd':
        map->grid[xPos][yPos] = 0;
        xPos++;
        map->grid[xPos][yPos] = 1;
        break;
      case 'w':
        map->grid[xPos][yPos] = 0;
        yPos--;
        map->grid[xPos][yPos] = 1;
        break;
      case 's':
        map->grid[xPos][yPos] = 0;
        yPos++;
        map->grid[xPos][yPos] = 1;
        break;
      case 't':
        running = false;
        break;
      }
    }

    queueMutex.unlock();

    printf("\033[4;36mCOORDS: %i | %i\033[0m", xPos, yPos);

    std::this_thread::sleep_for(10ms);
  }

  inputHandler.join();

  system("clear");
  printf(" | APPLICATION TERMINATED | ");

  return 0;
}
