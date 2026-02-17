#include "sdl2_input.h"
#include <SDL_gamecontroller.h>

namespace sdl2input {

static bool g_initialized = false;

bool init() {
  bool success = false;

  if (g_initialized) {
    success = true;
  } else {
    int result = SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
    if (result == 0) {
      g_initialized = true;
      success = true;
    }
  }

  return success;
}

void quit() {
  if (g_initialized) {
    SDL_Quit();
    g_initialized = false;
  }
}

void update() {
  SDL_JoystickUpdate();
}

int getNumJoysticks() {
  return SDL_NumJoysticks();
}

JoystickInfo getJoystickInfo(int deviceIndex) {
  JoystickInfo info;
  info.deviceIndex = deviceIndex;

  const char* name = SDL_JoystickNameForIndex(deviceIndex);
  info.name = name ? name : "";

  SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(deviceIndex);
  char guidStr[33];
  SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
  info.guid = guidStr;

  SDL_Joystick* joy = SDL_JoystickOpen(deviceIndex);
  if (joy) {
    info.numAxes = SDL_JoystickNumAxes(joy);
    info.numButtons = SDL_JoystickNumButtons(joy);
    info.numHats = SDL_JoystickNumHats(joy);
    info.numBalls = SDL_JoystickNumBalls(joy);
    SDL_JoystickClose(joy);
  } else {
    info.numAxes = 0;
    info.numButtons = 0;
    info.numHats = 0;
    info.numBalls = 0;
  }

  info.isGameController = SDL_IsGameController(deviceIndex) == SDL_TRUE;

  return info;
}

bool isGameController(int deviceIndex) {
  return SDL_IsGameController(deviceIndex) == SDL_TRUE;
}

SDL_Joystick* openJoystick(int deviceIndex) {
  return SDL_JoystickOpen(deviceIndex);
}

void closeJoystick(SDL_Joystick* joy) {
  if (joy) {
    SDL_JoystickClose(joy);
  }
}

JoystickState getJoystickState(SDL_Joystick* joy) {
  JoystickState state;

  if (!joy) {
    return state;
  }

  int numAxes = SDL_JoystickNumAxes(joy);
  state.axes.resize(numAxes);
  for (int i = 0; i < numAxes; i++) {
    state.axes[i] = SDL_JoystickGetAxis(joy, i);
  }

  int numButtons = SDL_JoystickNumButtons(joy);
  state.buttons.resize(numButtons);
  for (int i = 0; i < numButtons; i++) {
    state.buttons[i] = SDL_JoystickGetButton(joy, i) != 0;
  }

  int numHats = SDL_JoystickNumHats(joy);
  state.hats.resize(numHats);
  for (int i = 0; i < numHats; i++) {
    state.hats[i] = SDL_JoystickGetHat(joy, i);
  }

  return state;
}

bool rumbleJoystick(SDL_Joystick* joy, int lowFreq, int highFreq, int durationMs) {
  bool success = false;

  if (joy) {
    Uint16 low = static_cast<Uint16>(lowFreq);
    Uint16 high = static_cast<Uint16>(highFreq);
    Uint32 duration = static_cast<Uint32>(durationMs);
    int result = SDL_JoystickRumble(joy, low, high, duration);
    success = (result == 0);
  }

  return success;
}

SDL_GameController* openGameController(int deviceIndex) {
  return SDL_GameControllerOpen(deviceIndex);
}

void closeGameController(SDL_GameController* gc) {
  if (gc) {
    SDL_GameControllerClose(gc);
  }
}

GameControllerState getGameControllerState(SDL_GameController* gc) {
  GameControllerState state;

  if (!gc) {
    return state;
  }

  state.leftStickX = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTX);
  state.leftStickY = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTY);
  state.rightStickX = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTX);
  state.rightStickY = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTY);
  state.leftTrigger = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
  state.rightTrigger = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

  state.a = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_A) != 0;
  state.b = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_B) != 0;
  state.x = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_X) != 0;
  state.y = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_Y) != 0;
  state.leftShoulder = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) != 0;
  state.rightShoulder = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) != 0;
  state.back = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_BACK) != 0;
  state.start = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_START) != 0;
  state.guide = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_GUIDE) != 0;
  state.leftStick = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_LEFTSTICK) != 0;
  state.rightStick = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_RIGHTSTICK) != 0;
  state.dpadUp = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
  state.dpadDown = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
  state.dpadLeft = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0;
  state.dpadRight = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;

  return state;
}

bool rumbleGameController(SDL_GameController* gc, int lowFreq, int highFreq, int durationMs) {
  bool success = false;

  if (gc) {
    Uint16 low = static_cast<Uint16>(lowFreq);
    Uint16 high = static_cast<Uint16>(highFreq);
    Uint32 duration = static_cast<Uint32>(durationMs);
    int result = SDL_GameControllerRumble(gc, low, high, duration);
    success = (result == 0);
  }

  return success;
}

std::string getGameControllerName(SDL_GameController* gc) {
  std::string name;

  if (gc) {
    const char* n = SDL_GameControllerName(gc);
    if (n) {
      name = n;
    }
  }

  return name;
}

MouseState getMouseState() {
  MouseState state;
  int x = 0;
  int y = 0;
  Uint32 buttons = SDL_GetMouseState(&x, &y);

  state.x = x;
  state.y = y;
  state.left = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
  state.middle = (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  state.right = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
  state.x1 = (buttons & SDL_BUTTON(SDL_BUTTON_X1)) != 0;
  state.x2 = (buttons & SDL_BUTTON(SDL_BUTTON_X2)) != 0;

  return state;
}

MouseState getGlobalMouseState() {
  MouseState state;
  int x = 0;
  int y = 0;
  Uint32 buttons = SDL_GetGlobalMouseState(&x, &y);

  state.x = x;
  state.y = y;
  state.left = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
  state.middle = (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  state.right = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
  state.x1 = (buttons & SDL_BUTTON(SDL_BUTTON_X1)) != 0;
  state.x2 = (buttons & SDL_BUTTON(SDL_BUTTON_X2)) != 0;

  return state;
}

MouseState getRelativeMouseState() {
  MouseState state;
  int x = 0;
  int y = 0;
  Uint32 buttons = SDL_GetRelativeMouseState(&x, &y);

  state.x = x;
  state.y = y;
  state.left = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
  state.middle = (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  state.right = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
  state.x1 = (buttons & SDL_BUTTON(SDL_BUTTON_X1)) != 0;
  state.x2 = (buttons & SDL_BUTTON(SDL_BUTTON_X2)) != 0;

  return state;
}

}
