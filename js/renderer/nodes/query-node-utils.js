var math = require("mathjs");

QueryNodeUtils = {
  toFraction: function(value)
  {
    return math.fraction(value);
  },

  hasNoWidth: function(start, end)
  {
    return math.equal(start, end);
  },

  overlapBounds: function(requestStart, requestEnd, wholeStart, wholeEnd)
  {
    if (math.smallerEq(requestEnd, wholeStart) || math.largerEq(requestStart, wholeEnd))
    {
      return null;
    }

    return {
      partStart: math.larger(requestStart, wholeStart) ? requestStart : wholeStart,
      partEnd: math.smaller(requestEnd, wholeEnd) ? requestEnd : wholeEnd
    };
  }
};
