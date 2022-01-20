# Project 05 Roadmap

## Initial outline
### Body `void *info` field
- body struct has a `info` field, but demo defines what goes there.
### Scene upgrades
- Deferred action (to end of `scene_tick`)
- body removal
    - set flag in `forces_auxs` of `scene`
    - remove body
    - remove force creators involving body
### Collision Detection
### Demo: Space Invaders
- Define `info` struct
    - Differentiate between body classes - use `enum`.



## Plan
### Monday
- scene + body `info` [pair: noah + halle]
- collision detection + demo setup [pair: gabe + alex]
### Tuesday, Wednesday 7p pst
- put together [group]

## Subproject Roadmaps
### `body` upgrades
- Port forward previous weeks' body interface.
- Implement `body_remove` and `body_is_removed`.
    - Add field to struct.
    - Update `body_init`.
    - Figure out how this interacts with `forces`.
- Add `void *info` field.
    - Update struct.
    - Update `body_init`.
    - Add `body_init_with_info`.
    - Update `body_free`.
- Other group
    - Let other group know that demo needs a `free_func_t info_freer`.
    - Let them know that bodies is listed both in scene's force_aux and force's
      aux.


## Remaining TODO
- [ ] spaceinvaders
    - [ ] finish enemy wrapping^
    - actually not a thing this week [X] spawn new ememies
    - [X] end game, avoid segv when player dies :)
- [x] get old demos working
    - [x] add deprecated acceleration back into body after impluse fix.
    - [x] damping
    - [x] gravity
    - [x] pacman
- [ ] fix Makefile
    - [ ] we need `make test` to work, either remove test targets or port
          forward tests from previous projects.

^NOTE: Gabe's wrapping code has been commented out on `develop-spaceinvaders`
branch but not `si_wrap_wall_collision`, for testing purposes.
