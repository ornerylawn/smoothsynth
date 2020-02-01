#ifndef NODE_H_
#define NODE_H_

// A Node computes outputs from inputs. It is responsible for owning
// its outputs. Each input and output is an Optional, so that a Node
// can tell when its inputs are available and can communicate when its
// outputs are ready.
//
// The general use case is when there is a graph of nodes. The pattern
// is to make all of the outputs nil, then iteratively compute the
// nodes that have their inputs available until all of the nodes have
// been computed.
//
//   List<Node*> todo;
//   for (auto node : nodes) {
//     node->MakeOutputsNil();
//     todo.push(node);
//   }
//
//   while (!todo.empty()) {
//     for (iter = todo.begin(); iter != todo.end(); ) {
//       Node* node = *iter;
//       if (node->inputs_available()) {
//         node->Compute();
//         iter = todo.erase(iter);
//       } else {
//         ++iter;
//       }
//     }
//   }
class Node {
public:
	virtual ~Node() {}
	virtual void MakeOutputsNil() = 0;
	virtual bool inputs_available() const = 0;
	virtual void Compute(int frame_count) = 0;
};

#endif  // NODE_H_
