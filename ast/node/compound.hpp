

#ifndef __COMPOUND_H__
#define __COMPOUND_H__

#include "utilities.hpp"

class CompoundNode : public Node, public NodeAcceptor<CompoundNode> {
public:
  CompoundNode(const int &row, const int &col, const int &endrow,
               const int &endcol, const string &raw)
      : Node(row, col, endrow, endcol, raw) {
    vals_ = vector<shared_ptr<Node>>();
    isCompound(true);
  }
  CompoundNode(const int &row, const int &col, const int &endrow,
               const int &endcol, const string &raw,
               const vector<shared_ptr<Node>> &vals)
      : Node(row, col, endrow, endcol, raw), vals_(vals) {
    isCompound(true);
  }
  virtual ~CompoundNode() { vals_.clear(); }
  virtual bool isEmpty() const { return getArgCount() == 0; }
  CompoundNode *operator<<=(const bool &val) override {
    shared_ptr<BooleanNode> var(new BooleanNode(row_, col_, endrow_, endcol_,
                                                val ? "true" : "false", val));
    var->setParent(this);
    addChild(var);
    vals_.push_back(var);
    if (isBlock() || isProgram()) {
      var->isStatement(true);
    }
    return this;
  }
  CompoundNode &operator<<=(const int &val) override {
    shared_ptr<IntegerNode> var(new IntegerNode(row_, col_, endrow_, endcol_,
                                                convertToString(val), val));
    var->setParent(this);
    addChild(var);
    vals_.push_back(var);
    if (isBlock() || isProgram()) {
      var->isStatement(true);
    }
    return *this;
  }
  CompoundNode &operator<<=(const int64_t &val) override {
    shared_ptr<IntegerNode> var(new IntegerNode(row_, col_, endrow_, endcol_,
                                                convertToString(val), val));
    var->setParent(this);
    addChild(var);
    vals_.push_back(var);
    if (isBlock() || isProgram()) {
      var->isStatement(true);
    }
    return *this;
  }
  CompoundNode &operator<<=(const float &val) override {
    shared_ptr<RealNode> var(
        new RealNode(row_, col_, endrow_, endcol_, convertToString(val), val));
    var->setParent(this);
    addChild(var);
    vals_.push_back(var);
    if (isBlock() || isProgram()) {
      var->isStatement(true);
    }
    return *this;
  }
  CompoundNode &operator<<=(const double &val) override {
    shared_ptr<RealNode> var(
        new RealNode(row_, col_, endrow_, endcol_, convertToString(val), val));
    var->setParent(this);
    addChild(var);
    vals_.push_back(var);
    if (isBlock() || isProgram()) {
      var->isStatement(true);
    }
    return *this;
  }
  CompoundNode &operator<<=(const char *val) override {
    shared_ptr<StringNode> var(new StringNode(row_, col_, endrow_, endcol_,
                                              convertToString(val), val));
    var->setParent(this);
    addChild(var);
    vals_.push_back(var);
    if (isBlock() || isProgram()) {
      var->isStatement(true);
    }
    return *this;
  }
  CompoundNode &operator<<=(const string &val) override {
    shared_ptr<StringNode> var(new StringNode(row_, col_, endrow_, endcol_,
                                              convertToString(val), val));
    var->setParent(this);
    addChild(var);
    vals_.push_back(var);
    if (isBlock() || isProgram()) {
      var->isStatement(true);
    }
    return *this;
  }
  CompoundNode &operator<<=(const shared_ptr<Node> &c) override {
    if (!vals_.empty() && vals_.back() == c) {
      return *this;
    }
    if (c->isCompound()) {
      *this <<= std::static_pointer_cast<CompoundNode>(c);
    } else {
      c->setParent(this);
      addChild(c);
      vals_.push_back(c);
      if (isBlock() || isProgram()) {
        c->isStatement(true);
      }
    }
    return *this;
  }
  CompoundNode &operator<<=(CompoundNode *c) {

    vector<shared_ptr<Node>> vals = c->getValues();
    for (auto iter = vals.begin(); iter != vals.end(); iter++) {
      vals_.push_back(*iter);
      if (isBlock() || isProgram()) {
        (*iter)->isStatement(true);
      }
    }
    return *this;
  }
  CompoundNode &operator<<=(const shared_ptr<CompoundNode> &c) {
    if (!vals_.empty() && vals_.back() == c) {
      return *this;
    }
    vector<shared_ptr<Node>> vals = c->getValues();
    for (auto iter = vals.begin(); iter != vals.end(); iter++) {
      vals_.push_back(*iter);
      if (isBlock() || isProgram()) {
        (*iter)->isStatement(true);
      }
    }
    return *this;
  }
  bool isListInitialization(bool val) {
    isListInitialization_ = val;
    return isListInitialization_;
  }
  bool isListInitialization() { return isListInitialization_; }
  void push_back(Node *v) {
    shared_ptr<Node> var(v);
    var->setParent(this);
    addChild(var);
    vals_.push_back(var);
    if (isBlock()) {
      var->isStatement(true);
    }
    return;
  }
  void push_back(const shared_ptr<Node> &var) {
    if (!vals_.empty() && vals_.back() == var) {
      return;
    }
    var->setParent(this);
    addChild(var);
    vals_.push_back(var);
    if (isBlock()) {
      var->isStatement(true);
    }
    return;
  }
  void push_back(const vector<shared_ptr<Node>> &var) {
    for (auto iter : var) {
      iter->setParent(this);
      addChild(iter);
      push_back(iter);
      if (isBlock()) {
        iter->isStatement(true);
      }
    }
    return;
  }
  size_t getArgCount() const { return vals_.size(); }
  shared_ptr<Node> getPart(int idx) const { return vals_[idx]; }
  void setPart(int idx, const shared_ptr<Node> &var) {
    if (vals_.size() <= idx) {
      vals_.resize(idx + 1);
    }
    vals_[idx] = var;
    return;
  }
  bool isArgumentList() const { return isArgumentList_; }
  bool isArgumentList(bool val) {
    isArgumentList_ = val;
    return val;
  }
  virtual string getHead() const override { return head_; }
  vector<shared_ptr<Node>> getValues() override { return vals_; }

