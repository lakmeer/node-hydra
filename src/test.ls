
# Helpers

log = -> console.log.apply console, &; &0


# Require

hydra = require './hydra'


# Quit when Q pressed

-> log 'Exit'; process.exit!

