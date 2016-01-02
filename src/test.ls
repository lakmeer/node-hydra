
# Require

hydra = require './hydra'


# Helpers

log = -> console.log.apply console, &; &0

[ DIR_NONE, DIR_LEFT, DIR_RIGHT ] = [0 to 2]

dir-to-arrow = ->
  switch it
  | DIR_NONE  => '-'
  | DIR_LEFT  => '<'
  | DIR_RIGHT => '>'

dir-from-frame = (frame) ->
  if frame.new is frame.old then DIR_NONE
  else if frame.new < frame.old then DIR_LEFT
  else DIR_RIGHT

progress-bar = (v, dir, max = 1, length = 20) ->
  full = "=" * Math.round(v/max * length);
  arr  = dir-to-arrow dir
  rest = "-" * Math.round((max - v)/max * length);
  full + arr + rest


# Register listeners

hydra.on \controllers.0.buttons.button1, (frame) ->
  log "Left Wand, Button 1:", if frame.new then "DOWN" else "UP"

hydra.on \controllers.0.trigger, (frame) ->
  log "Left Wand, Trigger:", progress-bar frame.new, dir-from-frame frame

hydra.on \controllers.1.buttons.button1, (frame) ->
  log "Right Wand, Button 1:", if frame.new then "DOWN" else "UP"

hydra.on \controllers.1.trigger, (frame) ->
  log "Right Wand, Trigger:", progress-bar frame.new, dir-from-frame frame


# Begin

hydra.init!
hydra.start!

