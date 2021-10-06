# On-going tasks
- Fix collision detection for projectile on its side.
- Add shooting path while player is aiming.
- Save settings to device.

# To be verified
- Implement other level game objects, such as rocks, holes, and others...
- Fix transparency when blackholes and explode-effect overlaps.
- Improve level selector scrolling.

# Optional tasks
- Use `magnum-fontconverter` instead of loading `TTF` directly.

# Tasks done

- ~~Asset Manager decoupled from game objects.~~
- ~~Make IDrawCallback a callback to receive updates, such as drawable transformations (position, rotation, etc&#8230;).~~
- ~~Create prototype for projectile and collision against other bubbles.~~
- ~~Fix location for new bubbles, when the static positioning places it in an already occupied spot.~~
- ~~Destroy nearby bubbles, with the same color, when one is hit by a projectile.~~
- ~~Make bubble fall, if not supported by one bubble on top, at least.~~
- ~~Add Magnum's resource manager. Fix pending references.~~
- ~~Fix transparency and Z-Ordering problems.~~
- ~~Lambda as shader creation facility for complex drawables.~~
- ~~Animation for player shooter, to make the next ball appear with transition.~~
- ~~Render the bubble-gameplay scene to a separate "layer" render target, to avoid clipping and manage scene position independently.~~
- ~~Separate camera settings for each "layer" render target.~~
- ~~Scenery to the test room.~~
- ~~Improve game bubbles with a "semi-circle" and textures with "nature elements as color"~~
- ~~Remove the default "createGameSphere" method from "Utility".~~
- ~~Implement audio engine.~~
- ~~Refactor code for resource creation. Move them to CommonUtility, like water, sprite, etc...~~
- ~~Orthographic layers with GUI engine.~~
- ~~Intro screen with logo and various effects, to be melt with level screen.~~
- ~~Animate camera for every scenery available in the game.~~
- ~~Game powerups.~~
- ~~Complete level cycle, with start, winning and losing.~~
- ~~Small main menu for game.~~
- ~~Implement pause/resume system for game. And so the pause menu.~~
- ~~Score system based on time and difficulty.~~
- ~~"Vote This App" and "Other Games" buttons Settings in level selector.~~
- ~~Random congratulations with graphics and sounds during bubble hits.~~
- ~~Procedural level generation with difficulty.~~
- ~~Improve level failure screen.~~
- ~~Wall 3D model to avoid seam between different world scenes in level selector.~~
- ~~Place random pickups around the map (to make use of coins).~~
- ~~Random help tooltips in level selector.~~
~~- Buy powerups in powerup scroller.~~
~~- Implement different switches for `update` and `draw` phases, to avoid re-rendering of static scenes.~~
~~- Avoid rendering for `OverlayGui` and `OverlayText` when off-screen.~~
~~- Assets update on Android based on versioning.~~
~~- Keep state on Android while app is in background (keep OpenGL ES context, work on Magnum's source).~~
~~- Fix level end check.~~
~~- Pause background music when showing ads.~~
~~- Implement save/load game.~~
~~- Implement ads with back-end management to avoid Google's shit.~~
~~- Implement push notifications to increase powerups count.~~
~~- Improve collision detection and animations due to "async" between `update` and `draw`.~~
~~- Add Onboarding for UI/UX on mobile.~~
~~- Implement projectile-swap for player, while both bubbles are "valid" (no powerups).~~