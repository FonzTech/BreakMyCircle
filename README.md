# BreakMyCircle

Puzzle Bobble-like game for PC and Mobile. Currently in development. This project is written as a showcase of my capabilities as a C++ game developer.

## Dependencies

- Magnum Engine: [https://magnum.graphics/](https://magnum.graphics/)
- Corrade: [https://github.com/mosra/corrade](https://github.com/mosra/corrade)

## Technical Notes

- The variable `mParentIndex` is NOT available for use in any of class hierarchy for `GameObject`, unless the index is explicitly passed, if the object is instantiated outside of the in-game room loader. For example, take a look at the `Player` class: it needs the `parentIndex` argument for its constructor (which introduces some redundancy), because it needs to look for other game objects on the same layer where it lies.
- To get status for a mouse button:

  `InputManager::singleton->mouseStates[ImMouseButtons::Left]`

  which can contain one of the following values:
  
  - `IM_STATE_NOT_PRESSED = 0`: the key is not pressed at all;
  - `IM_STATE_PRESSED = 1`: the key has been just pressed for this tick;
  - `IM_STATE_PRESSING = 2`: the key is being kept pressed for two or more ticks;
  - `IM_STATE_RELEASED = -1`: the key has been just released for this tick;

  To check if key is down or not, do one of the following:

  - `InputManager::singleton->mouseStates[ImMouseButtons::Left] <= 0`: the key is up;
  - `InputManager::singleton->mouseStates[ImMouseButtons::Left] > 0`: the key is down;

- As of now, all materials use Phong shader.
- The GOL_PERSP_FIRST is the only layer which has the object ID buffer, used for mouse picking.
- The GOL_ORTHO_FIRST is the only layer which draws its objects right after update (such as `OverlayText`).
- Regarding `BaseDrawable` and its subclasses, their copy constructor (kind-of, please see their implementation) does NOT copy all of the properties.
- It's advised to use the `AssetManager` to load a resource of type `Shaders::Phong`, otherwise the `Object ID` property will seem to be randomized due to an undefined value.
- Objects, whose name ends with `_AvoidMe`, are NOT imported by `AssetManager`.
- It's highly advised to use `WaterShader` for "square" drawables.
- ~~Blending between `OverlayText` and `OverlayGui` is incorrect due to the blending function used. This can be fixed, but it's a time consuming task. Please, avoid this.~~