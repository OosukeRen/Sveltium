#include <nan.h>
#include "sdl2_input.h"

using namespace v8;
using namespace sdl2input;

static Local<Object> joystickInfoToJsObject(const JoystickInfo& info) {
  Local<Object> obj = Nan::New<Object>();

  Nan::Set(obj, Nan::New("deviceIndex").ToLocalChecked(),
    Nan::New(info.deviceIndex));

  Nan::Set(obj, Nan::New("name").ToLocalChecked(),
    Nan::New(info.name).ToLocalChecked());

  Nan::Set(obj, Nan::New("guid").ToLocalChecked(),
    Nan::New(info.guid).ToLocalChecked());

  Nan::Set(obj, Nan::New("numAxes").ToLocalChecked(),
    Nan::New(info.numAxes));

  Nan::Set(obj, Nan::New("numButtons").ToLocalChecked(),
    Nan::New(info.numButtons));

  Nan::Set(obj, Nan::New("numHats").ToLocalChecked(),
    Nan::New(info.numHats));

  Nan::Set(obj, Nan::New("numBalls").ToLocalChecked(),
    Nan::New(info.numBalls));

  Nan::Set(obj, Nan::New("isGameController").ToLocalChecked(),
    Nan::New(info.isGameController));

  return obj;
}

static Local<Object> joystickStateToJsObject(const JoystickState& state) {
  Local<Object> obj = Nan::New<Object>();

  Local<Array> axes = Nan::New<Array>(static_cast<uint32_t>(state.axes.size()));
  for (size_t i = 0; i < state.axes.size(); i++) {
    Nan::Set(axes, static_cast<uint32_t>(i), Nan::New(state.axes[i]));
  }
  Nan::Set(obj, Nan::New("axes").ToLocalChecked(), axes);

  Local<Array> buttons = Nan::New<Array>(static_cast<uint32_t>(state.buttons.size()));
  for (size_t i = 0; i < state.buttons.size(); i++) {
    Nan::Set(buttons, static_cast<uint32_t>(i), Nan::New(state.buttons[i]));
  }
  Nan::Set(obj, Nan::New("buttons").ToLocalChecked(), buttons);

  Local<Array> hats = Nan::New<Array>(static_cast<uint32_t>(state.hats.size()));
  for (size_t i = 0; i < state.hats.size(); i++) {
    Nan::Set(hats, static_cast<uint32_t>(i), Nan::New(state.hats[i]));
  }
  Nan::Set(obj, Nan::New("hats").ToLocalChecked(), hats);

  return obj;
}

static Local<Object> gameControllerStateToJsObject(const GameControllerState& state) {
  Local<Object> obj = Nan::New<Object>();

  Nan::Set(obj, Nan::New("leftStickX").ToLocalChecked(), Nan::New(state.leftStickX));
  Nan::Set(obj, Nan::New("leftStickY").ToLocalChecked(), Nan::New(state.leftStickY));
  Nan::Set(obj, Nan::New("rightStickX").ToLocalChecked(), Nan::New(state.rightStickX));
  Nan::Set(obj, Nan::New("rightStickY").ToLocalChecked(), Nan::New(state.rightStickY));
  Nan::Set(obj, Nan::New("leftTrigger").ToLocalChecked(), Nan::New(state.leftTrigger));
  Nan::Set(obj, Nan::New("rightTrigger").ToLocalChecked(), Nan::New(state.rightTrigger));

  Nan::Set(obj, Nan::New("a").ToLocalChecked(), Nan::New(state.a));
  Nan::Set(obj, Nan::New("b").ToLocalChecked(), Nan::New(state.b));
  Nan::Set(obj, Nan::New("x").ToLocalChecked(), Nan::New(state.x));
  Nan::Set(obj, Nan::New("y").ToLocalChecked(), Nan::New(state.y));
  Nan::Set(obj, Nan::New("leftShoulder").ToLocalChecked(), Nan::New(state.leftShoulder));
  Nan::Set(obj, Nan::New("rightShoulder").ToLocalChecked(), Nan::New(state.rightShoulder));
  Nan::Set(obj, Nan::New("back").ToLocalChecked(), Nan::New(state.back));
  Nan::Set(obj, Nan::New("start").ToLocalChecked(), Nan::New(state.start));
  Nan::Set(obj, Nan::New("guide").ToLocalChecked(), Nan::New(state.guide));
  Nan::Set(obj, Nan::New("leftStick").ToLocalChecked(), Nan::New(state.leftStick));
  Nan::Set(obj, Nan::New("rightStick").ToLocalChecked(), Nan::New(state.rightStick));
  Nan::Set(obj, Nan::New("dpadUp").ToLocalChecked(), Nan::New(state.dpadUp));
  Nan::Set(obj, Nan::New("dpadDown").ToLocalChecked(), Nan::New(state.dpadDown));
  Nan::Set(obj, Nan::New("dpadLeft").ToLocalChecked(), Nan::New(state.dpadLeft));
  Nan::Set(obj, Nan::New("dpadRight").ToLocalChecked(), Nan::New(state.dpadRight));

  return obj;
}

