math = require("mathjs");

Sequence = function(sequenceArray)
{
  this.length_ = 1;
  this.sequence_ = this.renderArray(sequenceArray , this.length_, 0);
}

Sequence.prototype.renderArray = function(sequenceArray, timeScale, timeOffset)
{
  var division = timeScale/(sequenceArray.length);
  return sequenceArray.map(function(x, index) {
    var position = timeOffset + division * index;
    if (Array.isArray(x))
    {
      return this.renderArray(x, division, position);
    }
    else
    {
      const fraction = math.fraction(position);
      return { time: ""+fraction.n+"/"+fraction.d, value: x };
    }
  }, this)
}
