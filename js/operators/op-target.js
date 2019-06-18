const math = require("mathjs");

makeTargetOperator = function(source, targetName)
{
  var setTargetFn = function(args)
  {
    var clone = args[0].clone();
    clone.targetName_  = args[1];
    console.log(clone.targetName_);
    return clone;
  }

  return new Operator(setTargetFn, [source, makeValueWrapperOperator(targetName)]);
}