static Local<Object> mouseStateToJsObject(const MouseState& state) {
  Local<Object> obj = Nan::New<Object>();

  Nan::Set(obj, Nan::New("x").ToLocalChecked(), Nan::New(state.x));
  Nan::Set(obj, Nan::New("y").ToLocalChecked(), Nan::New(state.y));
  Nan::Set(obj, Nan::New("left").ToLocalChecked(), Nan::New(state.left));
  Nan::Set(obj, Nan::New("middle").ToLocalChecked(), Nan::New(state.middle));
  Nan::Set(obj, Nan::New("right").ToLocalChecked(), Nan::New(state.right));
  Nan::Set(obj, Nan::New("x1").ToLocalChecked(), Nan::New(state.x1));
  Nan::Set(obj, Nan::New("x2").ToLocalChecked(), Nan::New(state.x2));

  return obj;
}

NAN_METHOD(Sdl2Init) {
  bool result = init();
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(Sdl2Quit) {
  quit();
}

NAN_METHOD(Sdl2Update) {
  update();
}

NAN_METHOD(Sdl2GetNumJoysticks) {
  int count = getNumJoysticks();
  info.GetReturnValue().Set(Nan::New(count));
}

NAN_METHOD(Sdl2GetJoystickInfo) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("First argument must be a device index number");
    return;
  }

  int deviceIndex = Nan::To<int>(info[0]).FromJust();
  JoystickInfo joyInfo = getJoystickInfo(deviceIndex);
  info.GetReturnValue().Set(joystickInfoToJsObject(joyInfo));
}

NAN_METHOD(Sdl2IsGameController) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("First argument must be a device index number");
    return;
  }

  int deviceIndex = Nan::To<int>(info[0]).FromJust();
  bool result = isGameController(deviceIndex);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(Sdl2OpenJoystick) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("First argument must be a device index number");
    return;
  }

  int deviceIndex = Nan::To<int>(info[0]).FromJust();
  SDL_Joystick* joy = openJoystick(deviceIndex);

  if (joy) {
    info.GetReturnValue().Set(Nan::New<External>(joy));
  } else {
    info.GetReturnValue().SetNull();
  }
}

NAN_METHOD(Sdl2CloseJoystick) {
  if (info.Length() < 1 || !info[0]->IsExternal()) {
    Nan::ThrowTypeError("First argument must be a joystick pointer");
    return;
  }

  SDL_Joystick* joy = static_cast<SDL_Joystick*>(Local<External>::Cast(info[0])->Value());
  closeJoystick(joy);
}

NAN_METHOD(Sdl2GetJoystickState) {
  if (info.Length() < 1 || !info[0]->IsExternal()) {
    Nan::ThrowTypeError("First argument must be a joystick pointer");
    return;
  }

  SDL_Joystick* joy = static_cast<SDL_Joystick*>(Local<External>::Cast(info[0])->Value());
  JoystickState state = getJoystickState(joy);
  info.GetReturnValue().Set(joystickStateToJsObject(state));
}

