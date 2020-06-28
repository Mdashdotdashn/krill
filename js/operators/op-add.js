require("../patterns/weaving.js");


makeAddOperator = function(source, pattern)
{
  var applyFn = function(args)
  {
    var operator = function(l,r)
    {
      return parseFloat(l) + parseFloat(r);
    }
    return weavePatterns(args[0], args[1], "both", operator);
  }

  return new Operator(applyFn, [source, pattern]);
}
