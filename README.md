# BreakMyCircle

Puzzle Bobble-like game for PC and Mobile. Currently in development. This project is written as a showcase of my capabilities as a C++ game developer.

## Dependencies

- Corrade: [https://github.com/mosra/corrade](https://github.com/mosra/corrade)
- Magnum Graphics: [https://magnum.graphics/](https://magnum.graphics/)
- Magnum Plugins: [https://github.com/mosra/magnum-plugins](https://github.com/mosra/magnum-plugins)
- OpenAL Soft: [https://github.com/kcat/openal-soft](https://github.com/kcat/openal-soft)
- SDL2 (for Windows only): [https://www.libsdl.org/](https://www.libsdl.org/)

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

- Use the `DEBUG` macro to check whether it's a debug build or not. Visual Studio, Android Studio and Xcode projects are already setup to be conformant to this project codebase.
- Object picking in `GOL_PERSP_FIRST` can be toggled using the `InputManager::singleton->mReadObjectId` flag. This is useful in some situations where object picking is not required, since it *may* cause OpenGL pipeline to sync, thus losing performance and framerate.
- Please, keep the `GameObject::mDrawables` array clean and consistent, otherwise orphans drawables will appear. Also, layers where drawables go should be the same where the `GameObject` instances goes, otherwise drawables handling becomes difficult and tricky. Just create another class to handle that drawable on your required layer.
- The concept for **Detached Drawing** is simple: the engine follows a pipeline, to render all the object on it's layer. If something needs to be rendered outside this pipeline (like some off-screen drawing or render-to-texture operations), we are talking about **Detached Drawing**. Just implement the *IDrawDetached* interface to give an uniformity and coherence regarding drawing operations. Beware to not let drawables end up in `RoomManager`-managed `DrawableGroup` object!

~~The `OverlayText` is *ALWAYS detached*.~~

The `OverlayText` is *NOT ALWAYS* detached anymore. A dummy drawable is added to be processed by the engine pipeline, so draw order is preserved!
- The concept for **Cycle Waste** is basically wasting a cycle where the `GameObject::update` method, for a specific instance, gets called by the engine, without doing anything useful for the instance's logic itself. This is useful after a loading, like a scene, audio, etc... to avoid big *delta times*, like animation jumps, or sometimes even state jumps!
- As of now, ~~all~~ almost materials use Phong shader.
- The GOL_PERSP_FIRST is the only layer which has the object ID buffer, used for mouse picking.
- The GOL_ORTHO_FIRST is the only layer which draws its objects right after update (such as `OverlayText`).
- Regarding `BaseDrawable` and its subclasses, their copy constructor (kind-of, please see their implementation) does NOT copy all of the properties.
- It's advised to use the `AssetManager` to load a resource of type `Shaders::Phong`, otherwise the `Object ID` property will seem to be randomized due to an undefined value.
- Objects, whose name ends with `_AvoidMe`, are NOT imported by `AssetManager`.
- It's highly advised to use `WaterShader` for "square" drawables.
- ~~Blending between `OverlayText` and `OverlayGui` is incorrect due to the blending function used. This can be fixed, but it's a time consuming task. Please, avoid this.~~
- Transformations for `OverlayText` is only computed automatically on the first update. Later times, the method `setPosition` must be called to trigger transformations update. Otherwise the object will be rendered with out-of-date transformations.
- The `screen_quad.frag` shader program has an hard-coded effect to resemble a fog, which is very tighted to this specific game appearance.
- [This](https://github.com/mosra/magnum/blob/3d136503d8a959b4c260b9b60ca925566cc9d095/src/Magnum/GL/Shader.cpp#L722) makes shader compilation fail on some drivers, expecially on Samsung phones. A manually-compiled release of Magnum, without that `#line 1`, is required.
- Float precision for fragment shaders should be always `mediump`. This is important, since some Android OpenGL ES implementations throws an error about missing precision. Higher precision is almost useless.