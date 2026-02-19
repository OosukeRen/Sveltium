#include "addon_api.h"
#include "sdl2_input.h"

using namespace sdl2input;

static ADDON_OBJECT_TYPE joystickInfoToJsObject(const JoystickInfo& joyInfo) {
  ADDON_OBJECT_TYPE obj = ADDON_OBJECT();

  ADDON_SET(obj, "deviceIndex", ADDON_INT(joyInfo.deviceIndex));
  ADDON_SET(obj, "name", ADDON_STRING(joyInfo.name));
  ADDON_SET(obj, "guid", ADDON_STRING(joyInfo.guid));
  ADDON_SET(obj, "numAxes", ADDON_INT(joyInfo.numAxes));
  ADDON_SET(obj, "numButtons", ADDON_INT(joyInfo.numButtons));
  ADDON_SET(obj, "numHats", ADDON_INT(joyInfo.numHats));
  ADDON_SET(obj, "numBalls", ADDON_INT(joyInfo.numBalls));
  ADDON_SET(obj, "isGameController", ADDON_BOOL(joyInfo.isGameController));

  return obj;
}

static ADDON_OBJECT_TYPE joystickStateToJsObject(const JoystickState& state) {
  ADDON_OBJECT_TYPE obj = ADDON_OBJECT();

  ADDON_ARRAY_TYPE axes = ADDON_ARRAY(state.axes.size());
  for (size_t i = 0; i < state.axes.size(); i++) {
    ADDON_SET_INDEX(axes, i, ADDON_INT(state.axes[i]));
  }
  ADDON_SET(obj, "axes", axes);

  ADDON_ARRAY_TYPE buttons = ADDON_ARRAY(state.buttons.size());
  for (size_t i = 0; i < state.buttons.size(); i++) {
    ADDON_SET_INDEX(buttons, i, ADDON_INT(state.buttons[i]));
  }
  ADDON_SET(obj, "buttons", buttons);

  ADDON_ARRAY_TYPE hats = ADDON_ARRAY(state.hats.size());
  for (size_t i = 0; i < state.hats.size(); i++) {
    ADDON_SET_INDEX(hats, i, ADDON_INT(state.hats[i]));
  }
  ADDON_SET(obj, "hats", hats);

  return obj;
}

static ADDON_OBJECT_TYPE gameControllerStateToJsObject(const GameControllerState& state) {
  ADDON_OBJECT_TYPE obj = ADDON_OBJECT();

  ADDON_SET(obj, "leftStickX", ADDON_INT(state.leftStickX));
  ADDON_SET(obj, "leftStickY", ADDON_INT(state.leftStickY));
  ADDON_SET(obj, "rightStickX", ADDON_INT(state.rightStickX));
  ADDON_SET(obj, "rightStickY", ADDON_INT(state.rightStickY));
  ADDON_SET(obj, "leftTrigger", ADDON_INT(state.leftTrigger));
  ADDON_SET(obj, "rightTrigger", ADDON_INT(state.rightTrigger));

  ADDON_SET(obj, "a", ADDON_BOOL(state.a));
  ADDON_SET(obj, "b", ADDON_BOOL(state.b));
  ADDON_SET(obj, "x", ADDON_BOOL(state.x));
  ADDON_SET(obj, "y", ADDON_BOOL(state.y));
  ADDON_SET(obj, "leftShoulder", ADDON_BOOL(state.leftShoulder));
  ADDON_SET(obj, "rightShoulder", ADDON_BOOL(state.rightShoulder));
  ADDON_SET(obj, "back", ADDON_BOOL(state.back));
  ADDON_SET(obj, "start", ADDON_BOOL(state.start));
  ADDON_SET(obj, "guide", ADDON_BOOL(state.guide));
  ADDON_SET(obj, "leftStick", ADDON_BOOL(state.leftStick));
  ADDON_SET(obj, "rightStick", ADDON_BOOL(state.rightStick));
  ADDON_SET(obj, "dpadUp", ADDON_BOOL(state.dpadUp));
  ADDON_SET(obj, "dpadDown", ADDON_BOOL(state.dpadDown));
  ADDON_SET(obj, "dpadLeft", ADDON_BOOL(state.dpadLeft));
  ADDON_SET(obj, "dpadRight", ADDON_BOOL(state.dpadRight));

  return obj;
}

