
#ifndef __CALL_H__
#define __CALL_H__

class CallNode : public Node, public NodeAcceptor<CallNode> {
public:
  CallNode(const int &row, const int &col, const int &endrow, const int &endcol,
           const string &raw)
      : Node(row, col, endrow, endcol, raw) {}
  CallNode(const int &row, const int &col, const int &endrow, const int &endcol,
           const string &raw, const shared_ptr<IdentifierNode> &fun,
           const vector<shared_ptr<Node>> &args)
      : Node(row, col, endrow, endcol, raw), fun_(fun),
        args_(new CompoundNode(row, col, endrow, endcol, raw, args)) {}
  ~CallNode() {}
  shared_ptr<IdentifierNode> getFunction() const { return fun_; }
  vector<shared_ptr<Node>> getArgs() const { return args_->getValues(); }
  void setFunction(const shared_ptr<IdentifierNode> &fun) { fun_ = fun; }
  void addArg(const shared_ptr<Node> &nd) {
    if (args_ == nullptr) {
      args_ = shared_ptr<CompoundNode>(
          new CompoundNode(row_, col_, endrow_, endcol_, raw_));
      args_->isArgumentList(true);
      args_->setParent(this);
    }
    args_->push_back(nd);
  }
  string getHead() const override { return head_; }
  void toCCode_(ostringstream &o) override {
    fun_->toCCode_(o);
    o << "(";
    if (args_ != nullptr) {
      args_->toCCode_(o);
    }
    o << ")";
  }
  void toString_(ostringstream &o) override {
    fun_->toString_(o);
    o << "(";
    if (args_ != nullptr) {
      args_->toString_(o);
    }
    o << ")";
  }
  Json toEsprima_() override {
    Json::object obj;
    vector<Json> args;
    obj["type"] = "CallExpression";
    obj["loc"] = getLocation();
    obj["raw"] = raw_;
    obj["cform"] = toCCode();
    obj["callee"] = fun_->toEsprima_();
    if (args_ != nullptr) {
      for (auto arg : args_->getValues()) {
        args.push_back(arg->toEsprima_());
      }
    }
    obj["arguments"] = args;
    return obj;
  }
  bool hasChildren() const override {
    return fun_ != nullptr && args_ != nullptr;
  }
  vector<shared_ptr<Node>> getChildren() override {
    if (hasChildren() == false) {
      return vector<shared_ptr<Node>>{};
    } else if (fun_ != nullptr && args_ != nullptr) {
      return vector<shared_ptr<Node>>{fun_, args_};
    } else if (fun_ == nullptr) {
      return vector<shared_ptr<Node>>{args_};
    } else {
      return vector<shared_ptr<Node>>{fun_};
    }
  }

  void traverse(ASTVisitor *visitor) override {
    accept(visitor);
    fun_->traverse(visitor);
    args_->traverse(visitor);
  }

private:
  string head_ = "Call";
  shared_ptr<IdentifierNode> fun_ = nullptr;
  shared_ptr<CompoundNode> args_ = nullptr;
};

#endif /* __CALL_H__ */
