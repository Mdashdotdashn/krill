
var TimelineOperator = function(operatorArray)
{
  this.nodes_ = operatorArray;
  this.current_ = 0;
}

TimelineOperator.prototype.tick = function()
{
  this.current_ = (this.current_ + 1) % this.nodes_.length;
  this.nodes_.forEach((x) => x.tick());
}

TimelineOperator.prototype.render = function()
{
  return this.nodes_[this.current_].render();
}

makeTimelineOperator = function(operatorArray)
{
  return new TimelineOperator(operatorArray);
}
