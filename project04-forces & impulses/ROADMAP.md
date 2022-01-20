- import body and scene, vector, list, polygon, ...

- implement impulse, but don't use it in the demos this week because no collisions modeled

- body has changes in add_force and add_impulse
    - body tick handles motion after receiving forces and impulses
    - forces and impulses get sent to body from scene
    - impulse = collision, avoid deformation of objects
    - body struct:
        - force vector_t net_force on this tick
        - impulse vector_t net_impulse on this tick

- scene:
    - for loop over all the bodies to get pairs... slow, update at end supposedly
    - scene struct stays same
        - function pointer for force function
        - prob not collisions
        - arraylist of function pointers, each is a force function / force updater
            - should not remember specific types of forces (e.g. gravity vs spring, should be general)
            - auxillary void * keeps track of force function
            - demo defines auxillary object and force creator
            - force creator functions in a list, added by create_spring...

- force abstraction + force_creators
    - force and mass replaces (??) acceleration in body
    - possibly in addition to acceleration,
    - scene sends body the force ?

- tests
    - ones required in test_util
    - lots more in student_tests.c


- demo x n-body gravity
    - gravitational constant
        - call gravity between each pair of objects
        - calls create_Newtonian_gravity

- demo x spring with damping/drag
    - spring constant
        - call between adjacent objects
        - calls create_spring
    - damping constant
        - call on individual objects
        - calls create_drag




demo calls create_Newtonian_gravity, takes in a pointer to scene
"force creator will be called each tick"
private functions in forces.c, spring, gravity, etc private, these take in aux pointer,
    these know how to free auxillary parameter
    private functions stored in scene,
    3 lists: aux, function pointers, freer functions
scene calls add_force_creator






4 tasks:
1. force abstraction, specific functions and changes in scene, body, and forces  - GABE
2. testing
3. demo gravity
4. demo spring/damping
last 3 to be decided once we hear input from Noah

tuesday night - check in, progress updates, anything we need to change 7 pm PST (quick)
wednesday night  - real check in, everybody aims to be done with their parts, pulling it
                    all together, Noah we're volunteering you to run this to check for memory
                    leaks last on wednesday, time TBD but longer meeting



# Tests
- Gravity: projectile motion.
- Equilibrium in both elastic and gravitational field.
- Drag: Terminal velocity.
