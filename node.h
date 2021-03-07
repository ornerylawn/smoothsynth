#ifndef NODE_H_
#define NODE_H_

// A Node computes output chunks from input chunks. It is responsible for owning
// its outputs while further nodes read from them.
//
// The general use case is when there is a graph of nodes. The pattern is to
// start with outputs unavailable, then iteratively have the nodes with
// available inputs compute their outputs, making them available. Eventually all
// of the nodes will have been computed.
class Node {
 public:
  virtual ~Node() {}

  // Returns true if all input chunks are transmitting.
  virtual bool Rx() const = 0;

  // Computes output chunks from input chunks. Make sure Rx() returns true
  // before calling.
  virtual void ComputeAndStartTx(int frame_count) = 0;

  // Makes all outputs unavailable.
  virtual void StopTx() = 0;
};

#endif  // NODE_H_
