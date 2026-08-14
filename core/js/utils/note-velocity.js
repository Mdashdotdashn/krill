var MIN_VELOCITY = 0;
var MAX_VELOCITY = 127;
var DEFAULT_VELOCITY = 127;
var MIN_FACTOR = 0;
var MAX_FACTOR = 1;

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

function resolveVelocityFactor(value)
{
  var numeric = Number(value);
  if (!isFinite(numeric) || numeric < MIN_FACTOR || numeric > MAX_FACTOR)
  {
    throw new Error("velocityFactor must be within [0, 1].");
  }

  return numeric;
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
  return resolveVelocityFactor(accumulatedFactor) * resolveVelocityFactor(nextContribution);
}

module.exports = {
  MIN_VELOCITY: MIN_VELOCITY,
  MAX_VELOCITY: MAX_VELOCITY,
  DEFAULT_VELOCITY: DEFAULT_VELOCITY,
  MIN_FACTOR: MIN_FACTOR,
  MAX_FACTOR: MAX_FACTOR,
  clampMidiVelocity: clampMidiVelocity,
  resolveVelocityToMidi: resolveVelocityToMidi,
  resolveVelocityFactor: resolveVelocityFactor,
  multiplyMidiVelocities: multiplyMidiVelocities,
  accumulateVelocityFactor: accumulateVelocityFactor
};