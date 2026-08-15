function isObjectLike(value)
{
  return value !== null && typeof value === "object";
}

function isPlainObject(value)
{
  if (!isObjectLike(value))
  {
    return false;
  }

  var proto = Object.getPrototypeOf(value);
  return proto === Object.prototype || proto === null;
}

module.exports = {
  isObjectLike: isObjectLike,
  isPlainObject: isPlainObject
};