NAN_METHOD(Sdl2RumbleJoystick) {
  if (info.Length() < 4) {
    Nan::ThrowTypeError("Expected 4 arguments: joystick, lowFreq, highFreq, durationMs");
    return;
  }

  if (!info[0]->IsExternal()) {
    Nan::ThrowTypeError("First argument must be a joystick pointer");
    return;
  }

  SDL_Joystick* joy = static_cast<SDL_Joystick*>(Local<External>::Cast(info[0])->Value());
  int lowFreq = Nan::To<int>(info[1]).FromJust();
  int highFreq = Nan::To<int>(info[2]).FromJust();
  int durationMs = Nan::To<int>(info[3]).FromJust();

  bool result = rumbleJoystick(joy, lowFreq, highFreq, durationMs);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(Sdl2OpenGameController) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("First argument must be a device index number");
    return;
  }

  int deviceIndex = Nan::To<int>(info[0]).FromJust();
  SDL_GameController* gc = openGameController(deviceIndex);

  if (gc) {
    info.GetReturnValue().Set(Nan::New<External>(gc));
  } else {
    info.GetReturnValue().SetNull();
  }
}

NAN_METHOD(Sdl2CloseGameController) {
  if (info.Length() < 1 || !info[0]->IsExternal()) {
    Nan::ThrowTypeError("First argument must be a game controller pointer");
    return;
  }

  SDL_GameController* gc = static_cast<SDL_GameController*>(Local<External>::Cast(info[0])->Value());
  closeGameController(gc);
}

NAN_METHOD(Sdl2GetGameControllerState) {
  if (info.Length() < 1 || !info[0]->IsExternal()) {
    Nan::ThrowTypeError("First argument must be a game controller pointer");
    return;
  }

  SDL_GameController* gc = static_cast<SDL_GameController*>(Local<External>::Cast(info[0])->Value());
  GameControllerState state = getGameControllerState(gc);
  info.GetReturnValue().Set(gameControllerStateToJsObject(state));
}

NAN_METHOD(Sdl2RumbleGameController) {
  if (info.Length() < 4) {
    Nan::ThrowTypeError("Expected 4 arguments: gameController, lowFreq, highFreq, durationMs");
    return;
  }

  if (!info[0]->IsExternal()) {
    Nan::ThrowTypeError("First argument must be a game controller pointer");
    return;
  }

  SDL_GameController* gc = static_cast<SDL_GameController*>(Local<External>::Cast(info[0])->Value());
  int lowFreq = Nan::To<int>(info[1]).FromJust();
  int highFreq = Nan::To<int>(info[2]).FromJust();
  int durationMs = Nan::To<int>(info[3]).FromJust();

  bool result = rumbleGameController(gc, lowFreq, highFreq, durationMs);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(Sdl2GetGameControllerName) {
  if (info.Length() < 1 || !info[0]->IsExternal()) {
    Nan::ThrowTypeError("First argument must be a game controller pointer");
    return;
  }

  SDL_GameController* gc = static_cast<SDL_GameController*>(Local<External>::Cast(info[0])->Value());
  std::string name = getGameControllerName(gc);
  info.GetReturnValue().Set(Nan::New(name).ToLocalChecked());
}

NAN_METHOD(Sdl2GetMouseState) {
  MouseState state = getMouseState();
  info.GetReturnValue().Set(mouseStateToJsObject(state));
}

NAN_METHOD(Sdl2GetGlobalMouseState) {
  MouseState state = getGlobalMouseState();
  info.GetReturnValue().Set(mouseStateToJsObject(state));
}

NAN_METHOD(Sdl2GetRelativeMouseState) {
  MouseState state = getRelativeMouseState();
  info.GetReturnValue().Set(mouseStateToJsObject(state));
}

void InitSdl2Input(Local<Object> exports) {
  Nan::Set(exports, Nan::New("sdl2Init").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2Init)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2Quit").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2Quit)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2Update").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2Update)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2GetNumJoysticks").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2GetNumJoysticks)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2GetJoystickInfo").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2GetJoystickInfo)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2IsGameController").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2IsGameController)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2OpenJoystick").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2OpenJoystick)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2CloseJoystick").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2CloseJoystick)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2GetJoystickState").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2GetJoystickState)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2RumbleJoystick").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2RumbleJoystick)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2OpenGameController").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2OpenGameController)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2CloseGameController").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2CloseGameController)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2GetGameControllerState").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2GetGameControllerState)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2RumbleGameController").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2RumbleGameController)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2GetGameControllerName").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2GetGameControllerName)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2GetMouseState").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2GetMouseState)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2GetGlobalMouseState").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2GetGlobalMouseState)).ToLocalChecked());

  Nan::Set(exports, Nan::New("sdl2GetRelativeMouseState").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Sdl2GetRelativeMouseState)).ToLocalChecked());
}
