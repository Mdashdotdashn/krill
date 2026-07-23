ScaleRenderNode = function(scaleName, source)
{
  this.scaleName_ = String(scaleName || "").toLowerCase();
  this.source_ = source;
  this.intervals_ = this.resolveIntervals_(this.scaleName_);
}

ScaleRenderNode.prototype.resolveIntervals_ = function(name)
{
  if (name === "major" || name === "ionian")
  {
    return [0, 2, 4, 5, 7, 9, 11];
  }
  if (name === "minor" || name === "aeolian")
  {
    return [0, 2, 3, 5, 7, 8, 10];
  }
  return null;
}

ScaleRenderNode.prototype.parseIntStrict_ = function(value)
{
  var text = String(value).trim();
  if (!/^[-+]?\d+$/.test(text))
  {
    return null;
  }
  return parseInt(text, 10);
}

ScaleRenderNode.prototype.floorDiv_ = function(a, b)
{
  return Math.floor(a / b);
}

ScaleRenderNode.prototype.positiveMod_ = function(a, b)
{
  var m = a % b;
  return m < 0 ? m + b : m;
}

ScaleRenderNode.prototype.mapValue_ = function(value)
{
  if (!this.intervals_ || !this.intervals_.length)
  {
    return String(value);
  }

  var degree = this.parseIntStrict_(value);
  if (degree === null)
  {
    return String(value);
  }

  var size = this.intervals_.length;
  var octave = this.floorDiv_(degree, size);
  var idx = this.positiveMod_(degree, size);
  var semitone = octave * 12 + this.intervals_[idx];
  return String(semitone);
}

ScaleRenderNode.prototype.query = function(start, end)
{
  var sourceFragments = this.source_.query(start, end) || [];
  return sourceFragments.map(function(fragment)
  {
    return {
      wholeStart: fragment.wholeStart,
      wholeEnd: fragment.wholeEnd,
      partStart: fragment.partStart,
      partEnd: fragment.partEnd,
      value: this.mapValue_(fragment.value)
    };
  }, this);
}