static ADDON_OBJECT_TYPE mouseStateToJsObject(const MouseState& state) {
  ADDON_OBJECT_TYPE obj = ADDON_OBJECT();

  ADDON_SET(obj, "x", ADDON_INT(state.x));
  ADDON_SET(obj, "y", ADDON_INT(state.y));
  ADDON_SET(obj, "left", ADDON_BOOL(state.left));
  ADDON_SET(obj, "middle", ADDON_BOOL(state.middle));
  ADDON_SET(obj, "right", ADDON_BOOL(state.right));
  ADDON_SET(obj, "x1", ADDON_BOOL(state.x1));
  ADDON_SET(obj, "x2", ADDON_BOOL(state.x2));

  return obj;
}

ADDON_METHOD(Sdl2Init) {
  ADDON_ENV;
  bool result = init();
  ADDON_RETURN(ADDON_BOOL(result));
}

ADDON_METHOD(Sdl2Quit) {
  ADDON_ENV;
  quit();
  ADDON_VOID_RETURN();
}

ADDON_METHOD(Sdl2Update) {
  ADDON_ENV;
  update();
  ADDON_VOID_RETURN();
}

ADDON_METHOD(Sdl2GetNumJoysticks) {
  ADDON_ENV;
  int count = getNumJoysticks();
  ADDON_RETURN(ADDON_INT(count));
}

ADDON_METHOD(Sdl2GetJoystickInfo) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a device index number");
    ADDON_VOID_RETURN();
  }

  int deviceIndex = ADDON_TO_INT(ADDON_ARG(0));
  JoystickInfo joyInfo = getJoystickInfo(deviceIndex);
  ADDON_RETURN(joystickInfoToJsObject(joyInfo));
}

ADDON_METHOD(Sdl2IsGameController) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a device index number");
    ADDON_VOID_RETURN();
  }

  int deviceIndex = ADDON_TO_INT(ADDON_ARG(0));
  bool result = isGameController(deviceIndex);
  ADDON_RETURN(ADDON_BOOL(result));
}

ADDON_METHOD(Sdl2OpenJoystick) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a device index number");
    ADDON_VOID_RETURN();
  }

  int deviceIndex = ADDON_TO_INT(ADDON_ARG(0));
  SDL_Joystick* joy = openJoystick(deviceIndex);

  if (joy) {
    ADDON_RETURN(ADDON_EXTERNAL(joy));
  }
  ADDON_RETURN_NULL();
}

ADDON_METHOD(Sdl2CloseJoystick) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_EXTERNAL(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a joystick pointer");
    ADDON_VOID_RETURN();
  }

  SDL_Joystick* joy = static_cast<SDL_Joystick*>(ADDON_CAST_EXTERNAL_PTR(ADDON_ARG(0)));
  closeJoystick(joy);
  ADDON_VOID_RETURN();
}

ADDON_METHOD(Sdl2GetJoystickState) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_EXTERNAL(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a joystick pointer");
    ADDON_VOID_RETURN();
  }

  SDL_Joystick* joy = static_cast<SDL_Joystick*>(ADDON_CAST_EXTERNAL_PTR(ADDON_ARG(0)));
  JoystickState state = getJoystickState(joy);
  ADDON_RETURN(joystickStateToJsObject(state));
}

ADDON_METHOD(Sdl2RumbleJoystick) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 4) {
    ADDON_THROW_TYPE_ERROR("Expected 4 arguments: joystick, lowFreq, highFreq, durationMs");
    ADDON_VOID_RETURN();
  }

  if (!ADDON_IS_EXTERNAL(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a joystick pointer");
    ADDON_VOID_RETURN();
  }

  SDL_Joystick* joy = static_cast<SDL_Joystick*>(ADDON_CAST_EXTERNAL_PTR(ADDON_ARG(0)));
  int lowFreq = ADDON_TO_INT(ADDON_ARG(1));
  int highFreq = ADDON_TO_INT(ADDON_ARG(2));
  int durationMs = ADDON_TO_INT(ADDON_ARG(3));

  bool result = rumbleJoystick(joy, lowFreq, highFreq, durationMs);
  ADDON_RETURN(ADDON_BOOL(result));
}

ADDON_METHOD(Sdl2OpenGameController) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a device index number");
    ADDON_VOID_RETURN();
  }

  int deviceIndex = ADDON_TO_INT(ADDON_ARG(0));
  SDL_GameController* gc = openGameController(deviceIndex);

  if (gc) {
    ADDON_RETURN(ADDON_EXTERNAL(gc));
  }
  ADDON_RETURN_NULL();
}

ADDON_METHOD(Sdl2CloseGameController) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_EXTERNAL(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a game controller pointer");
    ADDON_VOID_RETURN();
  }

  SDL_GameController* gc = static_cast<SDL_GameController*>(ADDON_CAST_EXTERNAL_PTR(ADDON_ARG(0)));
  closeGameController(gc);
  ADDON_VOID_RETURN();
}

