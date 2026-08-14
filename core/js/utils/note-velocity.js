var MIN_VELOCITY = 0;
var MAX_VELOCITY = 127;
var DEFAULT_VELOCITY = 127;

function clampMidiVelocity(value)
{
  return Math.max(MIN_VELOCITY, Math.min(MAX_VELOCITY, value));
}

function resolveVelocityToMidi(value)
{
  var numeric = Number(value);
  if (!isFinite(numeric))
  {
    return DEFAULT_VELOCITY;
  }

  if (numeric >= 0 && numeric <= 1)
  {
    return clampMidiVelocity(Math.round(DEFAULT_VELOCITY * numeric));
  }

  return clampMidiVelocity(Math.round(Math.abs(numeric)));
}

function multiplyMidiVelocities(left, right)
{
  var combined = Math.round(
    (resolveVelocityToMidi(left) * resolveVelocityToMidi(right)) / DEFAULT_VELOCITY
  );
  return clampMidiVelocity(combined);
}

// Render-tree composition helper: accumulate parent/child velocity contributions.
function accumulateVelocityFactor(accumulatedFactor, nextContribution)
{
  return multiplyMidiVelocities(accumulatedFactor, nextContribution);
}

module.exports = {
  MIN_VELOCITY: MIN_VELOCITY,
  MAX_VELOCITY: MAX_VELOCITY,
  DEFAULT_VELOCITY: DEFAULT_VELOCITY,
  clampMidiVelocity: clampMidiVelocity,
  resolveVelocityToMidi: resolveVelocityToMidi,
  multiplyMidiVelocities: multiplyMidiVelocities,
  accumulateVelocityFactor: accumulateVelocityFactor
};