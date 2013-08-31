
#
# Timer
#
# Encapsulates the concept of measuring
# how long since we last did something.
#


# Timer generator

timer = ->
  before = Date.now!
  return ->
    now = Date.now!
    Δ = now - before
    before := now
    return Δ


# Export

module.exports = timer
