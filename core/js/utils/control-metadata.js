var NoteVelocity = require("./note-velocity.js");
var TypeGuards = require("./type-guards.js");

function cloneControls(controls)
{
  return TypeGuards.isPlainObject(controls) ? Object.assign({}, controls) : null;
}

function mergeControls(parentControls, childControls)
{
  var mergedControls = cloneControls(parentControls) || {};
  var child = cloneControls(childControls);
  if (child)
  {
    mergedControls = Object.assign(mergedControls, child);
  }
  return mergedControls;
}

function canonicalizeModelControls(controls, isNestedSource)
{
  var modelControls = cloneControls(controls);
  if (!modelControls)
  {
    return null;
  }

  if (isNestedSource && modelControls.velocity !== undefined)
  {
    var nestedVelocityFactor = NoteVelocity.resolveVelocityFactor(modelControls.velocity);
    modelControls.velocityFactor = modelControls.velocityFactor !== undefined
      ? NoteVelocity.accumulateVelocityFactor(modelControls.velocityFactor, nestedVelocityFactor)
      : nestedVelocityFactor;
    delete modelControls.velocity;
  }

  if (isNestedSource && modelControls.velocityFactor !== undefined)
  {
    modelControls.velocityFactor = NoteVelocity.resolveVelocityFactor(modelControls.velocityFactor);
  }

  return Object.keys(modelControls).length > 0 ? modelControls : null;
}

function validateNestedControls(controls)
{
  var nestedControls = cloneControls(controls);
  if (!nestedControls)
  {
    return null;
  }

  if (nestedControls.velocity !== undefined)
  {
    throw new Error("Nested ElementRenderNode controls must use velocityFactor, not velocity.");
  }

  if (nestedControls.velocityFactor !== undefined)
  {
    nestedControls.velocityFactor = NoteVelocity.resolveVelocityFactor(nestedControls.velocityFactor);
  }

  return nestedControls;
}

function composeNestedControls(parentControls, childControls)
{
  var parent = cloneControls(parentControls) || {};
  var child = cloneControls(childControls);
  var mergedControls = mergeControls(parent, child);
  var accumulatedVelocityFactor;

  function accumulateContribution(contribution)
  {
    if (contribution === undefined)
    {
      return;
    }

    accumulatedVelocityFactor = accumulatedVelocityFactor === undefined
      ? NoteVelocity.resolveVelocityFactor(contribution)
      : NoteVelocity.accumulateVelocityFactor(accumulatedVelocityFactor, contribution);
  }

  accumulateContribution(parent.velocityFactor);
  accumulateContribution(child && child.velocityFactor);

  if (accumulatedVelocityFactor !== undefined)
  {
    mergedControls.velocityFactor = accumulatedVelocityFactor;
  }

  return mergedControls;
}

function resolveVelocityFromFragment(fragment)
{
  var controls = fragment && TypeGuards.isPlainObject(fragment.controls)
    ? fragment.controls
    : null;
  var velocity = controls && controls.velocity !== undefined ? controls.velocity : undefined;
  var velocityFactor = controls && controls.velocityFactor !== undefined ? controls.velocityFactor : undefined;

  var baseVelocity = velocity !== undefined
    ? NoteVelocity.resolveVelocityToMidi(velocity)
    : NoteVelocity.DEFAULT_VELOCITY;
  var factorVelocity = velocityFactor !== undefined
    ? NoteVelocity.resolveVelocityToMidi(velocityFactor)
    : NoteVelocity.DEFAULT_VELOCITY;

  return NoteVelocity.multiplyMidiVelocities(baseVelocity, factorVelocity);
}

module.exports = {
  cloneControls: cloneControls,
  mergeControls: mergeControls,
  canonicalizeModelControls: canonicalizeModelControls,
  validateNestedControls: validateNestedControls,
  composeNestedControls: composeNestedControls,
  resolveVelocityFromFragment: resolveVelocityFromFragment
};
