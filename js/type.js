mathjs = require("mathjs");

var JSONFraction = function(value)
{
  return "" + value.n + "/" + value.d;
}

var JSONReplacer = function(key,value)
{
  if (value.mathjs == "Fraction")
  {
    return JSONFraction(value);
  }
  if (typeof value === "object")
  {
    if (value instanceof PatternEvent)
    {
      return  JSONFraction(value.time_) + ' : [' + value.values_ + ']';
    }
  }
  return value;
}

Dump = function(o)
{
  console.log(JSON.stringify(o, JSONReplacer,1));
}

Log = function(text, o)
{
  console.log(text + ": " + JSON.stringify(o, undefined,1));
}

CHECK_TYPE = function(value, expectedType)
{
  var objectType = typeof value;
  var valid = (objectType === "object") ? (value instanceof expectedType) : (objectType === expectedType);
  if (!valid)
  {
    console.log("while checking ");
    throw TypeError("Expecting a " + expectedType + " but got " + JSON.stringify(value));
  }
  return true;
}

CloneString = function(s)
{
  return (' ' + s).slice(1);
}
