#
# Hydra.ls
#
# Wrapper module for hydra-node extenstion.
# Provides flexible, evented interface to the Hydra controller with a subscription model.
#


#
# Setup
#

# Helpers

log    = -> console.log.apply console, &; &0
flip   = (λ) -> (a, b) -> λ b, a
every  = flip set-interval
remove = (ix, list) -> [ x for x, i in list when i isnt ix ]

weighted-average = require \./weighted-average
limited-array    = require \./limited-array
timer            = require \./timer
leaf-diff        = require \./leaf-diff

Hydra            = require \../build/Release/hydra.node


# Weighted average based on half of a Savitzky-Golay 9-step window

smooth = weighted-average [ -21 14 39 54 59 ]


# Local flags and variables

DEFAULT_POLLING_RATE = 8ms

last-frame = null


#
# Start Hydra and Schedule Auto-stop
#

init = ->

  log 'Hydra::Init - connecting to Hydra...'
  Hydra.init!

  process.on \exit, ->
    clear-interval update-timer
    Hydra.exit!

  last-frame := Hydra.update!

  log 'Hydra::Init - ready.'


#
# Event Dispatch
#

# Async event loop to process changes to controller output

subscribers = {}

subscribe = (event-name, λ) ->
  log 'Hydra::On - subscribing to:', event-name
  if subscribers[ event-name ]?
    that.push λ
  else
    subscribers[ event-name ] = [ λ ]
  return subscribers[ event-name ].length - 1

unsubscribe = (event-name, index) ->
  subscribers[ event-name ] = remove index, subscribers[ event-name ]


#
# Derivatives and Smoothing
#

derivator = (scale-factor = 1) ->

  time-since = timer!
  prev-vec   = []

  raw-history = [
    limited-array 5
    limited-array 5
    limited-array 5
  ]

  proc-vector = (vector) ->
    elapsed = time-since!
    for v, ix in vector
      Δ = ( v - prev-vec[ix] ) / elapsed
      raw-history[ix].push Δ
      scale-factor * smooth raw-history[ix]

  return (vector) ->
    smoothed-vector = proc-vector vector
    prev-vec := vector
    return smoothed-vector


# Each derivator keeps track of its own time, so we need
# one for each controller or they'll confuse each other

# compute-vel  = [ (derivator 150), (derivator 150) ]
# compute-acc  = [ (derivator 150), (derivator 150) ]
# compute-jerk = [ (derivator 150), (derivator 150) ]


#
# Frame Tick
#

on-poll = ->

  # Collect new data
  this-frame = Hydra.update!

  # Include derivatives
  # for controller, ix in this-frame.controllers
    #controller.velocity     = compute-vel[ix] controller.position
    #controller.acceleration = compute-acc[ix] controller.velocity
    #controller.jerk         = compute-jerk[ix] controller.acceleration

  # Take difference bewteen last two frames
  diffs = leaf-diff last-frame, this-frame

  # Dispatch auto event
  for diff in diffs
    if subscribers[ diff.key ]?
      for λ in that
        λ { old: diff.old, new: diff.new }

  # Dispatch special events
  if subscribers.update?
    for λ in that
      λ old: last-frame, new: this-frame, diffs: diffs

  # Dispatch diff list
  # if subscribers.diff?
  #   for λ in that
  #     λ diffs

  # Save frame
  last-frame := this-frame


#
# Polling Timer
#

polling-rate = DEFAULT_POLLING_RATE
update-timer = 0

start-polling = ->
  clear-interval update-timer
  log "Hydra::Start - polling at #{ 1000 / polling-rate }Hz"
  update-timer := every polling-rate, on-poll

stop-polling  = ->
  log "Hydra::Stop - polling halted"
  clear-interval update-timer


# Publish public interface

module.exports =
  init  : init
  start : start-polling
  stop  : stop-polling

  on : (event-name, λ) -> subscribe event-name, λ

  set-polling-rate : ->
    polling-rate := it
    start-polling!

