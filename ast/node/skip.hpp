

#ifndef __SKIP_H__
#define __SKIP_H__

class SkipStmtNode : public Node, public NodeAcceptor<SkipStmtNode> {
public:
  SkipStmtNode(const int &row, const int &col) : Node(row, col) {}
  string getHead() const { return head_; }

  void toCCode_(ostringstream &o) {
  }
  void toString_(ostringstream &o) {
  }
  Json toEsprima_() override {
    Json::object obj{};
    return obj;
  }
  void toJSON_(ostringstream &o) {  }

  bool hasChildren() const override {
    return false;
  }
  vector<shared_ptr<Node> > getChildren() override {
    return vector<shared_ptr<Node> >{};
  }
  void traverse(ASTVisitor * visitor) override {
      accept(visitor);
  }
  bool isSkip() const override {
  	return true;
  }
private:
  string head_ = "Skip";
};
#endif /* __SKIP_H__ */