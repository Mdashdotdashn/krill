require ('./sequence.js')
require ('./command-evaluator.js')

Reducer = function()
{
  this.evaluator_ = new CommandEvaluator();
}

Reducer.prototype.evaluateCommand = function(name, args, sequence)
{
  return this.evaluator_[name](args, sequence);
}

// Tries to evaluate as many sequence data as possible
Reducer.prototype.reduce = function(node)
{
    // is the current node a sequence or operant ?
    switch(node.type_)
    {
      case "sequence":
        var sequence = new Sequence();
        sequence.renderArray(node);
        return sequence;
      case "command":
        return this.evaluateCommand(node.name_, node.args_, this.reduce(node.sequence_));
      default:
        throw new Exception("unkonw node type");
    }
}
