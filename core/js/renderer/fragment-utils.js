var TimeUtils = require("../utils/time-utils.js");
var TypeGuards = require("../utils/type-guards.js");

function createFragment(wholeStart, wholeEnd, partStart, partEnd, value, controls)
{
  var fragment = {
    wholeStart: TimeUtils.toFraction(wholeStart),
    wholeEnd: TimeUtils.toFraction(wholeEnd),
    partStart: TimeUtils.toFraction(partStart),
    partEnd: TimeUtils.toFraction(partEnd),
    value: String(value)
  };

  if (TypeGuards.isPlainObject(controls))
  {
    fragment.controls = Object.assign({}, controls);
  }

  return fragment;
}

function remapFragmentTiming(fragment, wholeStart, wholeEnd, partStart, partEnd)
{
  return createFragment(
    wholeStart,
    wholeEnd,
    partStart,
    partEnd,
    fragment.value,
    fragment.controls
  );
}

module.exports = {
  createFragment: createFragment,
  remapFragmentTiming: remapFragmentTiming
};
