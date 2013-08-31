
# LeafDiff
#
# Walk two copies of a structurally identical object,
# compile a flat list of the nodes where the values
# differ. Flattened results are in the form
#
#   key : "dotted.accessor.chain"
#   new : "'A' value"
#   old : "'B' value"
#

module.exports = leaf-diff = (a, b, chain-name = '', diffs = []) ->

  # Cue structure off A, as they should be identical

  if typeof a is \object and a  # (isn't null)
    for k, va of a
      vb = b[k]
      leaf-diff va, b[k], chain-name + "." + k, diffs

  else
    if a isnt b
      diffs.push {
        key : chain-name.replace(/^\./, '')
        old : a
        new : b
      }

  return diffs

