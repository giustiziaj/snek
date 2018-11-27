#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <cmath>

#define minSnekSize 10   // starting number of cells for snek's length
#define timeStep .06     // (float) number of seconds between gamesteps
#define cellsize 6       // size of individual cells in game grid (should divide width and height)
#define screenwidth 300  // horizontal resolution of window in pixels
#define screenheight 600 // vertical resolution of window in pixels
#define scorePerFood 2   // scale factor for snek length increase every time he eats food

#define fontfile "Hack-Regular.ttf" //font for game over screen

using namespace std;
using namespace sf;

//Check if direction change is valid.
//In Snek, the player cannot turn 180
constexpr Keyboard::Key directionChange(Keyboard::Key cd, Keyboard::Key nd) {
  switch (cd) {
    case Keyboard::Up:
      if (nd != Keyboard::Down) return nd;
      break;
    case Keyboard::Down:
      if (nd != Keyboard::Up) return nd;
      break;
    case Keyboard::Left:
      if (nd != Keyboard::Right) return nd;
      break;
    case Keyboard::Right:
      if (nd != Keyboard::Left) return nd;
      break;
    default: return nd;
  }
  return cd;
}

//show gameover screen then exit after 3 seconds
void gameOver(RenderWindow* window, int score) {
  window->clear();
  Text gameOverMsg;
  Font f;
  f.loadFromFile(fontfile);
  gameOverMsg.setFont(f);
  stringstream msgText;
  msgText << "Game Over!" << endl << "Score: " << score;
  gameOverMsg.setString(msgText.str());
  gameOverMsg.setCharacterSize(20);
  gameOverMsg.setFillColor(Color::White);
  gameOverMsg.setPosition(10,10);
  window->draw(gameOverMsg);
  window->display();
  sleep(seconds(3));
  window->close();
  exit(0);
}

//compare vectors
bool operator==(const Vector2f& a, const Vector2f& b) {
  return (a.x == b.x) && (a.y == b.y);
}

// Calculate distance between two vectors
constexpr int distance(const Vector2f& a, const Vector2f& b) {
  float dx = abs(a.x - b.x);
  float dy = abs(a.y - b.y);
  return ceil(sqrt((dy * dy) + (dx * dx)));
}

//ensure cells are snapped to valid grid positions
void snapToGrid(Vector2f& v) {
  v.x = floor(v.x / cellsize) * cellsize;
  v.y = floor(v.y / cellsize) * cellsize;
}

//create a new food at a random location at game start or when the snek eats a food
RectangleShape* generateFood(Vector2f headPos) {
  Vector2f foodPos = Vector2f(headPos);
  while(distance(foodPos,headPos) < 10 * cellsize) {
    foodPos = Vector2f(rand() % screenwidth,
                       rand() % screenheight);
    snapToGrid(foodPos);
  }
  RectangleShape* r = new RectangleShape(Vector2f(cellsize,cellsize));
  r->setFillColor(Color::White);
  r->setPosition(foodPos);
  return r;
}

// Update window title whenever score increases
void updateTitle(Window* w, int score) {
  stringstream ss;
  ss << "Snek: " << score;
  w->setTitle(ss.str());
}

int main() {
  srand(std::time(0));
  Clock* clock = new Clock();
  Keyboard::Key direction;
  queue<RectangleShape*> cells;
  RectangleShape cell(Vector2f(cellsize,cellsize));
  cell.setFillColor(Color::White);
  Vector2f startPos = Vector2f(screenwidth/2, screenheight/2);
  snapToGrid(startPos);
  cell.setPosition(startPos);
  int snekSize = minSnekSize;
  cells.push(new sf::RectangleShape(cell));
  RectangleShape* food = generateFood(cell.getPosition());

  RenderWindow* window = new sf::RenderWindow(
    VideoMode(screenwidth, screenheight),
    "Snek");
  while (window->isOpen()) {
    // Handle Events
    sf::Event event;
    if (window->pollEvent(event)) {
      switch (event.type) {
        case Event::KeyPressed:
          if (event.key.code == Keyboard::Up ||
              event.key.code == Keyboard::Down ||
              event.key.code == Keyboard::Left ||
              event.key.code == Keyboard::Right) {
                direction = directionChange(direction, event.key.code);
          }
          else if (event.key.code == Keyboard::Escape) {
            window->close();
            break;
          }

          break;
        case sf::Event::Closed:
          window->close();
          break;
        default: continue;
      }
    }
    //Lazy redraw when game state changes (every timestep)
    if(clock->getElapsedTime() > seconds(timeStep)) {
      clock->restart();
      //rect is the next block of the snek; the 'head'
      RectangleShape* rect = new RectangleShape(*cells.back());
      switch(direction) {
        case Keyboard::Up:
          rect->move(0,-cellsize);
          break;
        case Keyboard::Down:
          rect->move(0,cellsize);
          break;
        case Keyboard::Left:
          rect->move(-cellsize,0);
          break;
        case Keyboard::Right:
          rect->move(cellsize,0);
          break;
        default: continue;
      }

      Vector2f pos = rect->getPosition();
      //Handle warping movement
      if (pos.x < 0) pos.x += screenwidth;
      if (pos.x > screenwidth) pos.x -= screenwidth;
      if (pos.y < 0) pos.y += screenheight;
      if (pos.y > screenheight) pos.y -= screenheight;
      snapToGrid(pos);
      rect->setPosition(pos);

      //Check if dead
      for(int i = 0; i < cells.size(); i++) {
        Vector2f _pos = cells.front()->getPosition();
        if (pos == _pos) {
          gameOver(window, snekSize - minSnekSize);
        }
        cells.push(cells.front());
        cells.pop();
      }

      //Check for food
      Vector2f foodPos = food->getPosition();
      if (pos == foodPos) {
        snekSize += scorePerFood;
        food = generateFood(pos);
        updateTitle(window, snekSize - minSnekSize);
      }

      cells.push(rect);
      //Make sure snek is right length
      while(cells.size() > snekSize && snekSize >= minSnekSize) {
        cells.pop();
      }
    }
    window->clear();
    //Draw food
    window->draw(*food);
    //Draw Snek
    for (int i = 0; i < cells.size(); i++) {
        RectangleShape* r = cells.front();
        window->draw(*r);
        cells.push(cells.front());
        cells.pop();
    }
    window->display();
  }
  return 0;
}
