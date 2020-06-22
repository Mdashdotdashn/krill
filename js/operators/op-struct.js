require("../patterns/weaving.js");

var boolValue = function(b)
{
  switch (b.toLowerCase())
  {
    case "0":
    case "false":
    case "f":
    case "no":
    case "~":
      return false;
  }
  return true;
}

makeStructOperator = function(source, pattern)
{
  var applyStructFn = function(args)
  {
    var operator = function(l,r)
    {
      return boolValue(r) ? l : "~";
    }

    return weavePatterns(args[0], args[1], "right", operator);
  }

  return new Operator(applyStructFn, [source, pattern]);
}
