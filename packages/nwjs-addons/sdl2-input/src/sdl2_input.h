#pragma once

#include <SDL.h>
#include <string>
#include <vector>

namespace sdl2input {

struct JoystickInfo {
  int deviceIndex;
  std::string name;
  std::string guid;
  int numAxes;
  int numButtons;
  int numHats;
  int numBalls;
  bool isGameController;
};

struct JoystickState {
  std::vector<int> axes;
  std::vector<bool> buttons;
  std::vector<int> hats;
};

struct GameControllerState {
  int leftStickX;
  int leftStickY;
  int rightStickX;
  int rightStickY;
  int leftTrigger;
  int rightTrigger;
  bool a;
  bool b;
  bool x;
  bool y;
  bool leftShoulder;
  bool rightShoulder;
  bool back;
  bool start;
  bool guide;
  bool leftStick;
  bool rightStick;
  bool dpadUp;
  bool dpadDown;
  bool dpadLeft;
  bool dpadRight;
};

struct MouseState {
  int x;
  int y;
  bool left;
  bool middle;
  bool right;
  bool x1;
  bool x2;
};

bool init();
void quit();
void update();

int getNumJoysticks();
JoystickInfo getJoystickInfo(int deviceIndex);
bool isGameController(int deviceIndex);
SDL_Joystick* openJoystick(int deviceIndex);
void closeJoystick(SDL_Joystick* joy);
JoystickState getJoystickState(SDL_Joystick* joy);
bool rumbleJoystick(SDL_Joystick* joy, int lowFreq, int highFreq, int durationMs);

SDL_GameController* openGameController(int deviceIndex);
void closeGameController(SDL_GameController* gc);
GameControllerState getGameControllerState(SDL_GameController* gc);
bool rumbleGameController(SDL_GameController* gc, int lowFreq, int highFreq, int durationMs);
std::string getGameControllerName(SDL_GameController* gc);

MouseState getMouseState();
MouseState getGlobalMouseState();
MouseState getRelativeMouseState();

}
