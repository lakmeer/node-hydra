
# Helpers

lim   = (upper, x) -> if x > upper then upper else x
log   = -> console.log.apply console, &; &0
delay = (t, λ) -> set-timeout λ, t
every = (t, λ) -> set-interval λ, t


# Prelude

global <<<  require 'prelude-ls'


# Require

hydra = require './hydra'


# Quit when Q pressed

onkey.on 'q', -> log 'Exit'; process.exit!

