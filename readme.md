
# node-hydra

An evented interface for your Razer Hydra controller.


## Installation

1. Install [Sixense's Hyrda SDK](http://sixense.com/developers)
2. Clone this repo to node\_modules/node-hydra (npm install coming soon)
3. Check lib path in bindings.gyp in case libsixense got installed to somewhere different than me
4. Copy sixense.h from SDK bundle to src/sixense/sixense.h
5. `$ node-gyp build`
6. `$ node-gyp configure`


## Usage

``` Livescript
hydra = require 'node-hydra'

# Set up your program ...

hydra.on 'controllers.0.position.x', (frame) ->
  frame.new    # X position of left controller now
  frame.old    # X position of left controller last frame

hydra.start
```

## Evented interface

This Hydra module uses an even subscription interface, which should be familiar to Javascript developers. Events are identified by dotted.accessors.like.this. Eg:

``` Livescript
hydra.on "controllers.0.buttons.button1', (frame) ->
  ...

hydra.on "controllers.1.trigger', (frame) ->
  ...
```

Events are only dispatched when the data they are addressing actually changes compared to the previous frame.
The parameter to the event handler is an object representing the most recent data with the key `new`, and the data as of the previous frame with the key `old`.

``` Livescript
hydra.on "
```

## Available Events

The node-hydra extension computes a handful of extra values at the C++ level before passing the data to node. On top of the data made available by the normal interface to the Sixense SDK, node-hydra also provides timing measurements, the separation between the two controllers, joystick position in a variety of formats, and both named button references and the bitmask of all the button states. Here is a full list of the events and the names by which they may be addressed.  


#### Global (both controllers)

  - `sequenceNum` - sixense sequence number (see sixense docs)
  - `elapsedTime` - ms since last frame was delivered
  - `update` - All the data, every frame.
  - `seperation.[0-3]` - Distance between both controllers in hyrda-units (x, y, z, abs)


#### Per Controller

Subscribe to controller events with string in the form `controllers.[0,1].<event-name>`, for example: `hydra.on "controllers.1.joystick.x"`

  - `controllers.[0,1].position.[0-2]` - Position 3-vector (x, y, z)
  - `controllers.[0,1].rotation.[0-3]` - Rotation quaternion
  - `controllers.[0,1].joystick.[x,y]` - Joystick cartesian values (x, y)
  - `controllers.[0,1].joystick.[r,θ]` - Joystick polar values (magnitude, angle (clockwise in radians from top))
  - `controllers.[0,1].joystick.bearing` - Joystick bearing, like a compass
  - `controllers.[0,1].joystick.quadrant` - Joystick quadrant, 0 for dead, then _int_ clockwise starting at 1 in NE (top-right)
  - `controllers.[0,1].joystick.octant` - Joystick octant, 0 for dead, then _int_ clockwise starting at 1 in N (top/front)
  - `controllers.[0,1].trigger` - trigger analog value, from 0 - 1
  - `controllers.[0,1].buttons.button1` - Numbered buttons state (boolean)
  - `controllers.[0,1].buttons.button2` - Numbered buttons state (boolean)
  - `controllers.[0,1].buttons.button3` - Numbered buttons state (boolean)
  - `controllers.[0,1].buttons.button4` - Numbered buttons state (boolean)
  - `controllers.[0,1].buttons.home` - That little black button in the middle
  - `controllers.[0,1].buttons.bumper` - Shoulder button
  - `controllers.[0,1].buttons.joystick` - Joystick 'click'
  - `controllers.[0,1].buttons.bitmask` - all buttons represented as a bitmask


#### Helpful Conventions

For semantic reasons, I suggest setting up some simple constants like this:

``` Livescript
[ LEFT, RIGHT ] = [ X, Y, Z, ABS ] = [ 1 to 4 ]
```

so you can do this:

``` Livescript
hydra.on "controllers.#LEFT.position.#X", (frame) ->
  ...

hydra.on "seperation.#ABS", (frame) ->
  ...
```

Also, it means you can swap `LEFT` and `RIGHT` if you have an initialisation routine for the Hydra which reveals that the user has picked up the controller handles round the other way.

