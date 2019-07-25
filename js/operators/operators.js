require('../type.js')

Operator = function(renderFn, arguments)
{
  this.renderFn_ = renderFn;
  this.arguments_ = arguments;
}

Operator.prototype.tick = function()
{
  this.arguments_.forEach((x) => x.tick())
}

Operator.prototype.render = function()
{
  // render all arguments first
  var renderedArguments = this.arguments_.map((x) => x.render(x));
  var renderedSequence = this.renderFn_(renderedArguments);
//  CHECK_TYPE(renderedSequence, Sequence);
  return renderedSequence;
}

// Wrapper can be used to wrap fixed values

var ValueWrapperOperator = function(value)
{
  this.value_ = value;
}

ValueWrapperOperator.prototype.tick = function() {};

ValueWrapperOperator.prototype.render = function() { return this.value_};

makeValueWrapperOperator = function(v)
{
  return new ValueWrapperOperator(v);
}

require("./op-bjorklund.js");
require("./op-pattern.js");
require("./op-scale.js");
require("./op-slow.js");
require("./op-struct.js");
require("./op-target.js");
require("./op-timeline.js");