ADDON_METHOD(Sdl2GetGameControllerState) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_EXTERNAL(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a game controller pointer");
    ADDON_VOID_RETURN();
  }

  SDL_GameController* gc = static_cast<SDL_GameController*>(ADDON_CAST_EXTERNAL_PTR(ADDON_ARG(0)));
  GameControllerState state = getGameControllerState(gc);
  ADDON_RETURN(gameControllerStateToJsObject(state));
}

ADDON_METHOD(Sdl2RumbleGameController) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 4) {
    ADDON_THROW_TYPE_ERROR("Expected 4 arguments: gameController, lowFreq, highFreq, durationMs");
    ADDON_VOID_RETURN();
  }

  if (!ADDON_IS_EXTERNAL(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a game controller pointer");
    ADDON_VOID_RETURN();
  }

  SDL_GameController* gc = static_cast<SDL_GameController*>(ADDON_CAST_EXTERNAL_PTR(ADDON_ARG(0)));
  int lowFreq = ADDON_TO_INT(ADDON_ARG(1));
  int highFreq = ADDON_TO_INT(ADDON_ARG(2));
  int durationMs = ADDON_TO_INT(ADDON_ARG(3));

  bool result = rumbleGameController(gc, lowFreq, highFreq, durationMs);
  ADDON_RETURN(ADDON_BOOL(result));
}

ADDON_METHOD(Sdl2GetGameControllerName) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_EXTERNAL(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a game controller pointer");
    ADDON_VOID_RETURN();
  }

  SDL_GameController* gc = static_cast<SDL_GameController*>(ADDON_CAST_EXTERNAL_PTR(ADDON_ARG(0)));
  std::string name = getGameControllerName(gc);
  ADDON_RETURN(ADDON_STRING(name));
}

ADDON_METHOD(Sdl2GetMouseState) {
  ADDON_ENV;
  MouseState state = getMouseState();
  ADDON_RETURN(mouseStateToJsObject(state));
}

ADDON_METHOD(Sdl2GetGlobalMouseState) {
  ADDON_ENV;
  MouseState state = getGlobalMouseState();
  ADDON_RETURN(mouseStateToJsObject(state));
}

ADDON_METHOD(Sdl2GetRelativeMouseState) {
  ADDON_ENV;
  MouseState state = getRelativeMouseState();
  ADDON_RETURN(mouseStateToJsObject(state));
}

void InitSdl2Input(ADDON_INIT_PARAMS) {
  ADDON_EXPORT_FUNCTION(exports, "sdl2Init", Sdl2Init);
  ADDON_EXPORT_FUNCTION(exports, "sdl2Quit", Sdl2Quit);
  ADDON_EXPORT_FUNCTION(exports, "sdl2Update", Sdl2Update);
  ADDON_EXPORT_FUNCTION(exports, "sdl2GetNumJoysticks", Sdl2GetNumJoysticks);
  ADDON_EXPORT_FUNCTION(exports, "sdl2GetJoystickInfo", Sdl2GetJoystickInfo);
  ADDON_EXPORT_FUNCTION(exports, "sdl2IsGameController", Sdl2IsGameController);
  ADDON_EXPORT_FUNCTION(exports, "sdl2OpenJoystick", Sdl2OpenJoystick);
  ADDON_EXPORT_FUNCTION(exports, "sdl2CloseJoystick", Sdl2CloseJoystick);
  ADDON_EXPORT_FUNCTION(exports, "sdl2GetJoystickState", Sdl2GetJoystickState);
  ADDON_EXPORT_FUNCTION(exports, "sdl2RumbleJoystick", Sdl2RumbleJoystick);
  ADDON_EXPORT_FUNCTION(exports, "sdl2OpenGameController", Sdl2OpenGameController);
  ADDON_EXPORT_FUNCTION(exports, "sdl2CloseGameController", Sdl2CloseGameController);
  ADDON_EXPORT_FUNCTION(exports, "sdl2GetGameControllerState", Sdl2GetGameControllerState);
  ADDON_EXPORT_FUNCTION(exports, "sdl2RumbleGameController", Sdl2RumbleGameController);
  ADDON_EXPORT_FUNCTION(exports, "sdl2GetGameControllerName", Sdl2GetGameControllerName);
  ADDON_EXPORT_FUNCTION(exports, "sdl2GetMouseState", Sdl2GetMouseState);
  ADDON_EXPORT_FUNCTION(exports, "sdl2GetGlobalMouseState", Sdl2GetGlobalMouseState);
  ADDON_EXPORT_FUNCTION(exports, "sdl2GetRelativeMouseState", Sdl2GetRelativeMouseState);
}
