- review precision error from last week

- Implement collision resolution - one pair
    Update find_collision() to return the direction along which the bodies are colliding
    Write a force_creator_t that applies equal and opposite impulses to each body along the collision direction

    Noah and Gabe: all of collision stuff + forces update

- breakout demo - one pair
    There must be several rows of bricks that are rainbow colored
    The ball must bounce off the paddle, the walls, the bricks (as well as destroy the bricks)
    The game should reset when the player loses
    You must include a “special feature” of your choice (power-ups, health on the bricks, etc.)
        - level up by getting smaller and smaller player
        - health bricks (count number of collisions)
        - lives of paddle (earn "lives" based on how far you are in the game)
        - speed of paddle influences speed of ball
        - different levels (num rows, size bricks, health/lives, etc)
        - spin dynamics
    Additionally, we have provided you with a “pegs” demo which must work correctly with your code.

    All: special feature
    Alex and Halle: demo set up

# DONE
- two folks [alex + noah]
    - [x] centre brick layers
    - [x] enforce paddle's boundary
        - custom force creator, takes one body
- two folks [gabe + halle]
    - [x] special feature
        - try spin, fallback to health if needed.
        - how to test?
- [x] final check for TO-DOs
- [-] fix defines to constants
