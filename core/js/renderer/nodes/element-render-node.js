var math = require("mathjs");
var TimeUtils = require("../../utils/time-utils.js");
require("./query-node-utils.js");
require("./base-query-render-node.js");

ElementRenderNode = function(source)
{
  BaseQueryRenderNode.call(this);
  this.source_ = source;
  this.controls_ = null;

  if (arguments.length > 1 && arguments[1] && typeof arguments[1] === "object")
  {
    this.controls_ = Object.assign({}, arguments[1]);
  }
}

ElementRenderNode.prototype = Object.create(BaseQueryRenderNode.prototype);
ElementRenderNode.prototype.constructor = ElementRenderNode;

ElementRenderNode.prototype.withControls_ = function(fragment)
{
  if (!this.controls_ || !(this.controls_ instanceof Object))
  {
    return fragment;
  }

  var mergedControls = Object.assign({}, this.controls_);
  if (fragment.controls && typeof fragment.controls === "object")
  {
    mergedControls = Object.assign(mergedControls, fragment.controls);
  }

  return this.makeFragment_(
    fragment.wholeStart,
    fragment.wholeEnd,
    fragment.partStart,
    fragment.partEnd,
    fragment.value,
    mergedControls
  );
}

ElementRenderNode.prototype.executeQuery_ = function(requestStart, requestEnd)
{
  if (this.source_ && this.source_.query instanceof Function)
  {
    var childFragments = this.source_.query(requestStart, requestEnd) || [];
    var self = this;
    return childFragments.map(function(fragment) {
      return self.withControls_(fragment);
    });
  }

  var cycleIndex = math.floor(requestStart);
  var localStart = TimeUtils.subtract(requestStart, cycleIndex);
  var localEnd = TimeUtils.subtract(requestEnd, cycleIndex);

  var wholeStart = TimeUtils.toFraction(0);
  var wholeEnd = TimeUtils.toFraction(1);
  var bounds = QueryNodeUtils.overlapBounds(localStart, localEnd, wholeStart, wholeEnd);
  if (!bounds)
  {
    return [];
  }

  return [this.makeFragment_(
    TimeUtils.add(wholeStart, cycleIndex),
    TimeUtils.add(wholeEnd, cycleIndex),
    TimeUtils.add(bounds.partStart, cycleIndex),
    TimeUtils.add(bounds.partEnd, cycleIndex),
    this.source_,
    this.controls_
  )];
}
