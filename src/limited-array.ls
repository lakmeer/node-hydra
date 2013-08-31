
#
# Limited Array
#
# Push method knocks the the first value from the front of the array. Also
# supports 'last' getter to cut down on that x[x.length - 1] nonsense.
#

module.exports = (n, s=0) ->
  Object.define-properties [ s ] * n, {
    push :
      value : -> Array::push.apply this, &; @shift!; this
      writable: no
    last :
      get : -> this[@length - 1]
      enumerable: no
  }

