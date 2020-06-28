require('../type.js')

// Wrapper can be used to wrap fixed values

var ValueWrapperOperator = function(value)
{
  this.value_ = value;
}

ValueWrapperOperator.prototype.tick = function() {};

ValueWrapperOperator.prototype.render = function() { return this.value_};

// The operator framework class

Operator = function(renderFn, arguments)
{
  const makeTickable = function(a)
  {
    if (typeof a.tick === 'function')
    {
      return a;
    }
    else
    {
      return new ValueWrapperOperator(a);
    }
  }
  this.renderFn_ = renderFn;
  this.arguments_ = arguments.map( x => makeTickable(x));
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

require("./op-add.js");
require("./op-bjorklund.js");
require("./op-pattern.js");
require("./op-scale.js");
require("./op-shift.js");
require("./op-slice.js");
require("./op-stack.js");
require("./op-stretch.js");
require("./op-struct.js");
require("./op-timeline.js");
