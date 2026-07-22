require('../../type.js')

// Wrapper can be used to wrap fixed values

var ValueWrapperRenderNode = function(value)
{
  this.value_ = value;
}

ValueWrapperRenderNode.prototype.tick = function() {};

ValueWrapperRenderNode.prototype.render = function() { return this.value_};

// The operator framework class

RenderNode = function(renderFn, arguments, type)
{
  this.type_ = type ? type : "untyped operator";
  const makeTickable = function(a)
  {
    if (typeof a.tick === 'function')
    {
      return a;
    }
    else
    {
      return new ValueWrapperRenderNode(a);
    }
  }
  this.renderFn_ = renderFn;
  this.arguments_ = arguments.map( x => makeTickable(x));
}

RenderNode.prototype.tick = function()
{
  this.arguments_.forEach((x) => x.tick())
}

RenderNode.prototype.render = function()
{
  // render all arguments first
  var renderedArguments = this.arguments_.map((x) => x.render(x));
  var renderedSequence = this.renderFn_(renderedArguments);
//  CHECK_TYPE(renderedSequence, Sequence);
  return renderedSequence;
}

// Backward-compatible aliases while migrating call sites.
Operator = RenderNode;
ValueWrapperOperator = ValueWrapperRenderNode;

require("./add-render-node.js");
require("./bjorklund-render-node.js");
require("./slice-render-node.js");
require("./normalize-cycle-render-node.js");
require("./weighted-pattern-render-node.js");
require("./scale-render-node.js");
require("./shift-render-node.js");
require("./stack-render-node.js");
require("./stretch-render-node.js");
require("./struct-render-node.js");
require("./timeline-render-node.js");
require("./trunc-render-node.js");
