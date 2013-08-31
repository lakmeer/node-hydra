
#
# Weighted Average
#
# Impulse filter like you'd use to do moving
# mean or other kinds of on-the-fly smoothing.
#
# Includes default weightings based Savitzky-Golay
# numbers.
#


# Averager generator
#
# Takes list of coeffients, creates function
# which expects a list of data points whose
# length is the same as the original list of
# coefficients.
#

{ recip, sum, zip-with } = require \prelude-ls

weighted-average = (coeff) ->
  r = recip sum coeff
  return (points) ->
    r * sum zip-with (*), coeff, points


module.exports = weighted-average