  virtual void toCCode_(ostringstream &o) override {
    auto vals = getValues();
    auto len = vals.size();
    if (isBlock()) {
      o << "{\n";
    }
    if (isListInitialization()) {
      o << "{";
    }
    if (!isEmpty()) {
      for (auto v : vals) {
        len--;
        v->toCCode_(o);
        if (v->isBlock() || v->isSkip()) {
          continue;
        }
        if (v->isStatement() || isBlock() ||
            (!isProgram() && !isArgumentList() && len > 0)) {
          o << ";";
          o << " /* " << v->getHead() << "*/";
          o << "\n";
        } else if (!isProgram() && isArgumentList() && len > 0) {
          o << " /* " << v->getHead() << "*/";
          o << ", ";
        }
      }
    }
    if (isBlock()) {
      o << "}\n";
    }
    if (isListInitialization()) {
      o << "}";
    }
  }
  virtual Json toEsprima_() override {

    std::vector<Json> lst;
    for (auto elem : vals_) {
      lst.push_back(elem->toEsprima_());
    }
    if (isListInitialization()) {

      Json::object obj;
      vector<Json> args;
      obj["type"] = "ArrayExpression";
      obj["loc"] = getLocation();
      obj["raw"] = raw_;
      obj["cform"] = toCCode();
      obj["elements"] = lst;
      return obj;
    } else {
      return Json(lst);
    }
  }
  virtual void toString_(ostringstream &o) override {
    auto vals = getValues();
    auto len = vals.size();
    if (isBlock()) {
      o << "{\n";
    }
    if (isListInitialization()) {
      o << "{";
    }
    if (!isEmpty()) {
      for (auto v : vals) {
        len--;
        v->toString_(o);
        if (v->isBlock()) {
          continue;
        }
        if (v->isStatement() || isBlock()) {
          o << "\n";
        } else if (!isProgram() && len > 0) {
          o << ", ";
        }
      }
    }
    if (isBlock()) {
      o << "}\n";
    }
    if (isListInitialization()) {
      o << "}";
    }
  }
  virtual void toJSON_(ostringstream &o) { o << "{\"type\": \"unknown\"}"; }

  virtual void traverse(ASTVisitor *visitor) override {
    for (auto elem : vals_) {
      elem->traverse(visitor);
    }
  }

protected:
  string head_ = "CompoundNode";
  bool isArgumentList_{false};
  vector<shared_ptr<Node>> vals_;
  bool isListInitialization_{false};
};

#endif /* __COMPOUND_H__ */
