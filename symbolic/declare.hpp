
#ifndef __DECLARE_H__
#define __DECLARE_H__

class DeclareNode : public Node {
public:
  DeclareNode(const int &row, const int &col) : Node(row, col) {}
  void setIdentifier(const shared_ptr<IdentifierNode> &id) { id_ = id; }
  void setType(const shared_ptr<TypeNode> &typ) { typ_ = typ; }
  void setInitializer(const shared_ptr<Node> &init) { init_ = init; }
  shared_ptr<Node> getIdentifier() const { return id_; }
  shared_ptr<Node> getType() const { return typ_; }
  shared_ptr<Node> getInitializer() const { return init_; }
  bool hasInitializer() const { return init_ != nullptr; }
  string getHead() const { return head_; }

  void toCCode_(ostringstream &o) {
    assert(typ_ != nullptr);
    assert(id_ != nullptr);
    typ_->toCCode_(o);
    o << " ";
    id_->toCCode_(o);
    if (hasInitializer()) {
      o << " = ";
      init_->toCCode_(o);
    }
  }
  void toString_(ostringstream &o) {

    assert(typ_ != nullptr);
    assert(id_ != nullptr);
    typ_->toString_(o);
    o << " ";
    id_->toString_(o);
    if (hasInitializer()) {
      o << " = ";
      init_->toString_(o);
    }
  }
  void toJSON_(ostringstream &o) { o << "{\"type\": \"unknown\"}"; }
  bool isStatement() const { return true; }

private:
  string head_ = "Declare";
  shared_ptr<IdentifierNode> id_ = nullptr;
  shared_ptr<TypeNode> typ_ = nullptr;
  shared_ptr<Node> init_ = nullptr;
};

#endif /* __IDENTIFIER_H__ */
