
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
