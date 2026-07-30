var math = require("mathjs");

StructRenderNode = function(mask, source)
{
  this.mask_ = mask;
  this.source_ = source;
}

function isTruthyMask(value)
{
  var normalized = String(value).trim().toLowerCase();
  return normalized === "t" || normalized === "true" || normalized === "1";
}

StructRenderNode.prototype.query = function(start, end)
{
  var requestStart = math.fraction(start);
  var requestEnd = math.fraction(end);
  if (math.equal(requestStart, requestEnd))
  {
    return [];
  }

  var maskFragments = this.mask_.query(requestStart, requestEnd) || [];
  var out = [];

  maskFragments.forEach(function(maskFragment)
  {
    var slotStart = math.fraction(maskFragment.wholeStart);
    var slotEnd = math.fraction(maskFragment.wholeEnd);

    var partStart = math.max(requestStart, slotStart);
    var partEnd = math.min(requestEnd, slotEnd);
    if (!math.smaller(partStart, partEnd))
    {
      return;
    }

    if (isTruthyMask(maskFragment.value))
    {
      var sourceFragments = this.source_.query(partStart, partEnd) || [];
      sourceFragments.forEach(function(sourceFragment)
      {
        out.push({
          wholeStart: slotStart,
          wholeEnd: slotEnd,
          partStart: sourceFragment.partStart,
          partEnd: sourceFragment.partEnd,
          value: sourceFragment.value
        });
      });
      return;
    }

    out.push({
      wholeStart: slotStart,
      wholeEnd: slotEnd,
      partStart: partStart,
      partEnd: partEnd,
      value: "~"
    });
  }, this);

  return out;
}
