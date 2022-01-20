**Notice: This roadmap is entirely out of date and kept for posterity only.**


ROADMAP
=======

TODO: Global include header that has every struct so that we don't have to redeclare
in every other header.

OUTLINE N.0
-----------
### player
#### TODO
- [ ] add score
- [ ] add player to its body as info field
- remove
- speed (angular)
#### fields
- hippo_body
- state (enum)
- list_t powerups
#### methods
- set_state // handles switching sprites and polygons

### game
#### methods
- game_tick

### body
- Body should be initializable with a gfx_aux or just a shape, sdl is
  responsible for figuringn out which to render.
#### fields
- gfx_aux
- removed
- info
- shapes // Indices should be enumerated, correspond to gfx indices if possible.
- shape_idx // Active sprite.
#### methods
- init
- free
- set_centroid
- get_gfx

### gfx
#### sprite
- Sprites are initialized at the beginning of the game, packaged into gfx_aux,
  and then passed to bodies.
##### methods
- render
- set_rotation
- set_dims
- scale_dims
#### gfx_aux
- Graphic auxiliaries track all the sprites associated with a body, and provide
  an interface for rendering the currently selected sprite and switching between
  sprites.
- Sprites are addressable via index. This means that clients should define their
  own enumerations of sprite indices. For instance, hippos could have
  EATING_SPRITE, CHILLING_SPRITE index enumeration.
##### methods
- render(sdl stuff)
- set_sprite(size_t idx) // we should enumerate indices
- init
- free
##### fields
- body
- sprites
- rects

### sdl_wrapper
#### NOTES
- If body has no sprite, then render shape.
#### methods
- sdl_render_game
- sdl_render_scene
- sdl_render_body

### ball (ball and powerup)
#### NOTES:
- balls' sprites can be "shrunk down" within their collision shapes
- ball with only on eat effect - not have a white border
- init a power up ball, spawn
    - create a collision handler between ball and all hippos
        - says when a hippo eats a ball, call on eat
- hippo-ball collisions
    - hippo-mouth
    - hippo-body
    - two bodies?
    - or just one mouth body?
    - ball - hippo body (not mouth)
#### consts
- enum of effect types
    - (separate into enums for on eat and on activate?)
#### structs
- powerup
    - on_eat
    - on_activate
        - e.g. on_activate = spawn_x(aux), on_activate_aux = {.scene=, ...}
#### methods
- spawn_ball(ball_type_e ball_type)
- spawn

### scene

### dyn_func
Probably not needed.

### hungryhippos
#### methods
- spawn_powerups -wrapper> spawn_ball
- spawn_powerup_x
    - init ball
    - choose sprite(s)

TODO
----
- [x] test multiple textures on same rendering
- [x] how do they overlap
- [x] do multiple simultaneous key events actually work?
- [x] rect in body?
- [x] fancy graphics like foliage, background? (no time)
- [x] layering via multiple scenes?
- [x] using ampersand in `SDL_Query`
- [x] how to specify type (string vs enum)
- [x] best way to implement subclasses
- [x] clarify name of game type, noah proposese `game_t` for simplicity (and not
      `world_t`, bc we already sometimes use world as a synonym for scene).
- [x] comments, prototypes, includes

- [x] interfaces
    - [x] sprite
    - [x] gfx_aux
    - [x] body
    - [x] sdl_wrapper
- [x] how should position of body and gfx be synced?
- [x] when a body has multiple shapes, those shapes should not all be translated
      each tick. rather, the current shape should be translated, and when the
      shape is switched, it should be moved automatically to the body's new
      location. update: nevermind, since there is no longer so much a notion of
      "currnet shape", all shapes need to be ticked each tick.
- [x] merge into develop-gfx branch
- [x] update interfaces
    - [x] sprite
    - [x] gfx_aux
    - [x] body
- [x] update implementations
    - [x] sprite, render_sprite
    - [x] gfx
    - [x] body
- [x] sdl_wrapper
- [x] gfx init from path
- [x] body init from paths
- [x] revert body_get_shape breaking change, implement alt func
- [x] sdl_render_game
- [x] shape offset

- [ ] integrate
    - [ ] multiple hippos
    - [ ] key handling
    - [ ] powerups/eating

- [x] static: create shape csvs and cropped sprite transparencies
    - [x] hippo head
- [x] fix sprite offset
- [x] fix sprite import scaling
- [ ] fix shape vs sprite scaling with window
- [x] QUESTION@dnee: memory leaks from SDL?

#### Week 3
- [X] spawning [Alex]
    - [X] powerup balls
    - [X] regular balls different method
    - [X] method (like what it looks like)
- [x] IMPORTANT: body don't copy shape [Noah]
- [X] more hippos [Alex]
    - [X] fix key-handling issues with multiple hippos - initial position prob wrong
- [ ] sprites [Alex, Noah]
    - [X] make background a sprite
    - [x] hippo sprites
    - [X] png
    - [ ] -> crop+transperencise [Noah]
    - [ ] -> generate csvs [Noah]
- [ ] collisions
    - [X] collisions between balls [Noah, Alex]
    - [ ] collision between ball and hippo body [Noah, Alex]
    - [X] game boundary [Alex]
        - in spawn_ball function
- [ ] cli [Noah]
    - [x] point readout
    - [ ] powerup interface
- [ ] position forward timeout [later]
- [X] powerup/ball
    - [X] test/implement remaining powerups [Gabe]
    - [X] powerup for speed [Gabe]
    - [X] shooting
    - [X] powerup timeout
- [ ] gameflow
    - [X] ball limited [Alex]
    - [ ] single round [Halle]
    - [ ] multiple rounds


### FINAL THINGS:
- [ ] collisions
    - [X] arena and balls - Alex tonight
    - [X] collision between balls - less important but still should have - Alex, maybe others
    - [ ] collision between hippos and balls - eating vs physics colliding - Noah
- [x] text read out (maybe) - Noah
- [ ] command line interface - Noah
- [X] rounds? - Alex mainly, Halle
    - [X] if not, at least a win condition - Alex
        - [X] checking for a win - Alex
        - [ ] text box - Noah
    - [ ] fix carry-over of points, key-handling (??), and indexing of players so you can get the right sprites
- [X] position forward timeout - quick only, Gabe
- [ ] new sprites? only if lots of time - Noah, weekend, if time
- [ ] go through and look at TODOs everywhere, add comments mainly and remove printfs - Alex
- [X] fix key-handling to switch through powerups
- [ ] format files - Noah
- [ ] make a README with instructions for how to play the game - TBD, whomever has time
- [ ] reduce lag if possible
    - [ ] decrease max number balls
    - [ ] sdl preformance flags
